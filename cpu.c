#include "CPU.h"

CPU* cpu_init(int memory_size) {
    int* val1 = (int*)malloc(sizeof(int));
    *val1 = 0;
    int* val2 = (int*)malloc(sizeof(int));
    *val2 = 0;
    int* val3 = (int*)malloc(sizeof(int));
    *val3 = 0;
    int* val4 = (int*)malloc(sizeof(int));
    *val4 = 0;
    int* val5 = (int*)malloc(sizeof(int));
    *val5 = 0;
    int* val6 = (int*)malloc(sizeof(int));
    *val6 = 0;
    int* val7 = (int*)malloc(sizeof(int));
    *val7 = 0;

    // Initialisation dynamique du CPU
    CPU* new = (CPU*)malloc(sizeof(CPU));
    new->memory_handler = memory_init(memory_size);
    new->context = hashmap_create();
    new->constant_pool = hashmap_create();

    // Initialisation de Hashmap
    HashMap_insert(new->context, "AX", val1);
    HashMap_insert(new->context, "BX", val2);
    HashMap_insert(new->context, "CX", val3);
    HashMap_insert(new->context, "DX", val4);
    HashMap_insert(new->context, "IP", val5);
    HashMap_insert(new->context, "ZF", val6);
    HashMap_insert(new->context, "SF", val7);

    return new;
}

void cpu_destroy(CPU* cpu) {
    if (!cpu) {
        fprintf(stderr, "ERREUR : CPU est NULL\n");
        return;
    }

    if (cpu->context) {
        hashmap_destroy(cpu->context);
    }

    if (cpu->memory_handler) {
        if (cpu->memory_handler->allocated) {
            hashmap_destroy(cpu->memory_handler->allocated);
        }

        if (cpu->memory_handler->free_list) {
            liberer_segments(cpu->memory_handler->free_list);
        }

        free(cpu->memory_handler);
        cpu->memory_handler = NULL;
    }

    if (cpu->constant_pool) {
        hashmap_destroy(cpu->constant_pool);
    }

    free(cpu);
    cpu = NULL;
}

void* store(MemoryHandler* handler , const char* segment_name, int pos, void* data){

    int clef = simple_hash(segment_name);
    Segment* seg = handler->allocated->table[clef].value;
    
    if (seg == NULL) {
        fprintf(stderr, "ERREUR: segment %s non trouve\n", segment_name);
        return;
    }

    if (pos >= seg->size) {
        fprintf(stderr, "ERREUR: position %d hors limites pour le segment %s\n", pos, segment_name);
        return;
    }

    // On stocke la valeur dans la memoire

    handler->memory[seg->start + pos] = data;

    return data;
}

void* load(MemoryHandler* handler, const char* segment_name, int pos){

    // On recherche le segent dans la table de hachage
    Segment* DS = HashMap_get(handler->allocated, segment_name);
    
    if (DS == NULL) {
        fprintf(stderr, "ERREUR: segment %s non trouve\n", segment_name);
        return;
    }

    if (pos >= DS->size) {
        fprintf(stderr, "ERREUR: position %d hors limites pour le segment %s\n", pos, segment_name);
        return;
    }

    // Renvoie la valeur stockee dans la memoire
    return handler->memory[DS->start + pos];
}

void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count){

    if (!cpu || !cpu->memory_handler || !data_instructions) {
        fprintf(stderr, "ERREUR: CPU ou instructions de données NULL\n");
        return;
    }

    for (int i = 0; i < data_count; i++) {
        Instruction* inst = data_instructions[i];
        if (inst && inst->mnemonic && inst->operand1 && inst->operand2) {
            // On alloue de la mémoire pour le segment de données
            int size = atoi(inst->operand1);
            int start = cpu->memory_handler->total_size; // Adresse de début du segment
            create_segment(cpu->memory_handler, inst->mnemonic, start, size);
            cpu->memory_handler->total_size += size; // Met à jour la taille totale de la mémoire
        }
    }
}

void print_data_segment(CPU *cpu) {
    // Vérifie si le segment "DS" existe
    Segment *ds_segment = (Segment *)hashmap_get(cpu->memory_handler->allocated, "DS");
    if (!ds_segment) {
        printf("Segment de données 'DS' non trouvé.\n");
        return;
    }

    printf("Contenu du segment de données 'DS' (Adresse de début : %d, Taille : %d)\n", 
           ds_segment->start, ds_segment->size);
    printf("Adresse | Valeur\n");
    printf("------- | -----\n");

    // Parcourt chaque position du segment
    for (int i = 0; i < ds_segment->size; i++) {
        int memory_address = ds_segment->start + i;
        void *data = cpu->memory_handler->memory[memory_address];

        if (data) {
            // Affiche la valeur pointée par data (cast en int* pour simplifier)
            printf("%-7d | %d\n", memory_address, *(int *)data);
        } else {
            printf("%-7d | NULL\n", memory_address);
        }
    }
}

