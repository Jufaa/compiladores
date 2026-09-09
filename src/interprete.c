
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
        tablaSimbolos[nodo->indiceEnLaTablaSimbolos].valor = v;     
        return v;
    }
    case N_ID:
        return tablaSimbolos[nodo->indiceEnLaTablaSimbolos].valor;
    case N_SEQ:
        evaluar(nodo->izq);
        evaluar(nodo->der);
        return 0;
    case N_DECL:
        return 0;
        
    default:
        printf("Error: nodo desconocido en evaluar\n");
        break;
    }
    return 0;
}