#include "parser.h"

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations){

    int adresse = 0;
    char* buffer = strtok(line, " ");

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
            while (buffer = strtok(NULL, " ")){
                // On verifie si on a pas atteint la fin de operand2 
                adresse++;
            }
        }
        buffer = strtok(NULL, " ");
    }

    HashMap_insert(memory_locations, res->mnemonic, adresse);
    return res;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count){
    
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));

    if (fgets(buffer, sizeof(buffer), stdin)){
        buffer[strcspn(buffer, '\n')] = '\0';
    }

    char* tmp = strtok(buffer, ", ");
    char*  etiquette = NULL;

    while (tmp != NULL){
        // verif si ':' est dans la ligne si oui regarder le 1er mot
        if (strchr(buffer, ":") && (etiquette = strtok(tmp, ":"))){
            HashMap_insert(labels, etiquette, code_count);
            tmp = strtok(NULL, ", ");
        }
        if (res->mnemonic == NULL){
            strdup(res->mnemonic, tmp);
        }else if (res->operand1 == NULL){
            strdup(res->operand1, tmp);
        }else if (res->operand2 == NULL){
            strdup(res->operand2, tmp);
        }
        tmp = strtok(NULL, ", ");
    }
    return res;
}

ParserResult* parse(const_char* filename){

    FILE* f = fopen(filename, "r");
    const char* line[255];
    fgets(line, 255, f);

    ParserResult* new_parser = (ParserResult*) malloc(sizeof(ParserResult));
    new_parser->data_count = 0;
    new_parser->code_count = 0;
    new_parser->data_instructions = (Instruction**) malloc(sizeof(Instruction*));
    new_parser->code_instructions = (Instruction**) malloc(sizeof(Instruction*));
    new_parser->labels = (HashMap*)malloc(sizeof(HashMap));

    while (line != NULL){
        if (strcmp(line, ".DATA") == 0){

            new_parser->data_instructions=parse_data_instruction(line, new_parser->memory_locations);
            new_parser->data_count++;                
        }else if (strcmp(line, ".CODE") == 0){        
            while(strcmp(line, ".DATA") == 1){
                new_parser->code_instructions=parse_code_instruction(line, new_parser->labels,new_parser->code_count);
                new_parser->code_count++;
            }
        }
        fgets(line, 255, f);
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