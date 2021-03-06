#include <gojira/libs/hashmap.h>
#include <stdlib.h>

hashmap_t *hashmap_create( unsigned n ){
	hashmap_t *ret;

	ret = calloc( 1, sizeof( hashmap_t ));
	ret->buckets = calloc( 1, sizeof( list_head_t[n] ));
	ret->nbuckets = n;

	return ret;
}

void hashmap_free( hashmap_t *map ){
	if ( map ){
		free( map->buckets );
		free( map );
	}
}

void *hashmap_add( hashmap_t *map, unsigned hash, void *val ){
	list_head_t *list;
	list_node_t *node;
	void *ret = 0;
	
	list = map->buckets + ( hash % map->nbuckets );

	node = list_add_data( list, val );
	if ( node ){
		node->val = hash;
		ret = node->data;
	}

	return ret;
}

void *hashmap_get( hashmap_t *map, unsigned hash ){
	list_head_t *list;
	list_node_t *node;
	void *ret = 0;
	
	list = map->buckets + ( hash % map->nbuckets );
	node = list_get_val( list, hash );

	if ( node )
		ret = node->data;

	return ret;
}

void hashmap_remove( hashmap_t *map, unsigned hash ){
	list_head_t *list;
	list_node_t *node;
	
	list = map->buckets + ( hash % map->nbuckets );
	node = list_get_val( list, hash );

	if ( node )
		list_remove_node( node );
}

unsigned hash_string_old( char *str ){
	unsigned ret = 0, i;

	for ( i = 0; str[i]; i++ ){
		ret ^= str[i] * (i + 1);
		ret *= str[i];
	}

	return ret;
}

// djb2 hash function, see http://www.cse.yorku.ca/~oz/hash.html
unsigned hash_string( const char *str ){
	unsigned hash = 5381;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

// this is to allow 'concatenating' hashes
unsigned hash_string_accum( const char *str, unsigned hash ){
	int c;

	while (( c = *str++ )){
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}
