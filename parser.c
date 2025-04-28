#include "parser.h"

char* trim(char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = '\0';
    return str;
}

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
    free(buffer);
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
    free(etiquette);
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
    new_parser->data_instructions = (Instruction**) malloc(sizeof(Instruction*)*TABLE_SIZE);
    new_parser->code_instructions = (Instruction**) malloc(sizeof(Instruction*)*TABLE_SIZE);
    new_parser->memory_locations = hashmap_create();
    new_parser->labels = hashmap_create();

    while (fgets(line, 255, f) != NULL){
        // Supprime le saut de ligne
        line[strcspn(line, "\n")] = 0;

        // On verifie si on est dans la section .DATA ou .CODE
        if (strcmp(line, ".CODE") == 0){
            DorC = 0;
        }else if (strcmp(line, ".DATA") == 0){
            DorC = 1;
            printf("caca\n");
        }
        // On ajoute les instructions dans le tableau d'instructions correspondant
        else{    
            if (DorC == 1){
                printf("line : %s\n", line);
                new_parser->data_instructions[new_parser->data_count] = (parse_data_instruction(line, new_parser->memory_locations));
                new_parser->data_count++;              
            }else if (DorC == 0){      
                printf("line : %s\n", line);  
                new_parser->code_instructions[new_parser->code_count] = (parse_code_instruction(line, new_parser->labels, new_parser->code_count));
                new_parser->code_count++;
            }
        }
    }
    fclose(f);
    return new_parser;
}

void free_instruction(Instruction** Tabinst, int count) {

    // Le programme libère la mémoire allouée pour les champs de l'instruction

    for (int i = 0; i < count; i++){
        printf("numero instruction:%d\n",i);
        if (Tabinst[i] != NULL){
            free(Tabinst[i]->mnemonic);
            free(Tabinst[i]->operand1);
            free(Tabinst[i]->operand2);
        }
    }
    free(Tabinst);
}

void free_parser_result(ParserResult* result) {

    // Le programme vérifie si le résultat existe
    if (!result) return;

    // Le programme libère la mémoire des structures de données auxiliaires
    if (result->labels) {
        HashMap_destroy(result->labels);
        result->labels = NULL;
    }

    if (result->memory_locations) {
        HashMap_destroy(result->memory_locations);
        result->memory_locations = NULL;
    }

    // Le programme libère la mémoire allouée pour les instructions de la section DATA
    if (result->data_instructions) {
        free_instruction(result->data_instructions, result->data_count);
        result->data_instructions = NULL;
    }

    // Le programme libère la mémoire allouée pour les instructions de la section CODE
    if (result->code_instructions) {
        free_instruction(result->code_instructions, result->code_count);
        result->code_instructions = NULL;
    }

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
