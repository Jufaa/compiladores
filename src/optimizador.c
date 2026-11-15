#include "optimizador.h"
#include <limits.h>
#include <stdlib.h>

static void liberarSubarbol(Nodo *nodo) {
  if (nodo == NULL) return;
  liberarSubarbol(nodo->izq);
  liberarSubarbol(nodo->der);
  free(nodo);
}

static void reemplazarPorHijo(Nodo *padre, Nodo *hijoConservado, Nodo *hijoDescartado) {
  liberarSubarbol(hijoDescartado);
  Nodo copia = *hijoConservado;
  free(hijoConservado);
  *padre = copia;
}

// Convierte el nodo actual en una constante entera
static void transformarEnNum(Nodo *nodo, int valor) {
  liberarSubarbol(nodo->izq);
  liberarSubarbol(nodo->der);
  nodo->tipoNodo = N_NUM;
  nodo->tipoDato = T_INT;
  nodo->valor = valor;
  nodo->izq = NULL;
  nodo->der = NULL;
}

static void transformarEnBool(Nodo *nodo, int valor) {
  liberarSubarbol(nodo->izq);
  liberarSubarbol(nodo->der);
  nodo->tipoNodo = N_BOOL;
  nodo->tipoDato = T_BOOL;
  nodo->valor = valor ? 1 : 0;
  nodo->izq = NULL;
  nodo->der = NULL;
}

static void transformarEnFloat(Nodo *nodo, float valor) {
  liberarSubarbol(nodo->izq);
  liberarSubarbol(nodo->der);
  nodo->tipoNodo = N_FLOAT;
  nodo->tipoDato = T_FLOAT;
  nodo->valorFloat = valor;
  nodo->izq = NULL;
  nodo->der = NULL;
}

