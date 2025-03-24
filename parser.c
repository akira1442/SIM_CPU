#include "parser.h"

Instruction* parse_data_instruction(const char* line, HashMap* memory_locations){

    int i = 0;
    int j = 0;
    char* buffer[255]="";
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));

    while(line[i] != '\0'){
        //Dès qu'on recontre un espace on envoie la valeur de buffer dans:
        if (line[i]==" "){ 
            //mnemonic si c'est le premier "mot"
            if (!res->mnemonic){  
                res->mnemonic=buffer;
                buffer="";
                j = 0;
            }
            //operand1 si c'est le deuxieme "mot"
            else if (!res->operand1){ 
                res->operand1=buffer;
                buffer="";
                j = 0;
            }
            
        }
        buffer[j]+=line[i];
        i++;
        j++;
    }
    // On doit vérifier si le buffer est vide
    // Si il ne l'est pas on doit l'ajouter à operand2
    if ((!res->operand2) && (strlen(buffer)!=0)){ //operand2 si c'est de troisième mot 
        res->operand2=buffer;
    }

    HashMap_insert(memory_locations, res->mnemonic, res->operand2);
    return res;
}

Instruction* parse_code_instruction(const char* line, HashMap* labels, int code_count){
    
    Instruction* res = (Instruction*)malloc(sizeof(Instruction));

    if (fgets(buffer, sizeof(buffer), stdin)){
        buffer[strcspn(buffer, '\n')] = '\0';
    }

    char* tmp = strtok(buffer, ", ");

    while ((tmp = strtok(NULL, ", ")) != NULL){
        if (strcmp(tmp, "loop:") == 0){

        }
        else{

        }
    }
    
}