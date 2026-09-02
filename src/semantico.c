#include "semantico.h"
#include <stdio.h>
#include <stdlib.h>

int hayErrores = 0;

void resolverNombres(Nodo *nodo) {
    if (nodo == NULL) return;

    switch (nodo->tipoNodo) {

        case N_SEQ:
        case N_BLOQUE: {
            resolverNombres(nodo->izq);
            resolverNombres(nodo->der);
            break;
        }

        case N_DECL: {
            int indice = agregarSimbolo(nodo->tipoDato, nodo->nombre, nodo->linea);
            if (indice == -1) {
                hayErrores = 1;
            }
            nodo->indiceEnLaTablaSimbolos = indice;
            break;
        }

        case N_ID: {
            int indice = buscarVariable(nodo->nombre);
            if (indice == -1) {
                printf("Error linea %d: variable '%s' no declarada\n", nodo->linea, nodo->nombre);
                hayErrores = 1;
            }
            nodo->indiceEnLaTablaSimbolos = indice;
            break;
        }

        case N_ASIGN: {
            resolverNombres(nodo->izq);
            int indice = buscarVariable(nodo->nombre);
            if (indice == -1) {
                printf("Error linea %d: variable '%s' no declarada\n", nodo->linea, nodo->nombre);
                hayErrores = 1;
            }
            nodo->indiceEnLaTablaSimbolos = indice;
            break;
        }

        case N_SUMA:
        case N_MULT: {
            resolverNombres(nodo->izq);
            resolverNombres(nodo->der);
            break;
        }

        case N_NUM:
        case N_BOOL: {
            break;
        }

        default:
            printf("Error linea %d: nodo desconocido en resolverNombres\n", nodo->linea);
            hayErrores = 1;
            break;
    }
}

enum TipoDato chequearTipos(Nodo *nodo) {
    if (nodo == NULL) return T_ERROR;

    switch (nodo->tipoNodo) {

        case N_NUM:
            return nodo->tipoDato = T_INT;

        case N_BOOL:
            return nodo->tipoDato = T_BOOL;

        case N_ID: {
            if (nodo->indiceEnLaTablaSimbolos == -1) {
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = tablaSimbolos[nodo->indiceEnLaTablaSimbolos].tipoDato;
        }

        case N_SUMA:
        case N_MULT: {
            enum TipoDato tIzq = chequearTipos(nodo->izq);
            enum TipoDato tDer = chequearTipos(nodo->der);
            if (tIzq == T_ERROR || tDer == T_ERROR) {
                return nodo->tipoDato = T_ERROR;
            }
            if (tIzq == T_INT && tDer == T_INT) {
                return nodo->tipoDato = T_INT;
            }
            printf("Error linea %d: '%s' solo funciona con int\n",
                   nodo->linea, nombreNodo(nodo->tipoNodo));
            hayErrores = 1;
            return nodo->tipoDato = T_ERROR;
        }

        case N_ASIGN: {
            enum TipoDato tipoIzq = chequearTipos(nodo->izq);

            if (nodo->indiceEnLaTablaSimbolos == -1) {
                return nodo->tipoDato = T_ERROR;
            }
            if (tipoIzq == T_ERROR) {
                return nodo->tipoDato = T_ERROR;
            }

            enum TipoDato tipoVariable = tablaSimbolos[nodo->indiceEnLaTablaSimbolos].tipoDato;
            if (tipoVariable != tipoIzq) {
                printf("Error linea %d: tipo incompatible en la asignacion a '%s'\n",
                       nodo->linea, nodo->nombre);
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = tipoIzq;
        }

        case N_SEQ:
        case N_BLOQUE:
            chequearTipos(nodo->izq);
            chequearTipos(nodo->der);
            break;

        case N_DECL:
            break;

        default:
            printf("Error linea %d: nodo desconocido en chequearTipos\n", nodo->linea);
            hayErrores = 1;
            break;
    }
    return T_ERROR;
}
