#include <stdio.h>
#include <string.h>
#include "interprete.h"
#include "ast.h"
#include "tablaSimbolos.h"

#define MAX_METODOS 100
#define MAX_VARS_LOCALES 100

static Nodo *tablaMetodos[MAX_METODOS];
static int cantMetodos = 0;
static int enReturn = 0;
static int valorRetorno = 0;
static int evalNodo(Nodo *nodo);

static void registrarMetodos(Nodo *nodo) {
    if (nodo == NULL) return;
    if (nodo->tipoNodo == N_SEQ) {
        registrarMetodos(nodo->izq);
        registrarMetodos(nodo->der);
    } else if (nodo->tipoNodo == N_METODO) {
        if (cantMetodos < MAX_METODOS) {
            tablaMetodos[cantMetodos++] = nodo;
        }
    }
}

static Nodo *buscarMetodoAST(const char *nombre) {
    for (int i = 0; i < cantMetodos; i++) {
        if (strcmp(tablaMetodos[i]->nombre, nombre) == 0) {
            return tablaMetodos[i];
        }
    }
    return NULL;
}

static void aplanarLista(Nodo *nodo, Nodo **lista, int *cant) {
    if (nodo == NULL) return;
    if (nodo->tipoNodo == N_SEQ) {
        aplanarLista(nodo->izq, lista, cant);
        aplanarLista(nodo->der, lista, cant);
    } else {
        if (*cant < MAX_VARS_LOCALES) {
            lista[(*cant)++] = nodo;
        }
    }
}

static void recolectarLocales(Nodo *nodo, Simbolo **lista, int *cant) {
    if (nodo == NULL) return;
    if (nodo->tipoNodo == N_PARAM || nodo->tipoNodo == N_DECL) {
        if (nodo->simbolo != NULL && *cant < MAX_VARS_LOCALES) {
            lista[(*cant)++] = nodo->simbolo;
        }
    }
    recolectarLocales(nodo->izq, lista, cant);
    recolectarLocales(nodo->der, lista, cant);
}

static int ejecutarLlamada(Nodo *metodo, int *valoresArgs, int cantArgs) {
    Simbolo *locales[MAX_VARS_LOCALES];
    int backupValores[MAX_VARS_LOCALES];
    int cantLocales = 0;

    recolectarLocales(metodo->izq, locales, &cantLocales);
    recolectarLocales(metodo->der, locales, &cantLocales);

    for (int i = 0; i < cantLocales; i++) {
        backupValores[i] = locales[i]->valor;
        locales[i]->valor = 0; // Inicializamos locales en 0 para la nueva activación
    }
    Nodo *params[MAX_VARS_LOCALES];
    int cantParams = 0;
    aplanarLista(metodo->izq, params, &cantParams);
    for (int i = 0; i < cantParams && i < cantArgs; i++) {
        if (params[i]->simbolo != NULL) {
            params[i]->simbolo->valor = valoresArgs[i];
        }
    }
    int prevEnReturn = enReturn;
    int prevValorRetorno = valorRetorno;
    enReturn = 0;
    valorRetorno = 0;
    evalNodo(metodo->der);
    int resultado = valorRetorno;
    enReturn = prevEnReturn;
    valorRetorno = prevValorRetorno;
    if (strcmp(metodo->nombre, "main") != 0) {
        for (int i = 0; i < cantLocales; i++) {
            locales[i]->valor = backupValores[i];
        }
    }
    return resultado;
}

