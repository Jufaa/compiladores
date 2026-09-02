#include <stdio.h>
#include <string.h>
#include "ts.h"

extern int yylineno; 

int CANTSimbolos = 0;
TS tablaSimbolos[100];

void agregarSimbolo(enum TipoDato tipo, char *nombre) {
    if (CANTSimbolos >= 100) {
        printf("Error linea %d: tabla de simbolos llena (maximo 100)\n", yylineno);
        return;
    }
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            printf("Error linea %d: '%s' ya fue declarada\n", yylineno, nombre);
            return;
        }
    }
    tablaSimbolos[CANTSimbolos].nombre = nombre;
    tablaSimbolos[CANTSimbolos].tipoDato = tipo;
    tablaSimbolos[CANTSimbolos].valor = 0;
    CANTSimbolos++;
}

enum TipoDato buscarVariable(char *nombre) {
    for (int i = 0; i < CANTSimbolos; i++) {
        if (strcmp(tablaSimbolos[i].nombre, nombre) == 0) {
            return tablaSimbolos[i].tipoDato;
        }
    }
    return T_ERROR;
}