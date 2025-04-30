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

ParserResult* parse(const char* filename){

    FILE* f = fopen(filename, "r");
    char line[255];
    
    // boolean 1 si on pour si .DATA 0 si .CODE
    int DorC = 1;

    ParserResult* new_parser = (ParserResult*) malloc(sizeof(ParserResult));
    new_parser->data_count = 0;
    new_parser->code_count = 0;
    new_parser->data_instructions = NULL; //(Instruction**) malloc(sizeof(Instruction*));
    new_parser->code_instructions = NULL; //(Instruction**) malloc(sizeof(Instruction*));
    new_parser->memory_locations = hashmap_create();
    new_parser->labels = hashmap_create();

    while (fgets(line, 255, f) != NULL){
        // Supprime le saut de ligne
        line[strcspn(line, "\n")] = 0;

        // On verifie si on est dans la section .DATA ou .CODE
        if (strcmp(line, ".CODE") == 0){
            DorC = 0;
            continue;
        }
        else if (strcmp(line, ".DATA") == 0){
            DorC = 1;
            printf("caca\n");
            continue;
        }
        // On ajoute les instructions dans le tableau d'instructions correspondant
        
        Instruction** new_inst = NULL;
        if (DorC == 1){
            //printf("line : %s\n", line);
            new_inst = (Instruction**) realloc(new_parser->data_instructions, (new_parser->data_count+1)*sizeof(Instruction*));
            new_parser->data_instructions = new_inst;
            new_parser->data_instructions[new_parser->data_count] = parse_data_instruction(line, new_parser->memory_locations);
            new_parser->data_count++;              
        }else if (DorC == 0){      
            //printf("line : %s\n", line);  
            new_inst = (Instruction**) realloc(new_parser->code_instructions, (new_parser->code_count+1) * sizeof(Instruction*));
            new_parser->code_instructions = new_inst;
            new_parser->code_instructions[new_parser->code_count] = parse_code_instruction(line, new_parser->labels, new_parser->code_count);
            new_parser->code_count++;
        }
    }
    fclose(f);
    return new_parser;
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