static int evalNodo(Nodo *nodo) {
    if (nodo == NULL || enReturn) return 0;

    switch (nodo->tipoNodo) {
    case N_NUM:
    case N_BOOL:
        return nodo->valor;
    case N_FLOAT:
        return (int)nodo->valorFloat;
    case N_ID:
        return nodo->simbolo != NULL ? nodo->simbolo->valor : 0;
    case N_DECL:
    case N_PARAM:
    case N_METODO:
        return 0;
    case N_ASIGN: {
        int v = evalNodo(nodo->izq);
        if (nodo->simbolo != NULL) {
            nodo->simbolo->valor = v;
        }
        return v;
    }
    case N_SEQ:
    case N_BLOQUE:
        evalNodo(nodo->izq);
        if (!enReturn) {
            evalNodo(nodo->der);
        }
        return 0;
    case N_IF:
        if (evalNodo(nodo->izq)) {
            evalNodo(nodo->der);
        }
        return 0;
    case N_IFELSE:
        if (evalNodo(nodo->izq)) {
            evalNodo(nodo->der->izq); // rama then
        } else {
            evalNodo(nodo->der->der); // rama else
        }
        return 0;
    case N_WHILE:
        while (!enReturn && evalNodo(nodo->izq)) {
            evalNodo(nodo->der);
        }
        return 0;
    case N_RETURN:
        valorRetorno = (nodo->izq != NULL) ? evalNodo(nodo->izq) : 0;
        enReturn = 1;
        return valorRetorno;

    case N_LLAMADA: {
        Nodo *metodo = buscarMetodoAST(nodo->nombre);
        if (metodo == NULL) {
            return 0;
        }
        Nodo *nodosArgs[MAX_VARS_LOCALES];
        int valoresArgs[MAX_VARS_LOCALES];
        int cantArgs = 0;
        aplanarLista(nodo->izq, nodosArgs, &cantArgs);

        for (int i = 0; i < cantArgs; i++) {
            valoresArgs[i] = evalNodo(nodosArgs[i]);
        }
        return ejecutarLlamada(metodo, valoresArgs, cantArgs);
    }
    case N_SUMA:        return evalNodo(nodo->izq) + evalNodo(nodo->der);
    case N_RESTA:       return evalNodo(nodo->izq) - evalNodo(nodo->der);
    case N_MULT:        return evalNodo(nodo->izq) * evalNodo(nodo->der);
    case N_DIV: {
        int d = evalNodo(nodo->der);
        if (d == 0) {
            fprintf(stderr, "Error en tiempo de ejecucion (linea %d): division por cero\n", nodo->linea);
            return 0;
        }
        return evalNodo(nodo->izq) / d;
    }
    case N_MOD: {
        int d = evalNodo(nodo->der);
        if (d == 0) {
            fprintf(stderr, "Error en tiempo de ejecucion (linea %d): modulo por cero\n", nodo->linea);
            return 0;
        }
        return evalNodo(nodo->izq) % d;
    }
    case N_COMPARACION: return evalNodo(nodo->izq) == evalNodo(nodo->der);
    case N_NOIGUAL:     return evalNodo(nodo->izq) != evalNodo(nodo->der);
    case N_MENOR:       return evalNodo(nodo->izq) <  evalNodo(nodo->der);
    case N_MAYOR:       return evalNodo(nodo->izq) >  evalNodo(nodo->der);
    case N_MENORIGUAL:  return evalNodo(nodo->izq) <= evalNodo(nodo->der);
    case N_MAYORIGUAL:  return evalNodo(nodo->izq) >= evalNodo(nodo->der);
    case N_AND:         return evalNodo(nodo->izq) && evalNodo(nodo->der); 
    case N_OR:          return evalNodo(nodo->izq) || evalNodo(nodo->der); 
    case N_NEG:         return -evalNodo(nodo->izq);
    case N_NOT:         return !evalNodo(nodo->izq);

    default:
        printf("Error: nodo desconocido en evaluar\n");
        return 0;
    }
}

int evaluar(Nodo *raiz) {
    if (raiz == NULL) return 0;
    cantMetodos = 0;
    enReturn = 0;
    valorRetorno = 0;
    registrarMetodos(raiz);

    Nodo *mainMetodo = buscarMetodoAST("main");
    if (mainMetodo != NULL) {
        return ejecutarLlamada(mainMetodo, NULL, 0);
    }
    return evalNodo(raiz);
}