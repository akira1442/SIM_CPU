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

int remove_segment(MemoryHandler *handler, const char *name) {
    // Retire l'élément de la table de hachage et vérifie si l'opération a réussi
    int result = hashmap_remove(handler->allocated, name);
    if (result == 0) { // Si la suppression échoue
        printf("Segment non trouvé\n");
        return 0;
    }

    // Parcourt la table de hachage pour retrouver le segment
    unsigned long index = simple_hash(name) % handler->allocated->size; // Calcule l'index avec hachage
    HashEntry entry = handler->allocated->table[index]; // Accède à l'entrée de la table

    // Vérifie si l'entrée est valide
    if (entry.key == NULL || strcmp(entry.key, name) != 0) {
        printf("Segment avec le nom '%s' non trouvé dans la table allouée\n", name);
        return 0;
    }

    // Récupère le segment associé à l'entrée de la table
    Segment *segment = (Segment *)entry.value; // Cast de la valeur en Segment

    // Ajoute le segment dans la liste des segments libres
    Segment *prev = NULL;
    Segment *current = handler->free_list;

    // Trouve la position où insérer le segment dans free_list
    while (current && current->start < segment->start) {
        prev = current;
        current = current->next;
    }

    // Fusionne avec le segment libre précédent si adjacent
    if (prev && (prev->start + prev->size == segment->start)) {
        prev->size += segment->size;
        free(segment); // Libère le segment fusionné
        segment = prev;
    } else {
        segment->next = current;
        if (prev) {
            prev->next = segment;
        } else {
            handler->free_list = segment;
        }
    }

    // Fusionne avec le segment libre suivant si adjacent
    if (current && (segment->start + segment->size == current->start)) {
        segment->size += current->size;
        segment->next = current->next;
        free(current);
    }

    return 1;
}