void optimizarArbol(Nodo *nodo) {
  if (nodo == NULL) return;

  // optimización recursiva de los hijos (post-orden)
  optimizarArbol(nodo->izq);
  optimizarArbol(nodo->der);

  Nodo *izq = nodo->izq;
  Nodo *der = nodo->der;

  switch (nodo->tipoNodo) {

  // --- OPERADORES UNARIOS ---
  case N_NEG:
    if (izq->tipoNodo == N_NUM && izq->valor != INT_MIN) {
      transformarEnNum(nodo, -izq->valor);
    } else if (izq->tipoNodo == N_FLOAT) {
      transformarEnFloat(nodo, -izq->valorFloat);
    } else if (izq->tipoNodo == N_NEG) {
      // Doble negación
      reemplazarPorHijo(nodo, izq->izq, NULL);
      free(izq);
    }
    break;

  case N_NOT:
    if (izq->tipoNodo == N_BOOL) {
      transformarEnBool(nodo, !izq->valor);
    } else if (izq->tipoNodo == N_NOT) {
      // Doble negación lógica: !(!x) -> x
      reemplazarPorHijo(nodo, izq->izq, NULL);
      free(izq);
    }
    break;

  // --- OPERADORES ARITMÉTICOS ---
  case N_SUMA:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      int a = izq->valor, b = der->valor;
      if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) return;
      transformarEnNum(nodo, a + b);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      transformarEnFloat(nodo, izq->valorFloat + der->valorFloat);
    } else if (der->tipoNodo == N_NUM && der->valor == 0) {
      reemplazarPorHijo(nodo, izq, der); // x + 0 -> x
    } else if (izq->tipoNodo == N_NUM && izq->valor == 0) {
      reemplazarPorHijo(nodo, der, izq); // 0 + x -> x
    }
    break;

  case N_RESTA:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      int a = izq->valor, b = der->valor;
      if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b)) return;
      transformarEnNum(nodo, a - b);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      transformarEnFloat(nodo, izq->valorFloat - der->valorFloat);
    } else if (der->tipoNodo == N_NUM && der->valor == 0) {
      reemplazarPorHijo(nodo, izq, der); // x - 0 -> x
    }
    break;

  case N_MULT:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      long long prod = (long long)izq->valor * (long long)der->valor;
      if (prod > INT_MAX || prod < INT_MIN) return;
      transformarEnNum(nodo, (int)prod);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      transformarEnFloat(nodo, izq->valorFloat * der->valorFloat);
    } else if (der->tipoNodo == N_NUM && der->valor == 1) {
      reemplazarPorHijo(nodo, izq, der); // x * 1 -> x
    } else if (izq->tipoNodo == N_NUM && izq->valor == 1) {
      reemplazarPorHijo(nodo, der, izq); // 1 * x -> x
    }
    break;

  case N_DIV:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      int a = izq->valor, b = der->valor;
      if (b == 0 || (a == INT_MIN && b == -1)) return;
      transformarEnNum(nodo, a / b);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      if (der->valorFloat == 0.0f) return;
      transformarEnFloat(nodo, izq->valorFloat / der->valorFloat);
    } else if (der->tipoNodo == N_NUM && der->valor == 1) {
      reemplazarPorHijo(nodo, izq, der); // x / 1 -> x
    }
    break;

  case N_MOD:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      int a = izq->valor, b = der->valor;
      if (b == 0 || (a == INT_MIN && b == -1)) return;
      transformarEnNum(nodo, a % b);
    }
    break;

  // --- OPERADORES DE COMPARACIÓN ---
  case N_COMPARACION:
  case N_NOIGUAL:
    if ((izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) ||
        (izq->tipoNodo == N_BOOL && der->tipoNodo == N_BOOL)) {
      int iguales = (izq->valor == der->valor);
      transformarEnBool(nodo, nodo->tipoNodo == N_COMPARACION ? iguales : !iguales);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      int iguales = (izq->valorFloat == der->valorFloat);
      transformarEnBool(nodo, nodo->tipoNodo == N_COMPARACION ? iguales : !iguales);
    }
    break;

  case N_MENOR:
  case N_MAYOR:
  case N_MENORIGUAL:
  case N_MAYORIGUAL:
    if (izq->tipoNodo == N_NUM && der->tipoNodo == N_NUM) {
      int a = izq->valor, b = der->valor, res = 0;
      if (nodo->tipoNodo == N_MENOR)           res = (a < b);
      else if (nodo->tipoNodo == N_MAYOR)      res = (a > b);
      else if (nodo->tipoNodo == N_MENORIGUAL) res = (a <= b);
      else                                     res = (a >= b);
      transformarEnBool(nodo, res);
    } else if (izq->tipoNodo == N_FLOAT && der->tipoNodo == N_FLOAT) {
      float a = izq->valorFloat, b = der->valorFloat;
      int res = 0;
      if (nodo->tipoNodo == N_MENOR)           res = (a < b);
      else if (nodo->tipoNodo == N_MAYOR)      res = (a > b);
      else if (nodo->tipoNodo == N_MENORIGUAL) res = (a <= b);
      else                                     res = (a >= b);
      transformarEnBool(nodo, res);
    }
    break;

  // --- OPERADORES LÓGICOS ---
  case N_AND:
    if (izq->tipoNodo == N_BOOL && der->tipoNodo == N_BOOL) {
      transformarEnBool(nodo, izq->valor && der->valor);
    } else if (izq->tipoNodo == N_BOOL) {
      if (izq->valor == 0) transformarEnBool(nodo, 0); // false && x -> false (cortocircuito)
      else reemplazarPorHijo(nodo, der, izq);          // true && x  -> x
    } else if (der->tipoNodo == N_BOOL && der->valor == 1) {
      reemplazarPorHijo(nodo, izq, der);               // x && true  -> x
    }
    break;

  case N_OR:
    if (izq->tipoNodo == N_BOOL && der->tipoNodo == N_BOOL) {
      transformarEnBool(nodo, izq->valor || der->valor);
    } else if (izq->tipoNodo == N_BOOL) {
      if (izq->valor == 1) transformarEnBool(nodo, 1); // true || x  -> true (cortocircuito)
      else reemplazarPorHijo(nodo, der, izq);          // false || x -> x
    } else if (der->tipoNodo == N_BOOL && der->valor == 0) {
      reemplazarPorHijo(nodo, izq, der);               // x || false -> x
    }
    break;

  // --- ELIMINACIÓN DE CÓDIGO MUERTO EN CONTROL DE FLUJO ---
  case N_IF:
    if (izq->tipoNodo == N_BOOL) {
      if (izq->valor == 1) {
        reemplazarPorHijo(nodo, der, izq); // if (true) { B } -> { B }
      } else {
        // if (false) { B } -> (no genera instrucciones)
        liberarSubarbol(izq);
        liberarSubarbol(der);
        nodo->tipoNodo = N_SEQ;
        nodo->izq = NULL;
        nodo->der = NULL;
      }
    }
    break;

  case N_IFELSE:
    if (izq->tipoNodo == N_BOOL) {
      Nodo *ramaTrue = der->izq;
      Nodo *ramaFalse = der->der;
      free(der); // liberamos el N_SEQ intermedio que unía ambas ramas
      liberarSubarbol(izq);
      if (izq->valor == 1) {
        reemplazarPorHijo(nodo, ramaTrue, ramaFalse);
      } else {
        reemplazarPorHijo(nodo, ramaFalse, ramaTrue);
      }
    }
    break;

  case N_WHILE:
    if (izq->tipoNodo == N_BOOL && izq->valor == 0) {
      // while (false) { B } -> nunca se ejecuta
      liberarSubarbol(izq);
      liberarSubarbol(der);
      nodo->tipoNodo = N_SEQ;
      nodo->izq = NULL;
      nodo->der = NULL;
    }
    break;

  default:
    break;
  }
}