#include "MemoryHandler.h"

MemoryHandler* memory_init(int size){
    MemoryHandler *handler = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    handler->memory = (void**)malloc(size*sizeof(void*));
    handler->free_list = (Segment*)malloc(sizeof(Segment));
    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;
    handler->allocated = hashmap_create();
    handler->total_size = size;
    return handler;
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
    
    Segment* current = handler->free_list;
    
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

    // Si il existe un espace mÃ©moire libre et suffisant disponible
    if (seg_free){
        // CrÃ©ation du nouveau segment
        Segment* new_seg = (Segment*)malloc(sizeof(Segment));
        new_seg->size = size;
        new_seg->start = start;
        new_seg->next = NULL;

        // On doit rechaÃ®nÃ© les espaces libre entre eux, on a 4 cas diffÃ©rents
        // Cas oÃ¹ l'adresse est plus grande que l'adresse du segment libre
        if ((start > seg_free->start) && (start+size == seg_free->next->start)){
            seg_free->size -= new_seg->size;
        }
        // Cas oÃ¹ l'espace disponible est plus grand que l'espace demandÃ©
        else if (size < seg_free->size){
            seg_free->start += new_seg->size;
            seg_free->size -= size;
        }
        // Cas oÃ¹ l'espace demandÃ© se trouve au millieu de l'espace disponible
        else if ((start > seg_free->size) && (start+size < seg_free->next->size)){
            Segment* tmp = (Segment*)malloc(sizeof(Segment));
            tmp->start = start+size;
            tmp->size = seg_free->size - (seg_free->start + (start+size));
            tmp->next = seg_free->next;
            seg_free->size -= new_seg->size;
            seg_free->next = tmp;
        }
        // Notre 4e cas c'est lors que l'espace demandÃ© est de la mÃªme taille que l'espace disponible
        // Insertion du segment dans la table de hachage
        if (HashMap_insert(handler->allocated, name, new_seg) == 0){
            fprintf(stderr, "ERREUR:insertion du nouveau segment dans handler");
            return 0;
        }
        return 1;
    }
    // Le cas Ã©chÃ©ant
    return 0;
}

int remove_segment(MemoryHandler *handler, const char *name){
    Segment* seg = (Segment*)HashMap_get(handler->allocated, name);

    if (seg){
        Segment* prev = NULL;
        Segment* current = handler->free_list;
        while (current != NULL){    //On se deplace dans la free_list jusqu'a arriver a l'emplacement mÃ©moire
            if ((current->start+current->size) == seg->start){
                break;
            }
            prev = current;
            current = current->next;
        }
        //cas 1: full libre
        if (current->next->start == (current->start+current->size+seg->size)){
            Segment* tmp = current->next;
            current->size += seg->size+current->next->size;
            current->next = tmp->next;
            free(tmp);       
        }
        // cas 2: start++ ou start--
        else if(current->start == (seg->start+seg->size)){
            prev->next = seg;
        }
        //cas 3 chainage avant apres
        seg->next = current;
        return 1;
    }
    return 0;
}