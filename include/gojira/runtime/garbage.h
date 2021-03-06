#ifndef _GOJIRA_RUNTIME_GARBAGE_H
#define _GOJIRA_RUNTIME_GARBAGE_H
#include <stdbool.h>

#include <stdio.h>

enum {
	GC_COLOR_WHITE,
	GC_COLOR_GREY,
	GC_COLOR_BLACK,

	/*
	GC_UNMARKED,
	GC_MARKED,
	GC_FREED,
	*/
};

enum {
	GC_TYPE_TOKEN,
	GC_TYPE_ENVIRONMENT,
	GC_TYPE_CONTINUATION,
	GC_TYPE_VARIABLE,
	GC_TYPE_PROCEDURE,
};

enum {
	GC_PROFILE_FAST,
	GC_PROFILE_BALANCED,
	GC_PROFILE_LOWMEM,
};

typedef struct gbg_node {
	struct gbg_node *next;
	struct gbg_node *prev;
	unsigned type;
	unsigned status;
} gbg_node_t;

typedef struct gbg_list {
	gbg_node_t *start;
	gbg_node_t *end;
	unsigned length;
} gbg_list_t;

typedef struct gbg_collector {
	gbg_list_t colors[3];
	unsigned id;
	unsigned iter;
	unsigned interval;

	unsigned default_interval;
	double target_ratio;
} gbg_collector_t;

#include <gojira/tokens.h>
typedef struct token token_t;

token_t *gc_alloc_token( gbg_collector_t *gc );
token_t *gc_clone_token( gbg_collector_t *gc, token_t *token );
token_t *gc_clone_token_spine( gbg_collector_t *gc, token_t *token );
void *gc_register( gbg_collector_t *gc, void *thing );
//token_t *gc_register_token( gbg_collector_t *gc, token_t *token );
token_t *gc_register_token_tree( gbg_collector_t *gc, token_t *tokens );
token_t *gc_register_tokens( gbg_collector_t *gc, token_t *token );
token_t *gc_move_token( gbg_collector_t *to, gbg_collector_t *from, token_t *token );
void gc_free_token( gbg_collector_t *gc );
void gc_collect( gbg_collector_t *gc );
bool gc_should_collect( gbg_collector_t *gc );
gbg_collector_t *gc_init( gbg_collector_t *old_gc, gbg_collector_t *new_gc );
gbg_collector_t *gc_merge( gbg_collector_t *first, gbg_collector_t *second );

typedef struct stack_frame stack_frame_t;
void gc_try_to_collect_frame( stack_frame_t *frame );
gbg_collector_t *get_current_gc( stack_frame_t *frame );

void gc_set_profile( gbg_collector_t *gbg, unsigned profile );

#define DEPRECATED __attribute__((deprecated))

void gc_mark( token_t *tree ) DEPRECATED;
void gc_mark_tree( token_t *tree ) DEPRECATED;
void gc_unmark( token_t *tree ) DEPRECATED;
void gc_unmark_tree( token_t *tree ) DEPRECATED;
token_t *gc_sweep( token_t *tree ) DEPRECATED;

//void gc_dump( stack_frame_t *frame );

#undef DEPRECATED

#endif
