#include "cpu.h"
int main (){
    // Initialisation du CPU
    CPU* cpu = cpu_init(1024);

    // Exemple de données
    int* ax = (int *)hashmap_get(cpu->context, "AX");
    int* bx = (int *)hashmap_get(cpu->context, "BX");

    *ax = 10; // On met la valeur 10 dans AX

    // Copie de la valeur de AX dans BX
    handle_MOV(cpu, ax, bx);

    // BX devrait maintenant contenir 10
    printf("Valeur de BX après MOV : %d\n", *bx);

    // Destruction du CPU
    cpu_destroy(cpu);
    return 1;
}