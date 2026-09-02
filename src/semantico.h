#ifndef SEM_H
#define SEM_H
#include "ast.h"
#include "ts.h"
void resolverNombres(Nodo *nodo);
enum TipoDato chequearTipos(Nodo *nodo);
extern int hayErrores;
#endif