#include <gojira/runtime/syntax.h>
#include <gojira/runtime/runtime.h>
#include <gojira/runtime/builtin.h>
#include <gojira/runtime/garbage.h>
#include <gojira/parse_debug.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

stack_frame_t *expand_procedure( stack_frame_t *frame, token_t *tokens ){
	stack_frame_t *ret = NULL;
	token_t *move;
	token_t *temp;
	token_t *orig_expr;

	token_t *args;
	token_t *body;
	char *var_name;
	bool is_tailcall = false;

	if ( tokens->type == TYPE_PROCEDURE ){
		move = tokens->down;

		args = move->next;

		if ( args && args->type == TYPE_LIST ){
			body = clone_tokens( args->next );
			move = tokens->next;
			temp = args->down;

			foreach_in_list( temp ){
				if ( temp->type == TYPE_SYMBOL ){
					var_name = temp->data;

					if ( !move ){
						printf( "[%s] Error: Have unbound variable \"%s\"\n", __func__, var_name );
						break;
					}

					token_t *add;
					add = clone_token_tree( temp );
					add->down = clone_token_tree( move );

					gc_unmark( add );
					frame_register_token( frame, add );

					body = replace_symbol_safe( body, add, var_name );
					move = move->next;

				} else {
					printf( "[%s] Error: expected symbol in procedure definition, have \"%s\"\n",
							__func__, type_str( temp->type ));
					stack_trace( ret );
				}
			}

			if ( frame->last->ptr == NULL && frame->last->status == TYPE_PROCEDURE ){
				ret = frame->last;
				is_tailcall = true;
			} else {
				ret = frame;
			}

			ret->expr = ret->end = NULL;

			temp = ext_proc_token( builtin_return_last );
			frame_add_token_noclone( ret, temp );

			gc_unmark( body );

			frame_register_token( ret, body );

			for ( temp = body->next; temp; temp = temp->next )
				frame_register_token( ret, temp );

			ret->ptr = body;

			if ( is_tailcall ){
				gc_sweep( frame->heap );
				frame_free( frame );
			}
		}

	} else {
		printf( "[%s] Error: Trying to apply non-procedure as procedure (?!)\n", __func__ );
	}

	return ret;
}

token_t *expand_if_expr( stack_frame_t *frame, token_t *tokens ){
	token_t *ret = NULL; 
	token_t *move;
	int len;

	len = tokens_length( tokens );

	if ( len == 4 ){

		move = alloc_token( );
		move->type = TYPE_IF;
		move->down = clone_tokens( frame->ptr->next->next );
		move->next = frame_register_token( frame, clone_token_tree( frame->ptr->next ));
		gc_unmark( move );

		ret = move;

	} else {
		printf( "[%s] Error: If statement expected 4 tokens, but got %d\n", __func__, len );
	}

	return ret;
}

token_t *expand_syntax_rules( stack_frame_t *frame, token_t *tokens ){
	token_t *ret = NULL;

	token_t *keywords;
	token_t *cur;
	token_t *pattern;
	token_t *template;

	token_t *move, *foo;
	bool matched = false;
	int args;
	int len;

	cur = tokens->down;
	len = tokens_length( cur );
	args = tokens_length( tokens );

	if ( len >= 3 ){
		cur = cur->next->next;

		for ( ; cur; cur = cur->next ){
			pattern = cur->down->down;
			template = cur->down->next;

			if ( tokens_length( pattern ) == args ){
				matched = true;
				ret = clone_token_tree( template );

				for ( move = pattern, foo = tokens; move && foo;
						move = move->next, foo = foo->next )
				{
					ret = replace_symbol( ret, foo, move->data );
				}

				gc_unmark( ret );
				frame_register_token( frame, ret );
			}
		}

	} else {
		printf( "[%s] Error: Expected at least 3 tokens, but got %d\n", __func__, len );
	}

	if ( !matched ){
		printf( "[%s] Error: Could not match syntax pattern\n", __func__ );
	}

	return ret;
}