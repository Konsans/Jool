#include "usr/nl/attribute.h"

#include <errno.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>

static struct jool_result validate_mandatory_attrs(struct nlattr *attrs[],
		int maxtype, struct nla_policy const *policy)
{
	int i;

	/* All defined attributes are mandatory */
	for (i = 0; i < maxtype; i++) {
		if (policy[i].type && !attrs[i]) {
			return result_from_error(
				-EINVAL,
				"The kernel module's response is missing attribute %u.",
				i
			);
		}
	}

	return result_success();
}

/* Wrapper for genlmsg_parse(). */
struct jool_result jnla_parse_msg(struct nl_msg *msg, struct nlattr *tb[],
		int maxtype, struct nla_policy *policy,
		bool validate_mandatories)
{
	int error;

	error = genlmsg_parse(nlmsg_hdr(msg), sizeof(struct joolnlhdr), tb,
			maxtype, policy);
	if (!error) {
		return validate_mandatories
				? validate_mandatory_attrs(tb, maxtype, policy)
				: result_success();
	}

	return result_from_error(
		error,
		"Could not parse Jool's Netlink response: %s",
		nl_geterror(error)
	);
}

/* Wrapper for nla_parse_nested(). */
struct jool_result jnla_parse_nested(struct nlattr *tb[], int maxtype,
		struct nlattr *root, struct nla_policy *policy)
{
	int error;

	error = nla_parse_nested(tb, maxtype, root, policy);
	if (!error)
		return validate_mandatory_attrs(tb, maxtype, policy);

	return result_from_error(
		error,
		"Could not parse a nested attribute in Jool's Netlink response: %s",
		nl_geterror(error)
	);
}

/* Wrapper for nla_validate() for lists */
struct jool_result jnla_validate_list(struct nlattr *head, int len,
		char const *what, struct nla_policy const *policy)
{
	struct nlattr *attr;
	int rem;
	int error;

	error = nla_validate(head, len, LA_MAX, policy);
	if (error) {
		return result_from_error(
			error,
			"The kernel's response does not contain a valid '%s' attribute list. (Unknown cause)",
			what
		);
	}

	/* Sigh */
	nla_for_each_attr(attr, head, len, rem) {
		if (nla_type(attr) != LA_ENTRY) {
			return result_from_error(
				-EINVAL,
				"The kernel's response contains unexpected attribute '%d' in its '%s' list.",
				nla_type(attr), what
			);
		}
	}

	return result_success();
}

void nla_get_addr6(struct nlattr const *attr, struct in6_addr *addr)
{
	memcpy(addr, nla_data(attr), sizeof(*addr));
}

void nla_get_addr4(struct nlattr const *attr, struct in_addr *addr)
{
	memcpy(addr, nla_data(attr), sizeof(*addr));
}

struct jool_result nla_get_prefix6(struct nlattr *root, struct ipv6_prefix *out)
{
	struct nlattr *attrs[PA_COUNT];
	struct jool_result result;

	/*
	 * Because they can be empty on globals, the policy marks them as
	 * unspecified. We can't return an unset prefix in this function,
	 * so we need to validate this here.
	 */
	if (nla_len(root) == 0) {
		return result_from_error(
			-EINVAL,
			"The IPv6 prefix attribute is empty."
		);
	}

	result = jnla_parse_nested(attrs, PA_MAX, root, prefix6_policy);
	if (result.error)
		return result;

	nla_get_addr6(attrs[PA_ADDR], &out->addr);
	out->len = nla_get_u8(attrs[PA_LEN]);
	return result_success();
}

struct jool_result nla_get_prefix4(struct nlattr *root, struct ipv4_prefix *out)
{
	struct nlattr *attrs[PA_COUNT];
	struct jool_result result;

	/*
	 * Because they can be empty on globals, the policy marks them as
	 * unspecified. We can't return an unset prefix in this function,
	 * so we need to validate this here.
	 */
	if (nla_len(root) == 0) {
		return result_from_error(
			-EINVAL,
			"The IPv4 prefix attribute is empty."
		);
	}

	result = jnla_parse_nested(attrs, PA_MAX, root, prefix4_policy);
	if (result.error)
		return result;

	nla_get_addr4(attrs[PA_ADDR], &out->addr);
	out->len = nla_get_u8(attrs[PA_LEN]);
	return result_success();
}

struct jool_result nla_get_taddr6(struct nlattr *root, struct ipv6_transport_addr *out)
{
	struct nlattr *attrs[TAA_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, TAA_MAX, root, taddr6_policy);
	if (result.error)
		return result;

	nla_get_addr6(attrs[TAA_ADDR], &out->l3);
	out->l4 = nla_get_u16(attrs[TAA_PORT]);
	return result_success();
}

struct jool_result nla_get_taddr4(struct nlattr *root, struct ipv4_transport_addr *out)
{
	struct nlattr *attrs[TAA_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, TAA_MAX, root, taddr4_policy);
	if (result.error)
		return result;

