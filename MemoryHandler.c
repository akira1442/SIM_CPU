#include "MemoryHandler.h"

MemoryHandler* memory_init(int size){
    MemoryHandler *handler = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    handler->memory = (void**)malloc(size*sizeof(void*));
    handler->free_list = (Segment*)malloc(sizeof(Segment));
    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;
    handler->allocated = create_hash_map();
    handler->total_size = size;
    return handler;
}

Segment find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
    Segment *current = handler->free_list;
    while(current != NULL){
        if(current->start <= start && current->size >= size){
            return current;
        }
        *prev = current;
        current = current->next;
    }
    return NULL;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size){

    Segment* prev = NULL;
    Segment* seg_free = find_free_segment(handler, start, size, &prev);

    // Si il existe un espace mémoire libre et suffisant disponible
    if (seg_free){
        // Création du nouveau segment
        Segment* new_seg = (Segment*)malloc(sizeof(Segment));
        new_seg->size = size;
        new_seg->start = start;
        new_seg->next = NULL;

        // On doit rechaîné les espaces libre entre eux, on a 4 cas différents
        // Cas où l'adresse est plus grande que l'adresse du segment libre
        if ((start > seg_free->start) && (start+size == seg_free->next->start)){
            seg_free->size -= new_seg->size;
        }
        // Cas où l'espace disponible est plus grand que l'espace demandé
        else if (size < seg_free->size){
            seg_free->start += new_seg->size;
            seg_free->size -= size;
        }
        // Cas où l'espace demandé se trouve au millieu de l'espace disponible
        else if ((start > seg_free->size) && (start+size < seg_free->next->size)){
            Segment* tmp = (Segment*)malloc(sizeof(Segment));
            tmp->start = start+size;
            tmp->size = seg_free->size - (seg_free->start + (start+size));
            tmp->next = seg_free->next;
            seg_free->size -= new_seg->size;
            seg_free->next = tmp;
        }
        // Notre 4e cas c'est lors que l'espace demandé est de la même taille que l'espace disponible
        // Insertion du segment dans la table de hachage
        if (HashMap_insert(handler->allocated, name, new_seg) == 0){
            fprintf(stderr, "ERREUR:insertion du nouveau segment dans handler");
            return 0;
        }
        return 1;
    }
    // Le cas échéant
    return 0;
}