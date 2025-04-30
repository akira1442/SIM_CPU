#ifndef CPU_H 
#define CPU_H 
#include "parser.h"
#include "MemoryHandler.h"
#include <regex.h>

typedef struct {
    MemoryHandler* memory_handler; //Gestionnaire de memoire
    HashMap *context; //Registres (AX, BX, CX ,DX)
    HashMap *constant_pool; //Table de hachage pour stocker les valeurs immediates
} CPU;


CPU* cpu_init(int memory_size);
void cpu_destroy(CPU* cpu);
void* store(MemoryHandler* handler, const char* segment_name, int pos, void* data);
void* load(MemoryHandler* handler, const char* segment_name, int pos);
void allocate_variables(CPU* cpu, Instruction** data_instructions, int data_count);
void print_data_segment(CPU* cpu);
void* immediate_addressing(CPU* cpu, const char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
int resolve_constants(ParserResult *result);
void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count);
int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest);
int execute_instruction(CPU *cpu, Instruction *instr);
int search_and_replace(char **str, HashMap *values);
void print_cpu_state(CPU *cpu);
int run_program(CPU *cpu);
int matches(const char* pattern, const char* str);

#endif