#include "parser.h"

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations){

    int adresse = 0;
    char* buffer = strdup(line);
    buffer = strtok(buffer, " ");

    Instruction* res = (Instruction*)malloc(sizeof(Instruction));
    res->mnemonic = NULL;
    res->operand1 = NULL;
    res->operand2 = NULL;

    while(buffer != NULL){
        // Initialisation des champs d'instructions

        if (res->mnemonic == NULL) {
            res->mnemonic = strdup(buffer);
            //printf("buffer : %s\n", buffer);
        } else if (res->operand1 == NULL) {
            res->operand1 = strdup(buffer);
        } else if (res->operand2 == NULL) {
            res->operand2 = strdup(buffer);
        }
        buffer = strtok(NULL, " ");
    }

    // Initialisation de l'adresse mémoire
    if (res->operand2 == buffer) {
        while ((buffer = strtok(NULL, ","))) {
            // On vérifie si on a pas atteint la fin de operand2
            adresse++;
        }
    }

    // On ajoute l'adresse mémoire dans la table de hachage
    HashMap_insert(memory_locations, res->mnemonic, &adresse);
    return res;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count){
    
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));
    res->mnemonic = NULL;
    res->operand1 = NULL;
    res->operand2 = NULL;

    char* buffer = strdup(line);

    buffer = strtok(buffer, ": ");
    char* etiquette;

    // On verifie si ':' est dans la ligne si oui regarder le 1er mot
    if (strchr(line, ':') != NULL){ 
        etiquette = strtok(strdup(line), ":");
        //buffer[strcspn(line, ":")] = 0;
        HashMap_insert(labels, etiquette, &code_count);
        buffer = strtok(NULL, " ");
    }

    while(buffer != NULL){
        // Initialisation des champs d'instructions

        if (res->mnemonic == NULL) {
            res->mnemonic = strdup(buffer);
        } else if (res->operand1 == NULL) {
            res->operand1 = strdup(buffer);
        } else if (res->operand2 == NULL) {
            res->operand2 = strdup(buffer);
        }
        buffer = strtok(NULL, ", ");
    }

    return res;
}

ParserResult *parse(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Erreur lecture fichier\n");
        return NULL;
    }

    ParserResult *res = (ParserResult *)malloc(sizeof(ParserResult));
    res->data_instructions = NULL;
    res->data_count = 0;
    res->code_instructions = NULL;
    res->code_count = 0;
    res->labels = hashmap_create();
    res->memory_locations = hashmap_create();

    char line[256];
    int in_data = 0, in_code = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, ".DATA", 5) == 0) {
            in_data = 1;
            in_code = 0;
            continue;
        }
        if (strncmp(line, ".CODE", 5) == 0) {
            in_data = 0;
            in_code = 1;
            continue;
        }

        if (in_data) {
            res->data_count++;
            res->data_instructions = (Instruction **)realloc(res->data_instructions, res->data_count * sizeof(Instruction *));
            res->data_instructions[res->data_count - 1] = parse_data_instruction(line, res->memory_locations);
        } else if (in_code) {
            res->code_count++;
            res->code_instructions = (Instruction **)realloc(res->code_instructions, res->code_count * sizeof(Instruction *));
            res->code_instructions[res->code_count - 1] = parse_code_instruction(line, res->labels, res->code_count);
        }
    }

    fclose(f);
    return res;
}

void free_parser_result(ParserResult* result) {

    if (result == NULL) return;

    // Il faut supprimer les mnemonic de data_instructions qui sont dans memory_locations avant de les libérer
    if (result->data_instructions) {
        for (int i = 0; i < result->data_count; i++){
            HashMap_remove(result->memory_locations, result->data_instructions[i]->mnemonic);
            free(result->data_instructions[i]->mnemonic);
            free(result->data_instructions[i]->operand1);
            free(result->data_instructions[i]->operand2);
        }
        free(result->data_instructions);
    }

    if (result->code_instructions){
        for (int i = 0; i < result->code_count; i++){
            free(result->code_instructions[i]->mnemonic);
            free(result->code_instructions[i]->operand1);
            free(result->code_instructions[i]->operand2);
        }
        free(result->code_instructions);
    }

    HashMap_destroy(result->labels);
    HashMap_destroy(result->memory_locations);

    // On peut libérer result
    free(result);
}

void afficher_instructions(Instruction** instructions, int count) {
    for (int i = 0; i < count; ++i) {
        if (instructions[i] != NULL) {
            printf("Instruction %d: Mnemonic: %s Operand1: %s Operand2: %s\n", 
                    i, instructions[i]->mnemonic, instructions[i]->operand1, instructions[i]->operand2);
        }
    }
}

void afficher_parser_result(ParserResult* result) {
    if (result == NULL) {
        printf("ParserResult est NULL.\n");
        return;
    }

    printf("=== ParserResult ===\n");

    printf("\nInstructions .DATA:\n");
    afficher_instructions(result->data_instructions, result->data_count);

    printf("\nInstructions .CODE:\n");
    afficher_instructions(result->code_instructions, result->code_count);

    printf("\nLabels (indices dans code_instructions):\n");
    afficher_hashmap(result->labels);

    printf("\nEmplacements mémoire (noms de variables -> adresse mémoire):\n");
    afficher_hashmap(result->memory_locations);
    
    printf("\n=====================\n");
}
