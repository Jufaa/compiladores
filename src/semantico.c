#include "semantico.h"
#include <stdio.h>
#include <stdlib.h>

int hayErrores = 0;

int contarParametros(Nodo *nodo) {
    if (nodo == NULL) return 0;
    if (nodo->tipoNodo == N_SEQ) {
        return contarParametros(nodo->izq) + contarParametros(nodo->der);
    }
    if (nodo->tipoNodo == N_PARAM || nodo->tipoNodo != N_SEQ) { 
        return 1;
    }
    return 0;
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
        enum TipoSimbolos categoria = (nivelActual() <= 0) ? V_GLOBAL : V_LOCAL;
        Simbolo *newSimbolo = agregarSimbolo(categoria, nodo->tipoDato, nodo->nombre, nodo->linea);
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
                nodo->tipoDato = T_ERROR;
            }else{
                nodo->simbolo = simbolo; 
                nodo->tipoDato = simbolo->tipoDato;
            }
            break;
        }

        case N_ASIGN: {
            resolverNombres(nodo->izq);
            Simbolo *simbolo = buscarSimbolo(nodo->nombre);
            if (simbolo == NULL) {
                printf("Error linea %d: variable '%s' no declarada\n", nodo->linea, nodo->nombre);
                hayErrores = 1;
                nodo->tipoDato = T_ERROR;
            }else{
                nodo->simbolo = simbolo;
                nodo->tipoDato = simbolo->tipoDato;
            }
            break;
        }
        case N_METODO:{
            Simbolo *funcion = agregarSimbolo(V_FUNCION, nodo->tipoDato, nodo->nombre, nodo->linea);
            if (funcion == NULL) {
                hayErrores = 1;
            } else {
                nodo->simbolo = funcion;
            }
            abrirNivel();               // abro lvl de parametros
            resolverNombres(nodo->izq); //      parametros
            resolverNombres(nodo->der); //      cuerpo
            cerrarNivel();              // cierro param
            break;
        }
        case N_IF:
        case N_IFELSE:
        case N_WHILE:
        case N_BLOQUE:
        resolverNombres(nodo->izq);
        resolverNombres(nodo->der);     
        
        case N_SUMA:
        case N_MULT:
        case N_RESTA:
        case N_DIV:
        case N_MOD:
        case N_COMPARACION:
        case N_MENOR:
        case N_MAYOR:
        case N_AND:
        case N_OR:
        case N_NEG:
        case N_NOT:
        case N_LLAMADA:
        case N_RETURN:{
            resolverNombres(nodo->izq);
            resolverNombres(nodo->der);
        }
        case N_PARAM:{
            Simbolo *parametros = agregarSimbolo(V_PARAM, nodo->tipoDato, nodo->nombre, nodo->linea);
            if (parametros == NULL) {
                hayErrores = 1;
            } else {
                nodo->simbolo = parametros;
            }
            break;
        }
        case N_NUM:
        case N_BOOL:
        case N_FLOAT:{
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

        case N_FLOAT:
            return nodo->tipoDato = T_FLOAT;

        case N_RESTA:
        case N_DIV:
        case N_MOD:
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

        case N_COMPARACION:
        case N_MENOR:
        case N_MAYOR: {
            enum TipoDato tIzq = chequearTipos(nodo->izq);
            enum TipoDato tDer = chequearTipos(nodo->der);
            if (tIzq == T_ERROR || tDer == T_ERROR) {
                return nodo->tipoDato = T_ERROR;
            }
            if (tIzq != tDer) {
                printf("Error linea %d: '%s' no puede comparar %s con %s\n",
                       nodo->linea, nombreNodo(nodo->tipoNodo),
                       nombreTipo(tIzq), nombreTipo(tDer));
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = T_BOOL;
        }
        case N_AND:
        case N_OR: {
            enum TipoDato tIzq = chequearTipos(nodo->izq);
            enum TipoDato tDer = chequearTipos(nodo->der);
            if (tIzq != T_BOOL || tDer != T_BOOL) {
                printf("Error linea %d: '%s' solo funciona con boolean\n",
                       nodo->linea, nombreNodo(nodo->tipoNodo));
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = T_BOOL;
        }
        case N_NEG: {
            if (chequearTipos(nodo->izq) != T_INT) {
                printf("Error linea %d: '-' solo funciona con int\n", nodo->linea);
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = T_INT;
        }

        case N_NOT: {
            if (chequearTipos(nodo->izq) != T_BOOL) {
                printf("Error linea %d: '!' solo funciona con boolean\n", nodo->linea);
                hayErrores = 1;
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato = T_BOOL;
        }


       case N_ID: {
            if (nodo->simbolo == NULL) {
                return nodo->tipoDato = T_ERROR;
            }
            return nodo->tipoDato;
        }

        case N_PARAM:
        case N_DECL: {
            return nodo->tipoDato;
        }

        case N_ASIGN: {
            enum TipoDato tipoIzq = chequearTipos(nodo->izq);

            if (nodo->simbolo == NULL) {
                return nodo->tipoDato = T_ERROR;
            }
            if (tipoIzq == T_ERROR) {
                return nodo->tipoDato = T_ERROR;
            }

            enum TipoDato tipoVariable = nodo->tipoDato; 
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

        case N_IF:
        case N_IFELSE:
        case N_WHILE: {
            enum TipoDato tipoCond = chequearTipos(nodo->izq);
            if (tipoCond != T_BOOL) {
                printf("Error linea %d: condicion debe ser booleana\n", nodo->linea);
                hayErrores = 1;
            }
            chequearTipos(nodo->der);
            break;
        }

        case N_METODO:
            chequearTipos(nodo->izq); // parametros
            chequearTipos(nodo->der); // cuerpo
            break;
        case N_RETURN: {
            enum TipoDato tipoRetorno = chequearTipos(nodo->izq);
            //TODO: Falta checkear el tipo de retorno con el tipo de la funcion
            break;
        }

    case N_LLAMADA: {
        Simbolo *simboloFuncion = buscarSimbolo(nodo->nombre);
        if (simboloFuncion == NULL) {
            printf("Error linea %d: funcion '%s' no declarada\n", nodo->linea, nodo->nombre);
            hayErrores = 1;
            return nodo->tipoDato = T_ERROR;
        }
        chequearTipos(nodo->izq);
        return nodo->tipoDato = simboloFuncion->tipoDato;
    }
        case N_BLOQUE:
            chequearTipos(nodo->izq);
            chequearTipos(nodo->der);
            break;

        default:
            printf("Error linea %d: nodo desconocido en chequearTipos\n", nodo->linea);
            hayErrores = 1;
            break;
    }
    return T_ERROR;
}

