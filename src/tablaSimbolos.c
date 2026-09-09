#include <stdio.h>
#include <string.h>
#include "tablaSimbolos.h"

extern int yylineno; 

int CANTSimbolos = 0;
TS tablaSimbolos[100];


// TODO: aca cuando agregemos ambientes tenmos qe checkear
// porqe ahora con esto no hay forma de tener 2 variables con el mismo nombre
// solamente existe una si hay 2 explota.
int agregarSimbolo(enum TipoDato tipo, char *nombre, int linea) {
    if (CANTSimbolos >= 100) {
        printf("Error linea %d: tabla de simbolos llena (maximo 100)\n", linea);
        return -1;
    }
    if (buscarVariable(nombre) != -1) {
        printf("Error linea %d: '%s' ya fue declarada\n", linea, nombre);
        return -1;
    }
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipoDato = tipo;
    tablaSimbolos[CANTSimbolos].valor = 0;
    return CANTSimbolos++;
}

int buscarVariable(char *nombre) {
    for (int i = CANTSimbolos - 1; i >= 0; i--) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}
void imprimirTabla(void) {
    printf("\n--- TABLA DE SIMBOLOS ---\n");
    for (int i = 0; i < CANTSimbolos; i++) {
        printf("[%d] %-8s : %-4s = %d\n", i,
               tablaSimbolos[i].nombre,
               nombreTipo(tablaSimbolos[i].tipoDato),
               tablaSimbolos[i].valor);
    }
}
