#include <stdio.h>
#include <stdlib.h>
#define TABLE_SIZE 128 
#define TOMBSTONE  ((void*)-1)

typedef struct hashentry {
	char * key ;
	void * value ;
} HashEntry ;

typedef struct hashmap {
	int size ;
	HashEntry* table;
} HashMap ;

unsigned long simple_hash(const char* str);
HashMap *hashmap_create();
int HashMap_insert(HashMap* map, const char* key, void *value);
void* HashMap_get(HashMap* map, const char* key);
int HashMap_remove(HashMap* map, const char* key);
void HashMap_destroy(HashMap* map);
