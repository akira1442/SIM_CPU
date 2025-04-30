#include "cpu.h"

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
    if ((!new) || (!new->memory_handler) || (!new->context)){
        fprintf(stderr,"ERREUR ALLOCATION MEMOIRE");
    }

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
        HashMap_destroy(cpu->context);
    }

    if (cpu->memory_handler) {
        if (cpu->memory_handler->allocated) {
            HashMap_destroy(cpu->memory_handler->allocated);
        }

        if (cpu->memory_handler->free_list) {
            liberer_segments(cpu->memory_handler->free_list);
        }

        free(cpu->memory_handler);
        cpu->memory_handler = NULL;
    }

    if (cpu->constant_pool) {
        HashMap_destroy(cpu->constant_pool);
    }

    free(cpu);
    cpu = NULL;
}

void* store(MemoryHandler* handler , const char* segment_name, int pos, void* data){

    Segment* seg = HashMap_get(handler->allocated, segment_name);

    if (!seg){
        fprintf(stderr, "ERREUR: segment %s non trouve\n", segment_name);
        return NULL;
    }
    handler->memory[seg->start + pos] = data;
    return data;
}

void* load(MemoryHandler* handler, const char* segment_name, int pos){

    Segment* seg = HashMap_get(handler->allocated, segment_name);

    if (!seg || pos >= seg->size){
        return NULL;
    }
    return handler->memory[seg->start];
}

void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count){

    if (!cpu || !cpu->memory_handler || !data_instructions) {
        fprintf(stderr, "ERREUR: cpu ou instructions de données NULL\n");
        return;
    }

    create_segment(cpu->memory_handler, "DS", 0, data_count);

    for(int i = 0; i < data_count; i++){
        store(cpu->memory_handler, "DS", i, data_instructions[i]->operand2);
    }
}