int matches(const char* pattern, const char* str){

    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);

    if (result){
        fprintf(stderr, "Regex-compilation failed for pattern: %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

void* immediate_addressing(CPU* cpu, const char* operand){
    if (!cpu || !operand) {
        fprintf(stderr, "ERREUR: CPU ou operande NULL\n");
        return NULL;
    }

    // Vérifie si l'opérande est un nombre
    if (matches("^[0-9]+$", operand)) {
        int* value = (int*)malloc(sizeof(int));
        *value = atoi(operand);
        if(hashmap_get(cpu->constant_pool, value) == 1){
            HashMap_insert(cpu->constant_pool, operand, value);
            return value;
        }
    }

    fprintf(stderr, "ERREUR: operande %s non trouve\n", operand);
    return NULL;
}

void* register_addressing(CPU* cpu, const char* operand){

    if (!cpu || !operand){
        fprintf(stderr, "ERREUR: CPU ou operande NULL\n");
        return NULL;
    }

    if (matches("^[A-D][X]$", operand)){
        char* value  = strdup((char*)HashMap_get(cpu->context, operand)); //(char*) malloc(sizeof(char) * 2);
        if (value == NULL) {
            fprintf(stderr, "ERREUR: allocation de memoire echouee\n");
            return NULL;
        }        
    }
    return value;
}

void* memory_direct_addressing(CPU* cpu, const char* operand){

    if (!cpu || !operand){
        fprintf(stderr, "ERREUR: CPU ou operande NULL\n");
        return NULL;
    }

    if (matches("^\[[0-9]+\]$", operand)){
        int* value = atoi(operand + 1); //atoi(strdup(operand));
        if (value == NULL) {
            fprintf(stderr, "ERREUR: allocation de memoire echouee\n");
            return NULL;
        }
        Segment* segment = (Segment*)HashMap_get(cpu->memory_handler->allocated, operand);
        if ((!segment) && ((segment->start+value > segment->start) && (segment->start+value < segment->size-1))){
            return cpu->memory_handler->memory[segment->start + value];
        }
    }
    return NULL;
}

void* register_indirect_addressing(CPU* cpu, const char* operand){

    if (!cpu || ! operand){
        fprintf(stderr, "ERREUR: CPU ou operande NULL\n");
        return NULL;
    }

    Segment* segment = NULL;

    if (matches("[AX]|[BX]|[CX]|[DX]|[IP]|[ZF]|[SF]", operand)){
        char* value = strdup((char*)HashMap_get(cpu->context, operand));
        if (value == NULL) {
            fprintf(stderr, "ERREUR: allocation de memoire echouee\n");
            return NULL;
        }
        //???
        segment = (Segment*)HashMap_get(cpu->memory_handler->memory, value);
    }
    if (segment == NULL) {
        fprintf(stderr, "ERREUR: segment %s non trouve\n", value);
        return NULL;
    }
    return segment;
}

void handle_MOV(CPU* cpu, void* src, void* dest){

    
}

#include "cpu.h"

CPU* setup_cpu_environment() {
    // Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur : CPU initialization failed!\n");
        return NULL;
    }

    // Initialiser les registres avec des valeurs spécifiques
    cpu->bank->gp->eax = 1;
    cpu->bank->gp->ebx = 2;
    cpu->bank->gp->ecx = 3;
    cpu->bank->gp->edx = 4;
    cpu->bank->gp->esi = 5;
    cpu->bank->gp->edi = 6;
    cpu->bank->gp->esp = 7;
    cpu->bank->gp->ebp = 8;

    // Initialiser le segment de données
    if (cpu->bank->handler->load_segment(cpu->bank, "DS") < 0) {
        printf("Erreur : Failed to load data segment!\n");
        cpu_free(cpu);
        return NULL;
    }

    // Initialiser le segment de données avec des valeurs de test
    cpu->bank->memory[0] = 10;
    cpu->bank->memory[1] = 20;
    cpu->bank->memory[2] = 30;
    cpu->bank->memory[3] = 40;

    printf("CPU environment initialized.\n");
    return cpu;
}


int search_and_replace(char* str, HashMap values) {
    if (!str || !values) return 0;

    int replaced = 0;
    char* input = str;
    // Iterate through every key in the hashmap
    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].value) {
            char* key = values->table[i].key;
            int value = *((int*)values->table[i].value);

            // Find potential substring match
            char* match = strstr(input, key);
            if (!match) continue;

            // Construct replacement buffer
            char replacement[255];
            snprintf(replacement, sizeof(replacement), "%d", value);

            // Calculate lengths
            int key_len = strlen(key);
            int repl_len = strlen(replacement);

            // Allocate new string
            char* output = malloc(strlen(input) - key_len + repl_len + 1);
            strncpy(output, input, match - input);
            output[match - input] = '\0';
            strcat(output, replacement);
            strcat(output, match + key_len);

            // Free old input and update original string
            free(input);
            input = output;
            replaced = 1;
        }
    }

    // Trim the final string
    if (replaced) {
        char* trimmed = trim(input);
        memmove(input, trimmed, strlen(trimmed) + 1);
    }

    return replaced;
}