#include "cpu.h"
#include "parser.h"

int main() {
    // Initialiser le CPU avec une taille de mémoire de 1024
    CPU *cpu = cpu_init(1024);
    if (!cpu) {
        fprintf(stderr, "Erreur lors de l'initialisation du CPU\n");
        return 1;
    }

    // Parser un fichier assembleur (par exemple "test.txt")
    ParserResult *result = parse("test.txt");
    if (!result) {
        fprintf(stderr, "Erreur lors du parsing du fichier\n");
        cpu_destroy(cpu);
        return 1;
    }

    // Résoudre les constantes (remplacer variables et étiquettes par leurs adresses)
    if (!resolve_constants(result)) {
        fprintf(stderr, "Erreur lors de la résolution des constantes\n");
        free_parser_result(result);
        cpu_destroy(cpu);
        return 1;
    }

    // Allouer le segment de données
    allocate_variables(cpu, result->data_instructions, result->data_count);

    // Allouer le segment de code
    allocate_code_segment(cpu, result->code_instructions, result->code_count);

    // Exécuter le programme
    run_program(cpu);

    // Libérer les ressources
    free_parser_result(result);
    cpu_destroy(cpu);

    return 0;
}