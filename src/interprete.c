
#include <stdio.h>
#include "interprete.h"
#include "ast.h"
#include "tablaSimbolos.h"
int evaluar(Nodo *nodo){
    if (nodo == NULL) return 0;
    switch (nodo->tipoNodo)
    {
    case N_NUM:
        return nodo->valor;
    case N_BOOL:
        return nodo->valor;
    
    case N_MULT:
        return evaluar(nodo->izq) * evaluar(nodo->der);
    case N_SUMA:
        return evaluar(nodo->izq) + evaluar(nodo->der);
    case N_ASIGN:{
        int v = evaluar(nodo->izq);
        nodo->simbolo->valor = v;     
        return v;
    }
    case N_ID:
        return nodo->simbolo->valor;
    case N_SEQ:
        evaluar(nodo->izq);
        evaluar(nodo->der);
        return 0;
    case N_DECL:
        return 0;
    case N_RESTA:
        return evaluar(nodo->izq) - evaluar(nodo->der);
    case N_DIV:
        return evaluar(nodo->izq) / evaluar(nodo->der);
    case N_MOD:
        return evaluar(nodo->izq) % evaluar(nodo->der);
    case N_COMPARACION:
        return evaluar(nodo->izq) == evaluar(nodo->der);
    case N_MENOR:
        return evaluar(nodo->izq) < evaluar(nodo->der);
    case N_MAYOR:
        return evaluar(nodo->izq) > evaluar(nodo->der);
    case N_NOIGUAL:
        return evaluar(nodo->izq) != evaluar(nodo->der);
    case N_MENORIGUAL:
        return evaluar(nodo->izq) <= evaluar(nodo->der);
    case N_MAYORIGUAL:
        return evaluar(nodo->izq) >= evaluar(nodo->der);
    case N_AND:
        return evaluar(nodo->izq) && evaluar(nodo->der);
    case N_OR:
        return evaluar(nodo->izq) || evaluar(nodo->der);
    case N_NEG:
        return -evaluar(nodo->izq);
    case N_NOT:
        return !evaluar(nodo->izq);
        
    default:
        printf("Error: nodo desconocido en evaluar\n");
        break;
    }
    return 0;
}