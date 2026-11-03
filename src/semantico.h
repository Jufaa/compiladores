#ifndef SEMANTICO_H
#define SEMANTICO_H
#include "ast.h"
#include "tablaSimbolos.h"
void resolverNombres(Nodo *nodo);
enum TipoDato chequearTipos(Nodo *nodo);
int nivelActual(void);
extern int hayErrores;
#endif