	nla_get_addr4(attrs[TAA_ADDR], &out->l3);
	out->l4 = nla_get_u16(attrs[TAA_PORT]);
	return result_success();
}

struct jool_result nla_get_eam(struct nlattr *root, struct eamt_entry *out)
{
	struct nlattr *attrs[EA_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, EA_MAX, root, eam_policy);
	if (result.error)
		return result;

	result = nla_get_prefix6(attrs[EA_PREFIX6], &out->prefix6);
	if (result.error)
		return result;

	return nla_get_prefix4(attrs[EA_PREFIX4], &out->prefix4);
}

struct jool_result nla_get_pool4(struct nlattr *root, struct pool4_entry *out)
{
	struct nlattr *attrs[P4A_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, P4A_MAX, root, pool4_entry_policy);
	if (result.error)
		return result;

	out->mark = nla_get_u32(attrs[P4A_MARK]);
	out->iterations = nla_get_u32(attrs[P4A_ITERATIONS]);
	out->flags = nla_get_u8(attrs[P4A_FLAGS]);
	out->proto = nla_get_u8(attrs[P4A_PROTO]);
	out->range.ports.min = nla_get_u16(attrs[P4A_PORT_MIN]);
	out->range.ports.max = nla_get_u16(attrs[P4A_PORT_MAX]);
	return nla_get_prefix4(attrs[P4A_PREFIX], &out->range.prefix);
}

struct jool_result nla_get_bib(struct nlattr *root, struct bib_entry *out)
{
	struct nlattr *attrs[BA_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, BA_MAX, root, bib_entry_policy);
	if (result.error)
		return result;

	result = nla_get_taddr6(attrs[BA_SRC6], &out->addr6);
	if (result.error)
		return result;
	result = nla_get_taddr4(attrs[BA_SRC4], &out->addr4);
	if (result.error)
		return result;
	out->l4_proto = nla_get_u8(attrs[BA_PROTO]);
	out->is_static = nla_get_u8(attrs[BA_STATIC]);
	return result_success();
}

struct jool_result nla_get_session(struct nlattr *root, struct session_entry_usr *out)
{
	struct nlattr *attrs[SEA_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, SEA_MAX, root, session_entry_policy);
	if (result.error)
		return result;

	result = nla_get_taddr6(attrs[SEA_SRC6], &out->src6);
	if (result.error)
		return result;
	result = nla_get_taddr6(attrs[SEA_DST6], &out->dst6);
	if (result.error)
		return result;
	result = nla_get_taddr4(attrs[SEA_SRC4], &out->src4);
	if (result.error)
		return result;
	result = nla_get_taddr4(attrs[SEA_DST4], &out->dst4);
	if (result.error)
		return result;
	out->proto = nla_get_u8(attrs[SEA_PROTO]);
	out->state = nla_get_u8(attrs[SEA_STATE]);
	out->dying_time = nla_get_u32(attrs[SEA_EXPIRATION]);
	return result_success();
}

struct jool_result nla_get_plateaus(struct nlattr *root,
		struct mtu_plateaus *out)
{
	struct nlattr *attr;
	int rem;
	struct jool_result result;

	result = jnla_validate_list(nla_data(root), nla_len(root), "plateus",
			plateau_list_policy);
	if (result.error)
		return result;

	out->count = 0;
	nla_for_each_nested(attr, root, rem) {
		if (out->count >= PLATEAUS_MAX) {
			return result_from_error(
				-EINVAL,
				"The kernel's response has too many plateaus."
			);
		}
		out->values[out->count] = nla_get_u16(attr);
		out->count++;
	}

	return result_success();
}

static int nla_put_addr6(struct nl_msg *msg, int attrtype, struct in6_addr const *addr)
{
	return nla_put(msg, attrtype, sizeof(*addr), addr);
}

static int nla_put_addr4(struct nl_msg *msg, int attrtype, struct in_addr const *addr)
{
	return nla_put(msg, attrtype, sizeof(*addr), addr);
}

int nla_put_prefix6(struct nl_msg *msg, int attrtype, struct ipv6_prefix const *prefix)
{
	struct nlattr *root;

	if (!prefix) {
		if (nla_put(msg, attrtype, 0, NULL) < 0)
			goto abort;
		return 0;
	}

	root = nla_nest_start(msg, attrtype);
	if (!root)
		goto abort;

	if (nla_put_addr6(msg, PA_ADDR, &prefix->addr) < 0)
		goto cancel;
	if (nla_put_u8(msg, PA_LEN, prefix->len) < 0)
		goto cancel;

	nla_nest_end(msg, root);
	return 0;

cancel:	nla_nest_cancel(msg, root);
abort:	return -NLE_NOMEM;
}

