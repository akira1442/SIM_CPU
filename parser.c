#include "parser.h"

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations){

    int adresse = 0;
    char* buffer = strdup(line);
    buffer = strtok(buffer, " ");

    Instruction* res = (Instruction*)malloc(sizeof(Instruction));

    while(buffer){
        //mnemonic si c'est le premier "mot"
        if (!res->mnemonic){  
            res->mnemonic=buffer;
        }
        //operand1 si c'est le deuxieme "mot"
        else if (!res->operand1){ 
            res->operand1=buffer;
        }
        //operand2 si c'est le troisieme "mot"
        else if (!res->operand2){
            res->operand2=buffer;
            while ((buffer = strtok(NULL, ","))){
                // On verifie si on a pas atteint la fin de operand2 
                adresse++;
            }
        }
        buffer = strtok(NULL, " ");
    }

    HashMap_insert(memory_locations, res->mnemonic, &adresse);
    return res;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count){
    
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));

    char* buffer = strdup(line);

    char* tmp = strtok(buffer, ", ");
    char*  etiquette = NULL;

    while (tmp != NULL){
        // verif si ':' est dans la ligne si oui regarder le 1er mot
        if (strchr(buffer, ':') && (etiquette = strtok(tmp, ":"))){
            HashMap_insert(labels, etiquette, &code_count);
            tmp = strtok(NULL, ", ");
        }
        if (res->mnemonic == NULL){
            tmp=strdup(res->mnemonic);
        }else if (res->operand1 == NULL){
            tmp =strdup(res->operand1);
        }else if (res->operand2 == NULL){
            tmp=strdup(res->operand2);
        }
        tmp = strtok(NULL, ", ");
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
    new_parser->data_instructions = (Instruction**) malloc(sizeof(Instruction*));
    new_parser->code_instructions = (Instruction**) malloc(sizeof(Instruction*));
    new_parser->labels = (HashMap*)malloc(sizeof(HashMap));

    while (fgets(line, 255, f)){
        if ((strcmp(line, ".DATA") == 0) && DorC){
            new_parser->data_instructions[new_parser->data_count] = (parse_data_instruction(line, new_parser->memory_locations));
            new_parser->data_count++;   
            DorC = 1;             
        }else if ((strcmp(line, ".CODE") == 0) && !DorC){        
            new_parser->code_instructions[new_parser->code_count] = (parse_code_instruction(line, new_parser->labels,new_parser->code_count));
            new_parser->code_count++;
            DorC = 0;
        }
        //fgets(line, 255, f);
    }
    
    fclose(f);
    return new_parser;
}

void free_instruction(Instruction* inst){
    if(inst) return;

    free((inst)->mnemonic);
    free((inst)->operand1);
    free((inst)->operand2);
    free(inst);
}

void free_parser_result(ParserResult* result){

    if (!result){
        return;
    }

    // On libere .DATA
    if (result->data_instructions){
        int i = 0;
        while(i < result->data_count){
            if (result->data_instructions[i]){

            }
        }
    }
}

void afficher_instructions(Instruction** instructions, int count) {
    for (int i = 0; i < count; ++i) {
        if (instructions[i] != NULL) {
            printf("Instruction %d: Mnemonic: %s, Operand1: %s, Operand2: %s\n", 
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
