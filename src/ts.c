#include <stdio.h>
#include <string.h>
#include "ts.h"

extern int yylineno; 

int CANTSimbolos = 0;
TS tablaSimbolos[100];

int agregarSimbolo(enum TipoDato tipo, char *nombre, int linea) {
    int indice = 0;
    if (CANTSimbolos >= 100) {
        printf("Error linea %d: tabla de simbolos llena (maximo 100)\n", linea);
        return -1;
    }
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            printf("Error linea %d: '%s' ya fue declarada\n", linea, nombre);
            return -1;
        }
        indice++;
    }
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipoDato = tipo;
    tablaSimbolos[CANTSimbolos].valor = 0;
    CANTSimbolos++;
    return indice;
}

int buscarVariable(char *nombre) {
    for (int i = CANTSimbolos - 1; i >= 0; i--) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}
char *nombreTipo(enum TipoDato t) {
    switch (t) {
        case T_INT:  return "int";
        case T_BOOL: return "bool";
        default:     return "error";
    }
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
