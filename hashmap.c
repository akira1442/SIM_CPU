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
	HashMap* newMap = (HashMap*)calloc(sizeof(HashMap));
	newMap -> size = TABLE_SIZE;
	newMap -> table = (HashEntry*)calloc(TABLE_SIZE,sizeof(HashMap));
	return newMap;

}

int HashMap_insert(HashMap* map, const char* key, void* value){
	
	HashEntry* tab = map->table;
	
	for (int i = 0; i < map->size; i++){
		if (tab[i] == NULL){
			tab[i] = (HashEntry*)malloc(sizeof(HashEntry));
			tab[i].key = strdup(key);
			tab[i].value = value;
			return 1;
		}
		else{
			tab[i].key = strdup(key);
			tab[i].value = value;
			return 1;
		}
	}
	return 0;
}

void* HashMap_get(HashMap* map, const char* key){
	
	for (int i =0; i < map->size; i++){
		if (strcmp(map->table[i].key, key)){
			return map->table[i].value ;
		}
	}
	return NULL ;
}

int HashMap_remove(HashMap* map, const char* key){
	
	for (int i = 0; i < map->size; i++){
		if (strcmp(map->table[i].key,key)){
			map->table[i] = TOMBSTONE;
			return 1;
		}
	}
	return 0;
}

void HashMap_destroy(HashMap *map){
	for (int i = 0; i < map->size; i++){
		free(map->table[i].key);
	}
	free(map);
}