int nla_put_prefix4(struct nl_msg *msg, int attrtype, struct ipv4_prefix const *prefix)
{
	struct nlattr *root;

	if (!prefix) {
		if (nla_put(msg, attrtype, 0, NULL) < 0)
			goto abort;
		return 0;
	}

	root = nla_nest_start(msg, attrtype);
	if (!root)
		goto abort;

	if (nla_put_addr4(msg, PA_ADDR, &prefix->addr) < 0)
		goto cancel;
	if (nla_put_u8(msg, PA_LEN, prefix->len) < 0)
		goto cancel;

	nla_nest_end(msg, root);
	return 0;

cancel:	nla_nest_cancel(msg, root);
abort:	return -NLE_NOMEM;
}

static int nla_put_taddr6(struct nl_msg *msg, int attrtype, struct ipv6_transport_addr const *taddr)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	if (nla_put_addr6(msg, TAA_ADDR, &taddr->l3) < 0)
		goto cancel;
	if (nla_put_u16(msg, TAA_PORT, taddr->l4) < 0)
		goto cancel;

	nla_nest_end(msg, root);
	return 0;

cancel:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}

static int nla_put_taddr4(struct nl_msg *msg, int attrtype, struct ipv4_transport_addr const *taddr)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	if (nla_put_addr4(msg, TAA_ADDR, &taddr->l3) < 0)
		goto cancel;
	if (nla_put_u16(msg, TAA_PORT, taddr->l4) < 0)
		goto cancel;

	nla_nest_end(msg, root);
	return 0;

cancel:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}

int nla_put_plateaus(struct nl_msg *msg, int attrtype, struct mtu_plateaus const *plateaus)
{
	struct nlattr *root;
	unsigned int i;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	for (i = 0; i < plateaus->count; i++) {
		if (nla_put_u16(msg, LA_ENTRY, plateaus->values[i]) < 0) {
			nla_nest_cancel(msg, root);
			return -NLE_NOMEM;
		}
	}

	nla_nest_end(msg, root);
	return 0;
}

int nla_put_eam(struct nl_msg *msg, int attrtype, struct eamt_entry const *entry)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	if (nla_put_prefix6(msg, EA_PREFIX6, &entry->prefix6) < 0)
		goto cancel;
	if (nla_put_prefix4(msg, EA_PREFIX4, &entry->prefix4) < 0)
		goto cancel;

	nla_nest_end(msg, root);
	return 0;

cancel:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}

int nla_put_pool4(struct nl_msg *msg, int attrtype, struct pool4_entry const *entry)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	NLA_PUT_U32(msg, P4A_MARK, entry->mark);
	NLA_PUT_U32(msg, P4A_ITERATIONS, entry->iterations);
	NLA_PUT_U8(msg, P4A_FLAGS, entry->flags);
	NLA_PUT_U8(msg, P4A_PROTO, entry->proto);
	if (nla_put_prefix4(msg, P4A_PREFIX, &entry->range.prefix) < 0)
		goto nla_put_failure;
	NLA_PUT_U16(msg, P4A_PORT_MIN, entry->range.ports.min);
	NLA_PUT_U16(msg, P4A_PORT_MAX, entry->range.ports.max);

	nla_nest_end(msg, root);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}

int nla_put_bib_attrs(struct nl_msg *msg, int attrtype,
		struct ipv6_transport_addr const *addr6,
		struct ipv4_transport_addr const *addr4,
		l4_protocol proto,
		bool is_static)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	if (addr6 && nla_put_taddr6(msg, BA_SRC6, addr6) < 0)
		goto nla_put_failure;
	if (addr4 && nla_put_taddr4(msg, BA_SRC4, addr4) < 0)
		goto nla_put_failure;
	NLA_PUT_U8(msg, BA_PROTO, proto);
	NLA_PUT_U8(msg, BA_STATIC, is_static);

	nla_nest_end(msg, root);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}

int nla_put_bib(struct nl_msg *msg, int attrtype, struct bib_entry const *entry)
{
	return nla_put_bib_attrs(msg, attrtype, &entry->addr6, &entry->addr4,
			entry->l4_proto, entry->is_static);
}

int nla_put_session(struct nl_msg *msg, int attrtype, struct session_entry_usr const *entry)
{
	struct nlattr *root;

	root = nla_nest_start(msg, attrtype);
	if (!root)
		return -NLE_NOMEM;

	if (nla_put_taddr6(msg, SEA_SRC6, &entry->src6) < 0)
		goto nla_put_failure;
	if (nla_put_taddr6(msg, SEA_DST6, &entry->dst6) < 0)
		goto nla_put_failure;
	if (nla_put_taddr4(msg, SEA_SRC4, &entry->src4) < 0)
		goto nla_put_failure;
	if (nla_put_taddr4(msg, SEA_DST4, &entry->dst4) < 0)
		goto nla_put_failure;
	NLA_PUT_U8(msg, SEA_PROTO, entry->proto);
	NLA_PUT_U8(msg, SEA_STATE, entry->state);
	NLA_PUT_U32(msg, SEA_EXPIRATION, entry->dying_time);

	nla_nest_end(msg, root);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, root);
	return -NLE_NOMEM;
}