#include "semantico.h"
#include <stdio.h>
#include <stdlib.h>

int hayErrores = 0;

int contarParametros(Nodo *nodo) {
    if (nodo == NULL) return 0;

    switch (nodo->tipoNodo) {
        case N_PARAM:
            return 1 + contarParametros(nodo->sig);
        default:
            return 0;
    }
}


void resolverNombres(Nodo *nodo) {
    if (nodo == NULL) return;

    switch (nodo->tipoNodo) {

        case N_SEQ:{
            resolverNombres(nodo->izq);
            resolverNombres(nodo->der);
            break;
        }

        case N_DECL: {
        Simbolo *newSimbolo = agregarSimbolo(V_LOCAL, nodo->tipoDato, nodo->nombre, nodo->linea);
            if (newSimbolo == NULL) {
                hayErrores = 1;
            } else {
                nodo->simbolo = newSimbolo;
            }
            break;
        }   

        case N_ID: {
            Simbolo *simbolo = buscarSimbolo(nodo->nombre);
            if (simbolo == NULL) {
                printf("Error linea %d: variable '%s' no declarada\n", nodo->linea, nodo->nombre);
                hayErrores = 1;
            }
            nodo->simbolo = simbolo;
            break;
        }

        case N_ASIGN: {
            resolverNombres(nodo->izq);
            Simbolo *simbolo = buscarSimbolo(nodo->nombre);
            if (simbolo == NULL) {
                printf("Error linea %d: variable '%s' no declarada\n", nodo->linea, nodo->nombre);
                hayErrores = 1;
            }
            nodo->simbolo = simbolo;
            break;
        }
        case N_IF:
        case N_IFELSE:
        case N_WHILE:
        case N_RETURN:
        case N_BLOQUE:
        case N_METODO:
        case N_PARAM:
        case N_LLAMADA:
            resolverNombres(nodo->izq);
            resolverNombres(nodo->der);     

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
            if (nodo->simbolo == NULL) {
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = nodo->simbolo->tipoDato;
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

            if (nodo->simbolo == NULL) {
                return nodo->tipoDato = T_ERROR;
            }
            if (tipoIzq == T_ERROR) {
                return nodo->tipoDato = T_ERROR;
            }

            enum TipoDato tipoVariable = nodo->simbolo->tipoDato;
            if (tipoVariable != tipoIzq) {
                printf("Error linea %d: tipo incompatible en la asignacion a '%s'\n",
                       nodo->linea, nodo->nombre);
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = tipoIzq;
        }

        case N_SEQ:
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
