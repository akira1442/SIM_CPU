#include "hashmap.h" 

unsigned long simple_hash(const char* str){
	
	unsigned long res = 0;
	int i = 0;
	
	while(str[i] != '\0'){
		res += str[i];
		i++;
	}	
	return res % TABLE_SIZE;
}

HashMap* hashmap_create(){
	HashMap* newMap = (HashMap*)malloc(sizeof(HashMap));
	newMap -> size = TABLE_SIZE;
	newMap -> table = (HashEntry*)calloc(TABLE_SIZE,sizeof(HashMap));
	return newMap;

}

int HashMap_insert(HashMap* map, const char* key, void* value){
	
	if (!map) return 0;

	for (int i = 0; i < TABLE_SIZE; i++){
		
		if (map->table[simple_hash(key)+i].value == NULL){
			map->table[simple_hash(key)+i].key = strdup(key);
			map->table[simple_hash(key)+i].value = value;
			return 1;
		}
	}
	return 0;
}

void* HashMap_get(HashMap* map, const char* key){
	
	if (!map) return NULL;

	for (int i =0; i < map->size; i++){
		if (strcmp(map->table[simple_hash(key)+i].key, key)){
			return map->table[simple_hash(key)+i].value ;
		}
	}
	return NULL ;
}

int HashMap_remove(HashMap* map, const char* key){
	
	if (!map) return 0;

	for (int i = 0; i < map->size; i++){
		if ((map->table[simple_hash(key)+i].key != NULL) && (strcmp(map->table[simple_hash(key)+i].key, key) == 0)){
			map->table[i].value = TOMBSTONE;
			free(map->table[i].key);
			map->table[i].key = NULL;
			return 1;
		}
	}
	return 0;
}

void HashMap_destroy(HashMap *map){

	if (map == NULL) return;

	for (int i = 0; i < TABLE_SIZE; i++){
		if (map->table[i].key != NULL){
			free(map->table[i].key);
		}
	}
	free(map);
}

void afficher_hashmap(HashMap* map){
	
	for (int i = 0; i < map->size; i++){
		if (map->table[i].value != NULL){
			void* value = (void*)map->table[i].value;
			printf("Key: %s, Value: %p\n", map->table[i].key, value);
		}
	}
}