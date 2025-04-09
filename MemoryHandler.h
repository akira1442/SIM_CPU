#include "hashmap.h"

typedef struct segment{
    int start;              // Position de debut (adresse) du segment dans la memoire
    int size;               // Taille du segment en unités de taille
    struct segment* next;   // Pointeur vers le segment suivant dans la liste chainee
}Segment;

typedef struct memoryHandler{
    void **memory;  // Tableau de pointeurs vers la memoire allouee
    Segment *free_list; // Liste chainee des segments de memoire libres
    HashMap *allocated; // Table de hachage (nom, segment)
    int total_size; //  Taille totale de la memoire geree
}MemoryHandler;

MemoryHandler *memory_init(int size);
Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev);
int create_segment(MemoryHandler *handler, const char *name, int start, int size);
int remove_segment(MemoryHandler *handler, const char *name);
