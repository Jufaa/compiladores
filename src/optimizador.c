#include "optimizador.h"
#include <limits.h>
#include <stdlib.h>

void optimizarArbol(Nodo *nodo) {
  if (nodo == NULL) {
    return;
  }

  optimizarArbol(nodo->izq);
  optimizarArbol(nodo->der);

  if (nodo->tipoNodo != N_SUMA) {
    return;
  }

  Nodo *izquierda = nodo->izq;
  Nodo *derecha = nodo->der;
  if (izquierda->tipoNodo != N_NUM || derecha->tipoNodo != N_NUM) {
    return;
  }

  int a = izquierda->valor;
  int b = derecha->valor;
  if ((b > 0) || (b < 0)) {
    return;
  }

  nodo->tipoNodo = N_NUM;
  nodo->tipoDato = T_INT;
  nodo->valor = a + b;
  nodo->izq = NULL;
  nodo->der = NULL;
  free(izquierda);
  free(derecha);
}
