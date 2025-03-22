#include "parser.h"

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations){

    int i = 0;
    int j = 0;
    char* buffer[255]="";
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));
    while(line[i] != '\0'){
        if (line[i]==" "){ //Dès qu'on recontre un espace on envoie la valeur de buffer dans:
            if (!res->mnemonic){  //mnemonic si c'est le premier "mot"
                res->mnemonic=buffer;
                buffer="";
                j = 0;
            }
            else if (!res->operand1){ //operand1 si c'est le deuxieme "mot"
                res->operand1=buffer;
                buffer="";
                j = 0;
            }
            
        }
        buffer[j]+=line[i];
        i++;
        j++;
    }
    if ((!res->operand2) && (strlen(buffer)!=0)){ //operand2 si c'est de troisième mot 
        res->operand2=buffer;
    }
    HashMap_insert(memory_locations, res->mnemonic, res->operand2);
    return res;
}