#ifndef _JOOL_MOD_NAMESPACE_H
#define _JOOL_MOD_NAMESPACE_H

#include "config.h"

/**
 * A Jool translator "instance". The point is that each network namespace has
 * a separate instance (if Jool has been loaded there).
 *
 * The instance holds all the databases and configuration the translating code
 * should use to handle a packet in the respective namespace.
 */
struct xlator {
	/* Shared */
	struct jool_stats *stats;
	struct global_config *global;
	struct config_candidate *newcfg;

	/* SIIT */
	struct eam_table *eamt;

	/* NAT64 */
	struct pool4 *pool4;
	struct bib *bib;
	struct joold_queue *joold;
};

int xlator_add(struct xlator *jool);
int xlator_replace(struct xlator *instance);

//int xlator_find(struct net *ns, struct xlator *result);
//int xlator_find_current(struct xlator *result);

void xlator_get(struct xlator *instance);
void xlator_put(struct xlator *instance);

typedef int (*xlator_foreach_cb)(struct xlator *, void *);
int xlator_foreach(xlator_foreach_cb cb, void *args);

void xlator_copy_config(struct xlator *instance, struct full_config *copy);

#endif /* _JOOL_MOD_NAMESPACE_H */
