#include <stdio.h>
#include <stdlib.h>
#include <gojira/parse_debug.h>
#include <gojira/tokens.h>
#include <stdbool.h>
#include <string.h>

token_t *dump_tokens( token_t *tokens, int level ){
	if ( tokens ){
		int i;
		for ( i = 0; i < level * 4; i++ )
			putchar( ' ' );

		printf( "%s", type_str( tokens->type ));
		switch( tokens->type ){
			case TYPE_SYMBOL:
				printf( ": %s", (char *)tokens->data );
				break;
			case TYPE_CHAR:
				printf( ": %c", tokens->smalldata );
				break;
			case TYPE_NUMBER:
				printf( ": %d", tokens->smalldata );
				break;
			case TYPE_BOOLEAN:
				printf( ": %c", (tokens->smalldata == true)? 't' : 'f' );
				break;
			case TYPE_STRING:
				printf( ": \"%s\"", (char *)tokens->data );
				break;

			default:
				break;
		}
		printf( "\n" );
		//printf( "Token, type: %d\n", tokens->type );
		dump_tokens( tokens->down, level + 1 );
		dump_tokens( tokens->next, level );
	}

	return tokens;
}

// Strips a token from a tree. If the stripped token has a lower token,
// the lower token (and all "next" nodes) will be merged into the same level
// as the original token.
token_t *strip_token( token_t *tokens, type_t type ){
	token_t *ret = tokens;
	token_t *temp;

	if ( tokens ){
		if ( tokens->type == type ){
			if ( tokens->down ){
				ret = strip_token( tokens->down, type );
				temp = ret->next;
				if ( temp ){
					while ( temp->next ) temp = temp->next;
					temp->next = strip_token( tokens->next, type );
				} else {
					ret->next = strip_token( tokens->next, type );
				}

				free( tokens );

			} else {
				ret = strip_token( tokens->next, type );
				free( tokens );
			}

		} else {
			ret->down = strip_token( tokens->down, type );
			ret->next = strip_token( tokens->next, type );
		}
	}

	return ret;
}

token_t *remove_punc_tokens( token_t *tokens ){
	token_t *ret = tokens;
	type_t remove[] = {
		TYPE_OPEN_PAREN, TYPE_CLOSE_PAREN, TYPE_APOSTR,
		TYPE_OCTOTHORPE, TYPE_TOKEN_LIST, TYPE_BASE_TOKEN, TYPE_NULL
	};

	int i;
	int size = sizeof( remove ) / sizeof( type_t );

	for ( i = 0; ret && i < size; i++ )
		ret = strip_token( ret, remove[i] );

	if ( ret ){
		ret->down = remove_punc_tokens( ret->down );
		ret->next = remove_punc_tokens( ret->next );
	}

	return ret;
}

// Clones every token reachable from the given token
token_t *clone_tokens( token_t *tree ){
	token_t *ret = NULL;

	if ( tree ){
		ret = calloc( 1, sizeof( token_t ));
		// TODO: copy tree->data or tree->smalldata depending on the type
		memcpy( ret, tree, sizeof( token_t ));

		ret->down = clone_tokens( tree->down );
		ret->next = clone_tokens( tree->next );
	}
	
	return ret;
}

// Clones a token and all lower nodes
token_t *clone_token_tree( token_t *tree ){
	token_t *ret = NULL;

	if ( tree ){
		ret = calloc( 1, sizeof( token_t ));
		memcpy( ret, tree, sizeof( token_t ));

		ret->down = clone_tokens( tree->down );
		ret->next = NULL;
	}

	return ret;
}

token_t *alloc_token( void ){
	token_t *ret;

	ret = calloc( 1, sizeof( token_t ));

	return ret;
}

void free_tokens( token_t *tree ){
	if ( tree ){
		free_tokens( tree->down );
		free_tokens( tree->next );
		free( tree );

		// TODO: Free data field in tokens properly, will likely need reference tracking.
	}
}

void free_token_tree( token_t *tree ){
	if ( tree ){
		free_tokens( tree->down );
		free( tree );
	}
}

unsigned tokens_length( token_t *tree ){
	unsigned ret;
	token_t *move = tree;

	for ( ret = 0; move; move = move->next, ret++ );

	return ret;
}

token_t *replace_symbol( token_t *tokens, token_t *replace, char *name ){
	token_t *ret = tokens;

	if ( tokens ){
		if ( tokens->type == TYPE_SYMBOL && ( strcmp( tokens->data, name )) == 0 ){
			ret = clone_token_tree( replace );
			ret->next = replace_symbol( tokens->next, replace, name );
			free_token_tree( tokens );

		} else {
			ret->down = replace_symbol( ret->down, replace, name );
			ret->next = replace_symbol( ret->next, replace, name );
		}
	}
	
	return ret;
}

// Replaces tokens while preserving variable definitions in lambdas/procedures.
token_t *replace_symbol_safe( token_t *tokens, token_t *replace, char *name ){
	token_t *ret = tokens;

	if ( tokens ){
		if ( tokens->type == TYPE_SYMBOL && ( strcmp( tokens->data, name )) == 0 ){
			ret = clone_token_tree( replace );
			ret->next = replace_symbol_safe( tokens->next, replace, name );
			free_token_tree( tokens );

		} else if ( tokens->type == TYPE_LAMBDA ){
			ret->next->next = replace_symbol_safe( ret->next->next, replace, name );

		} else if ( tokens->type == TYPE_PROCEDURE ){
			ret->next = replace_symbol_safe( ret->next, replace, name );

		} else {
			ret->down = replace_symbol_safe( ret->down, replace, name );
			ret->next = replace_symbol_safe( ret->next, replace, name );
		}
	}
	
	return ret;
}

