#include "MemoryHandler.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Initialisation du gestionnaire de mémoire avec une taille de 1024 unités
    MemoryHandler *handler = memory_init(1024);
    if (!handler) {
        fprintf(stderr, "Erreur lors de l'initialisation du gestionnaire de mémoire\n");
        return 1;
    }

    // Création de segments de mémoire
    if (!create_segment(handler, "Segment1", 0, 256)) {
        fprintf(stderr, "Erreur lors de la création du segment Segment1\n");
        memory_destroy(handler);
        return 1;
    }
    printf("Segment1 créé avec succès (start=0, size=256)\n");

    if (!create_segment(handler, "Segment2", 256, 128)) {
        fprintf(stderr, "Erreur lors de la création du segment Segment2\n");
        memory_destroy(handler);
        return 1;
    }
    printf("Segment2 créé avec succès (start=256, size=128)\n");

    // Suppression d'un segment de mémoire
    if (!remove_segment(handler, "Segment1")) {
        fprintf(stderr, "Erreur lors de la suppression du segment Segment1\n");
        memory_destroy(handler);
        return 1;
    }
    printf("Segment1 supprimé avec succès\n");

    // Vérification de la liste des segments libres
    Segment *current = handler->free_list;
    printf("Segments libres après suppression de Segment1 :\n");
    while (current) {
        printf("Segment libre - start: %d, size: %d\n", current->start, current->size);
        current = current->next;
    }

    // Libération des ressources
    memory_destroy(handler);
    printf("Gestionnaire de mémoire détruit avec succès\n");

    return 0;
}