#include "parser.h"

int main() {
    // Parsing d'un fichier assembleur
    ParserResult* result = parse("test.txt");
    if (!result) {
        fprintf(stderr, "Erreur lors du parsing du fichier\n");
        return 1;
    }

    // Affichage des résultats du parsing
    afficher_parser_result(result);

    // Vérification des résultats (exemple de vérification)
    if (result->data_count > 0 && result->code_count > 0) {
        printf("Nombre d'instructions .DATA : %d\n", result->data_count);
        printf("Nombre d'instructions .CODE : %d\n", result->code_count);

        // Vérifier les premières instructions .DATA
        Instruction* data_inst = result->data_instructions[0];
        if (data_inst) {
            printf("Instruction .DATA 0 : %s %s %s\n", data_inst->mnemonic, data_inst->operand1, data_inst->operand2);
        }

        // Vérifier les premières instructions .CODE
        Instruction* code_inst = result->code_instructions[0];
        if (code_inst) {
            printf("Instruction .CODE 0 : %s %s %s\n", code_inst->mnemonic, code_inst->operand1, code_inst->operand2);
        }
    }

    // Libération des ressources
    free_parser_result(result);

    return 0;
}