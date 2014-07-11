#include <gojira/runtime/runtime.h>
#include <gojira/runtime/builtin.h>
#include <gojira/parser.h>
#include <gojira/lexer.h>
#include <gojira/parse_debug.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

token_t *eval_function( st_frame_t *frame ){
	token_t *token = NULL;
	token_t *func = NULL;
	token_t *ret = NULL;
	ext_proc_t *ext;
	scheme_func handle;

	token = func = frame->expr;
	printf( "[%s] Dumping token list for type %s\n",
			__func__, type_str( func->type ));

	switch ( func->type ){
		case TYPE_EXTERN_PROC:
			ext = func->data;
			handle = ext->handler;
			printf( "[%s] Have external procedure, evaluating at %p...\n",
					__func__, handle );

			ret = handle( frame );
			break;

		case TYPE_DEFINE_EXPR:
			printf( "[%s] Have define expression\n", __func__ );

		default:
			// debugging output until procedures are implemented
			ret = calloc( 1, sizeof( token_t ));
			ret->type = TYPE_LIST;
			ret->down = token;
			break;
	}

	printf( "===\n" );
	stack_trace( frame );
	printf( "===\n" );

	dump_tokens( ret, 2 );

	return ret;
}

st_frame_t *eval_loop( st_frame_t *frame, token_t *tokens ){
	token_t expr;
	token_t *cptr;
	st_frame_t *cur_frame;

	memset( &expr, 0, sizeof( token_t ));
	cptr = tokens;

	cur_frame = frame;

	while ( cur_frame ){
		while ( cptr ){

			// Evaluate a token list, if it's a list
			if ( cptr->type == TYPE_LIST && cptr->down ){

				printf( "[%s] Got a list, ret = %p\n", __func__, cptr );

				if ( cptr->down->type == TYPE_SYMBOL ){
					printf( "[%s] Will be evaluating \"%s\"...\n",
							__func__, (char *)cptr->down->data );
				}

				cur_frame = frame_create( cur_frame, cptr->next );
				cptr = cptr->down;

			// Otherwise add tokens in the list to the current stack frame
			} else {

				token_t *add = NULL;
				char *name = cptr->data;
				bool builtin = false;

				// handle special functions where expressions in the list shouldn't be evaluated
				if ( cptr->type == TYPE_SYMBOL && (
						strcmp( cptr->data, "define" ) == 0 ||
						strcmp( cptr->data, "lambda" ) == 0 )){

					token_t *move;
					token_t *add;

					builtin = true;
					add = frame_find_var( cur_frame, (char *)cptr->data );

					if ( add ){
						frame_add_token( cur_frame, add );

						printf( "[%s]\t symbol: \"%s\" at %p\n", __func__,
								(char *)cptr->data, add );
					} else {
						printf( "[%s] Error: got null value for builtin function (!?)\n", __func__ );
					}

					for ( cptr = cptr->next; cptr; cptr = cptr->next )
						frame_add_token( cur_frame, cptr );

					stack_trace( cur_frame );

				// Create new stack frame, enter it, and set cptr inside the list
				} else if ( cptr->type == TYPE_SYMBOL ){
					add = frame_find_var( cur_frame, (char *)cptr->data );

					printf( "[%s]\t symbol: \"%s\" at %p\n", __func__,
							name, add );

					if ( !add ){
						printf( "[%s] Error: variable \"%s\" not bound\n", __func__, name );
						// Uncomment and error out
						// stack_trace( cur_frame );
						add = cptr;
					}

				} else {
					add = cptr;
				}

				if ( !builtin ){
					cptr = cptr->next;

					//list_add_data( cur_frame->expr, add );
					frame_add_token( cur_frame, add );

					printf( "[%s] Adding token to current expression "
							"list of type \"%s\"\n", __func__, type_str( add->type ));
				}

				stack_trace( cur_frame );
			}
		}

		// Evaluate function
		cur_frame->value = eval_function( cur_frame );

		// return from current frame, if cur_frame->ret
		// is not null
		printf( "[%s] Returning to %p\n", __func__, cur_frame->ret );

		if ( cur_frame->last && cur_frame->value ){
			//list_add_data( cur_frame->last->expr, cur_frame->value );
			frame_add_token( cur_frame->last, cur_frame->value );

			printf( "[%s] Returning token to last expression type \"%s\"\n",
					__func__, type_str( cur_frame->value->type ));

		} else if ( !cur_frame->value ){
			printf( "[%s] Warning: Frame returned no value, "
					"something might be broken\n", __func__ );
		}

		cptr = cur_frame->ret;
		cur_frame = cur_frame->last;
	}

	return frame;
}

token_t *eval_tokens( stack_frame_t *st_frame, token_t *tokens ){
	//token_t *ret = tokens;
	token_t *ret;
	st_frame_t *temp_frame = frame_create( NULL, NULL );

	frame_add_var( temp_frame, "foo",
			remove_punc_tokens( parse_tokens( lexerize( "#(a b c)" ))));

	frame_add_var( temp_frame, "define",
			ext_proc_token( builtin_define ));

	frame_add_var( temp_frame, "lambda",
			ext_proc_token( builtin_lambda ));

	eval_loop( temp_frame, tokens );
	ret = temp_frame->expr;

	return ret;
}

st_frame_t *frame_create( st_frame_t *cur_frame, token_t *ret_pos ){
	st_frame_t *ret;

	ret = calloc( 1, sizeof( st_frame_t ));
	ret->last = cur_frame;
	ret->ret = ret_pos;

	ret->value = NULL;
	ret->expr = NULL;

	return ret;
}

token_t *frame_find_var( st_frame_t *frame, char *key ){
	token_t *ret = NULL;
	list_node_t *temp;
	variable_t *var;

	if ( frame ){
		if ( frame->vars ){
			temp = frame->vars->base;
			foreach_in_list( temp ){
				var = temp->data;

				if ( strcmp( var->key, key ) == 0 ){
					ret = var->token;
					break;
				}
			}

		} else {
			ret = frame_find_var( frame->last, key );
		}
	}
	return ret;
}

variable_t *frame_add_var( st_frame_t *frame, char *key, token_t *token ){
	variable_t *new_var;

	if ( !frame->vars )
		frame->vars = list_create( 0 );

	new_var = calloc( 1, sizeof( variable_t ));
	new_var->key = strdup( key );
	new_var->token = token;

	list_add_data( frame->vars, new_var );

	return new_var;
}

void stack_trace( st_frame_t *frame ){
	st_frame_t *move = frame;
	token_t *token = NULL;
	token_t *func = NULL;
	unsigned i;

	printf( "[stack trace]\n" );
	for ( move = frame, i = 0; move; move = move->last, i++ ){
		printf( "[%u] ", i );

		func = move->expr;
		token = func;

		foreach_in_list( token ){
			printf( "%s", type_str( token->type ));

			if ( token->type == TYPE_SYMBOL ){
				char *name = token->data;

				printf( " \"%s\"", name );
				printf( " (at %p)", frame_find_var( frame, name ));
			}

			if ( token->next )
				printf( " -> " );
		}

		printf( "\n" );
	}
}

token_t *frame_add_token( st_frame_t *frame, token_t *token ){
	token_t *ret = token;

	if ( !frame->expr ){
		frame->expr = frame->end = clone_token_tree( token );
		
	} else {
		frame->end->next = clone_token_tree( token );
		frame->end = frame->end->next;
	}

	frame->ntokens++;

	return ret;
}
