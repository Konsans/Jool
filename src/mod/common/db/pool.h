#ifndef SRC_MOD_SIIT_POOL_H_
#define SRC_MOD_SIIT_POOL_H_

/**
 * @file
 * This is a handler for pool of IPv4 addresses.
 */

#include "common/types.h"

struct addr4_pool;

/* Do-not-use-when-you-can't-sleep-functions */

struct addr4_pool *pool_alloc(void);
void pool_get(struct addr4_pool *pool);
void pool_put(struct addr4_pool *pool);

int pool_add(struct addr4_pool *pool, struct ipv4_prefix *prefix, bool force);
int pool_rm(struct addr4_pool *pool, struct ipv4_prefix *prefix);
int pool_flush(struct addr4_pool *pool);

/* Safe-to-use-during-packet-translation functions */

bool pool_contains(struct addr4_pool *pool, struct in_addr *addr);
int pool_foreach(struct addr4_pool *pool,
		int (*func)(struct ipv4_prefix *, void *), void *arg,
		struct ipv4_prefix *offset);
bool pool_is_empty(struct addr4_pool *pool);
void pool_print_refcount(struct addr4_pool *pool);

#endif /* SRC_MOD_SIIT_POOL_H_ */
