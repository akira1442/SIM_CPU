#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Création d'une table de hachage
    HashMap *map = hashmap_create();
    if (!map) {
        fprintf(stderr, "Erreur lors de la création de la table de hachage\n");
        return 1;
    }

    // Insertion de paires clé-valeur
    char *key1 = "clé1";
    int value1 = 42;
    char *key2 = "clé2";
    int value2 = 123;
    char *key3 = "clé3";
    int value3 = 789;

    if (!HashMap_insert(map, key1, &value1)) {
        fprintf(stderr, "Erreur lors de l'insertion de %s\n", key1);
        HashMap_destroy(map);
        return 1;
    }
    printf("Inséré: %s -> %d\n", key1, value1);

    if (!HashMap_insert(map, key2, &value2)) {
        fprintf(stderr, "Erreur lors de l'insertion de %s\n", key2);
        HashMap_destroy(map);
        return 1;
    }
    printf("Inséré: %s -> %d\n", key2, value2);

    if (!HashMap_insert(map, key3, &value3)) {
        fprintf(stderr, "Erreur lors de l'insertion de %s\n", key3);
        HashMap_destroy(map);
        return 1;
    }
    printf("Inséré: %s -> %d\n", key3, value3);

    // Récupération de valeurs
    int *retrieved_value1 = (int *)HashMap_get(map, key1);
    if (retrieved_value1 && *retrieved_value1 == value1) {
        printf("Récupéré: %s -> %d (OK)\n", key1, *retrieved_value1);
    } else {
        fprintf(stderr, "Erreur lors de la récupération de %s\n", key1);
        HashMap_destroy(map);
        return 1;
    }

    int *retrieved_value2 = (int *)HashMap_get(map, key2);
    if (retrieved_value2 && *retrieved_value2 == value2) {
        printf("Récupéré: %s -> %d (OK)\n", key2, *retrieved_value2);
    } else {
        fprintf(stderr, "Erreur lors de la récupération de %s\n", key2);
        HashMap_destroy(map);
        return 1;
    }

    // Suppression d'une entrée
    if (!HashMap_remove(map, key2)) {
        fprintf(stderr, "Erreur lors de la suppression de %s\n", key2);
        HashMap_destroy(map);
        return 1;
    }
    printf("Supprimé: %s\n", key2);

    // Vérification de la suppression
    if (HashMap_get(map, key2)) {
        fprintf(stderr, "Erreur: %s devrait avoir été supprimé\n", key2);
        HashMap_destroy(map);
        return 1;
    }

    // Affichage de la table de hachage
    afficher_hashmap(map);

    // Destruction de la table de hachage
    HashMap_destroy(map);
    printf("Table de hachage détruite avec succès\n");

    return 0;
}