#include <stdio.h>
#include "src/tablaSimbolos.h"



void testGlobal2() {
    printf("Test: variables globales \n");

    abrirNivel(); 
    printf("Agregados 2 Juan\n");
    printf("Genera error: \n");
    agregarSimbolo(V_GLOBAL, T_INT, "Juan", 1);
    agregarSimbolo(V_GLOBAL, T_INT, "Juan", 5);

    imprimirTabla();
}

void testGlobal() {
    printf("Test: variables globales \n");

    abrirNivel(); // nivel 0 
    agregarSimbolo(V_GLOBAL, T_INT, "x", 1);
    agregarSimbolo(V_GLOBAL, T_INT, "y", 2);
    printf("Agregados x e y como globales en nivel 0\n");

    imprimirTabla();
}

void testFuncionSinParams() {
    printf("\nTest: funcion sin parametros \n");

    // Nivel 1 - nivel de param (vacio en este parte)
    abrirNivel();
    printf("Nivel 1 abierto (parametros - vacio)\n");

    // Nivel 2 - nivel para el cuerpo de la funcion
    abrirNivel();
    printf("Nivel 2 abierto cuerpo de la funcion\n");

    agregarSimbolo(V_LOCAL, T_INT, "resultado", 10);
    printf("Agregado resultado en nivel 2 cuerpo\n");

    imprimirTabla();

    // cerrar body funcion
    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo)\n");

    // cerrar param
    cerrarNivel();
    printf("Nivel 1 cerrado (parametros)\n");

    imprimirTabla();
}

void testFuncionConParams() {
    printf("\nTest: funcion con parametros \n");

    // Nivel 1 - nivel de parametros
    abrirNivel();
    printf("Nivel 1 abierto (parametros)\n");

    agregarSimbolo(V_PARAM, T_INT, "a", 20);
    agregarSimbolo(V_PARAM, T_INT, "b", 21);
    printf("Agregados parametros a y b en nivel 1\n");

    imprimirTabla();

    // Nivel 2 - nivel del cuerpo de la funcion
    abrirNivel();
    printf("Nivel 2 abierto cuerpo de la funcion\n");

    agregarSimbolo(V_LOCAL, T_INT, "resultado", 22);
    printf("Agregado resultado en nivel 2 cuerpo\n");

    // shadowing: a existe en nivel 1 (param), pero se permite en nivel 2 (cuerpo)
    agregarSimbolo(V_LOCAL, T_INT, "a", 23);
    printf("Agregado a en nivel 2 (shadowing del param)\n");

    imprimirTabla();

    // verificar que a existe en ambos niveles (shadowing)
    int a_cuerpo = buscarSimboloEnUnNivel("a", 2);
    int a_param = buscarSimboloEnUnNivel("a", 1);
    printf("\nVerificacion de shadowing:\n");
    printf("a en nivel 2 (cuerpo): %s\n", a_cuerpo == 1 ? "encontrado" : "no encontrado");
    printf("a en nivel 1 (param):  %s\n", a_param == 1 ? "encontrado" : "no encontrado");

    // duplicado en nivel 2: resultado ya existe, deberia fallar
    printf("\nIntentando declarar resultado de nuevo en nivel 2 (deberia fallar):\n");
    Simbolo *res = agregarSimbolo(V_LOCAL, T_INT, "resultado", 24);
    if (res == NULL) {
        printf("OK: duplicado detectado en nivel 2\n");
    }

    // cerrar cuerpo
    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo)\n");

    // cerrar parametros
    cerrarNivel();
    printf("Nivel 1 cerrado (parametros)\n");
}

void testMain() {
    printf("\n Test: funcion main \n");

    // Nivel 1 - parametros de main (vacio)
    abrirNivel();
    printf("Nivel 1 abierto (parametros de main - vacio)\n");

    // Nivel 2 - cuerpo de main
    abrirNivel();
    printf("Nivel 2 abierto (cuerpo de main)\n");

    agregarSimbolo(V_LOCAL, T_INT, "z", 30);
    printf("Agregado 'z' en nivel 2 (cuerpo de main)\n");

    // buscar variable global desde main
    Simbolo *global = buscarSimbolo("x");
    printf("\nBuscando global 'x' desde main: %s\n", global != NULL ? "encontrado" : "NULL");

    // buscar parametro de otra funcion (no deberia encontrar)
    Simbolo *param = buscarSimbolo("a");
    printf("Buscando param 'a' de otra funcion: %s\n", param != NULL ? "encontrado (incorrecto)" : "NULL (correcto)");

    imprimirTabla();

    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo de main)\n");

    cerrarNivel();
    printf("Nivel 1 cerrado (parametros de main)\n");
}

int main() {
    testGlobal2();
    testGlobal();
    testFuncionSinParams();
    testFuncionConParams();
    testMain();

    printf("\n Estado final de la tabla \n");
    imprimirTabla();

    printf("\n Todos los tests completados \n");
    return 0;
}