void print_data_segment(CPU *cpu) {
    
    Segment *ds = (Segment *)HashMap_get(cpu->memory_handler->allocated, "DS");
    if (!ds) {
        printf("Segment de données 'DS' non trouvé.\n");
        return;
    }

    printf("Contenu du segment de données 'DS' (Adresse de début : %d, Taille : %d)\n", ds->start, ds->size);
    printf("Adresse | Valeur\n");
    printf("------- | -----\n");

    // Parcourt chaque position du segment
    for (int i = 0; i < ds->size; i++) {
        int memory_address = ds->start + i;
        void *data = cpu->memory_handler->memory[memory_address];

        if (data) {
            // Affiche la valeur pointée par data (cast en int* pour simplifier)
            printf("%d | %d\n", memory_address, *(int *)data);
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
        char* value = (char*)malloc(sizeof(char));
        *value = atoi(operand);
        if(HashMap_get(cpu->constant_pool, value)){
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

    char* value = NULL;

    if (matches("^[A-D][X]$", operand)){
        value  = strdup((char*)HashMap_get(cpu->context, operand)); //(char*) malloc(sizeof(char) * 2);
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

    char* value = NULL;
    
    if (matches("^\\[[0-9]+\\]$" , operand)){
        value = strdup(operand+1);
        if (value == NULL) {
            fprintf(stderr, "ERREUR: allocation de memoire echouee\n");
            return NULL;
        }
        int v = atoi(value);
        Segment* segment = (Segment*)HashMap_get(cpu->memory_handler->allocated, operand);
        if ((!segment) && ((segment->start+v > segment->start) && (segment->start+v < segment->size-1))){
            return cpu->memory_handler->memory[segment->start + v];
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
    char* value = NULL;
    if (matches("[AX]|[BX]|[CX]|[DX]|[IP]|[ZF]|[SF]", operand)){
        value = strdup((char*)HashMap_get(cpu->context, operand));
        if (value == NULL) {
            fprintf(stderr, "ERREUR: allocation de memoire echouee\n");
            return NULL;
        }
        segment = (Segment*)HashMap_get(cpu->memory_handler->allocated, value);
    }
    if (segment == NULL) {
        fprintf(stderr, "ERREUR: segment %s non trouve\n", value);
        return NULL;
    }
    return segment;

}

void handle_MOV(CPU* cpu, void* src, void* dest) {
    // On vérifie si les pointeurs sont valides
    if (src == NULL || dest == NULL) {
        fprintf(stderr, "ERREUR: Pointeurs invalides pour handle_MOV\n");
        return;
    }

    // On copie la valeur de la source vers la destination
    int src_value = *(int *)src;
    *(int *)dest = src_value;

    // Mise à jour des drapeaux ZF et SF en fonction de la valeur copiée
    int* zf = (int *)HashMap_get(cpu->context, "ZF");
    int* sf = (int *)HashMap_get(cpu->context, "SF");

    if (src_value == 0) {
        *zf = 1; // Le résultat est zéro
    } else {
        *zf = 0;
    }

    if (src_value < 0) {
        *sf = 1; // Le résultat est négatif
    } else {
        *sf = 0;
    }
}


CPU* setup_cpu_environment() {
    // Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if (!cpu) {
        printf("Erreur : Initialisation du CPU échouée\n");
        return NULL;
    }

    // Initialiser les registres avec des valeurs spécifiques
    int* ax = (int *)HashMap_get(cpu->context, "AX");
    int* bx = (int *)HashMap_get(cpu->context, "BX");
    int* cx = (int *)HashMap_get(cpu->context, "CX");
    int* dx = (int *)HashMap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    // Créer et initialiser le segment de données
    if (!HashMap_get(cpu->memory_handler->allocated, "DS")) {
        create_segment(cpu->memory_handler, "DS", 0, 20);

        // Initialiser le segment de données avec des valeurs de test
        for (int i = 0; i < 10; i++) {
            int* value = (int *)malloc(sizeof(int));
            *value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
            store(cpu->memory_handler, "DS", i, value);
        }
    }
    printf("Test environment initialized.\n");
    return cpu;
}

//EXERCICE 6 

char *trim(char *str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) 
    {
        *end = '\0';
        end--;
    }
    return str;
}

int search_and_replace(char **str, HashMap *values) {
    if (!str || !*str || !values) return 0;

    int replaced = 0;
    char *input = *str;

    // Iterate through all keys in the hashmap
    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].value != (void *)-1) {
            char *key = values->table[i].key;
            int value = (int)(long)values->table[i].value;

            // Find potential substring match
            char *substr = strstr(input, key);
            if (substr) {
                // Construct replacement buffer
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                // Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                int remain_len = strlen(substr + key_len);

                // Create new string
                char *new_str = (char *)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                // Free and update original string
                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    // Trim the final string
    if (replaced) {
        char *trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}

void* resolve_addressing(CPU *cpu, const char *operand) {
    if (!cpu || !operand) {
        fprintf(stderr, "ERREUR: CPU ou opérande NULL\n");
        return NULL;
    }

    // 1. Adressage immédiat
    void *value = immediate_addressing(cpu, operand);
    if (value != NULL) {
        return value;
    }

    // 2. Adressage par registre
    value = register_addressing(cpu, operand);
    if (value != NULL) {
        return value;
    }

    // 3. Adressage indirect par registre
    value = register_indirect_addressing(cpu, operand);
    if (value != NULL) {
        return value;
    }

    // 4. Adressage direct
    value = memory_direct_addressing(cpu, operand);
    if (value != NULL) {
        return value;
    }

    // Si aucun mode d'adressage n'est trouvé
    fprintf(stderr, "ERREUR: Mode d'adressage non reconnu pour l'opérande '%s'\n", operand);
    return NULL;
}


int resolve_constants(ParserResult *result) {
    // Parcourir toutes les instructions .CODE
    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];

        // Remplacer les variables et étiquettes dans les opérandes
        if (instr->operand1) {
            // Remplacer dans operand1
            char *new_operand1 = strdup(instr->operand1);
            search_and_replace(&new_operand1, result->memory_locations);
            search_and_replace(&new_operand1, result->labels);
            free(instr->operand1);
            instr->operand1 = new_operand1;
        }

        if (instr->operand2) {
            // Remplacer dans operand2
            char *new_operand2 = strdup(instr->operand2);
            search_and_replace(&new_operand2, result->memory_locations);
            search_and_replace(&new_operand2, result->labels);
            free(instr->operand2);
            instr->operand2 = new_operand2;
        }
    }
    return 1; // Indiquer la réussite
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count) {
    if (!cpu || !code_instructions) {
        fprintf(stderr, "ERREUR: CPU ou code_instructions est NULL\n");
        return;
    }

    // Calculer la taille totale nécessaire pour stocker les instructions
    int total_size = code_count * sizeof(Instruction*);

    // Allouer un segment de mémoire pour les instructions
    create_segment(cpu->memory_handler, "CS", 0, total_size);

    // Stocker les instructions dans le segment de code
    for (int i = 0; i < code_count; i++) {
        Instruction *instr = code_instructions[i];
        if (instr) {
            // Convertir l'instruction en une chaîne de caractères pour la stocker
            char *instruction_str = (char *)malloc(256 * sizeof(char));
            snprintf(instruction_str, 256, "%s %s %s", 
                    instr->mnemonic ? instr->mnemonic : "",
                    instr->operand1 ? instr->operand1 : "",
                    instr->operand2 ? instr->operand2 : "");

            // Stocker la chaîne dans le segment de code
            store(cpu->memory_handler, "CS", i * sizeof(char *), &instruction_str);
        }
    }
    // Initialiser le registre IP pour pointer sur le début du segment de code
    int *ip = (int *)HashMap_get(cpu->context, "IP");
    if (ip) {
        *ip = 0;
    }
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest) {
    if (!cpu || !instr) {
        fprintf(stderr, "ERREUR: CPU ou Instruction est NULL\n");
        return -1;
    }

    // Récupérer les valeurs des opérandes
    void *src_value = resolve_addressing(cpu, instr->operand1);
    void *dest_value = resolve_addressing(cpu, instr->operand2);

    if (!src_value || !dest_value) {
        fprintf(stderr, "ERREUR: Impossible de résoudre les adresses des opérandes\n");
        return -1;
    }

    // Comparaison pour déterminer le type d'instruction et exécuter la bonne opération
    if (strcmp(instr->mnemonic, "MOV") == 0) {
        // Instruction MOV : copie la valeur de src vers dest
        int *src_int = (int *)src_value;
        int *dest_int = (int *)dest_value;
        *dest_int = *src_int;

        // Mettre à jour les drapeaux ZF et SF
        int *zf = (int *)HashMap_get(cpu->context, "ZF");
        int *sf = (int *)HashMap_get(cpu->context, "SF");
        *zf = (*dest_int == 0) ? 1 : 0;
        *sf = (*dest_int < 0) ? 1 : 0;

    } else if (strcmp(instr->mnemonic, "ADD") == 0) {
        // Instruction ADD : additionne src à dest
        int *src_int = (int *)src_value;
        int *dest_int = (int *)dest_value;
        *dest_int += *src_int;

        // Mettre à jour les drapeaux ZF et SF
        int *zf = (int *)HashMap_get(cpu->context, "ZF");
        int *sf = (int *)HashMap_get(cpu->context, "SF");
        *zf = (*dest_int == 0) ? 1 : 0;
        *sf = (*dest_int < 0) ? 1 : 0;

    } else if (strcmp(instr->mnemonic, "CMP") == 0) {
        // Instruction CMP : compare src et dest
        int *src_int = (int *)src_value;
        int *dest_int = (int *)dest_value;
        int *zf = (int *)HashMap_get(cpu->context, "ZF");
        int *sf = (int *)HashMap_get(cpu->context, "SF");
        *zf = (*dest_int == *src_int) ? 1 : 0;
        *sf = (*dest_int < *src_int) ? 1 : 0;

    } else if (strcmp(instr->mnemonic, "JMP") == 0) {
        // Instruction JMP : sauter à l'adresse spécifiée
        int *dest_int = (int *)dest_value;
        int *ip = (int *)HashMap_get(cpu->context, "IP");
        *ip = *dest_int;

    } else if (strcmp(instr->mnemonic, "JZ") == 0) {
        // Instruction JZ : sauter si ZF est 1
        int *zf = (int *)HashMap_get(cpu->context, "ZF");
        if (*zf == 1) {
            int *dest_int = (int *)dest_value;
            int *ip = (int *)HashMap_get(cpu->context, "IP");
            *ip = *dest_int;
        }

    } else if (strcmp(instr->mnemonic, "JNZ") == 0) {
        // Instruction JNZ : sauter si ZF est 0
        int *zf = (int *)HashMap_get(cpu->context, "ZF");
        if (*zf == 0) {
            int *dest_int = (int *)dest_value;
            int *ip = (int *)HashMap_get(cpu->context, "IP");
            *ip = *dest_int;
        }

    } else if (strcmp(instr->mnemonic, "HALT") == 0) {
        // Instruction HALT : terminer l'exécution
        int *ip = (int *)HashMap_get(cpu->context, "IP");
        *ip = -1;

    } else {
        // Instruction non supportée
        fprintf(stderr, "ERREUR: Instruction non supportée : %s\n", instr->mnemonic);
        return -1;
    }

    return 0;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!cpu || !instr) {
        fprintf(stderr, "ERREUR: CPU ou Instruction est NULL\n");
        return -1;
    }

    // Résoudre les adresses des opérandes
    void *src = resolve_addressing(cpu, instr->operand1);
    void *dest = resolve_addressing(cpu, instr->operand2);

    if (!src || !dest) {
        fprintf(stderr, "ERREUR: Échec de la résolution des adresses des opérandes\n");
        return -1;
    }

    // Exécuter l'instruction
    return handle_instruction(cpu, instr, src, dest);
}

Instruction* fetch_next_instruction(CPU* cpu) {
    
    if (!cpu) return NULL;

    int* ip = hashmap_get(cpu->context, "IP");
    
    if (!ip || *ip == -1) return NULL;

    Segment* cs = hashmap_get(cpu->memory_handler->allocated, "CS");
    if (!cs || *ip >= cs->size) return NULL;

    Instruction* instr = load(cpu->memory_handler, "CS", *ip);
    (*ip)++;
    return instr;
}

//CETTE FONCTION N'EST PAS DEMANDÉ D'ORIGINE
void print_cpu_state(CPU *cpu) {
    // Affiche l'état du CPU (registres et segment de données)
    printf("\nÉtat actuel du CPU :\n");

    // Afficher les registres
    printf("Registres :\n");
    printf("AX: %d, BX: %d, CX: %d, DX: %d\n", 
           *(int *)HashMap_get(cpu->context, "AX"),
           *(int *)HashMap_get(cpu->context, "BX"),
           *(int *)HashMap_get(cpu->context, "CX"),
           *(int *)HashMap_get(cpu->context, "DX"));
    printf("IP: %d, ZF: %d, SF: %d\n", 
           *(int *)HashMap_get(cpu->context, "IP"),
           *(int *)HashMap_get(cpu->context, "ZF"),
           *(int *)HashMap_get(cpu->context, "SF"));

    // Afficher le segment de données
    print_data_segment(cpu);
}



int run_program(CPU *cpu) {
    
    if (!cpu) {
        fprintf(stderr, "ERREUR: CPU est NULL\n");
        return 0;
    }

    printf("Début de l'exécution du programme.\n");
    print_cpu_state(cpu);

    char input[10];
    while (1) {
        printf("\nAppuyez sur Entrée pour exécuter l'instruction suivante, ou 'q' pour quitter : ");
        scanf("%9s", input);

        if (strcmp(input, "q") == 0) {
            printf("Programme interrompu par l'utilisateur.\n");
            break;
        }

        Instruction *instr = fetch_next_instruction(cpu);
        if (!instr) {
            printf("Fin du programme.\n");
            break;
        }

        printf("Exécution de l'instruction : %s %s %s\n", 
               instr->mnemonic ? instr->mnemonic : "",
               instr->operand1 ? instr->operand1 : "",
               instr->operand2 ? instr->operand2 : "");

        // Résoudre les adresses des opérandes et exécuter l'instruction
        int result = execute_instruction(cpu, instr);
        if (result != 0) {
            fprintf(stderr, "ERREUR: Échec de l'exécution de l'instruction\n");
            break;
        }

        print_cpu_state(cpu);
    }

    return 0;
}