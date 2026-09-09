#include <stdio.h>
#include "ast.h"
#include "tablaSimbolos.h"
#include "semantico.h"
#include "interprete.h"
#include "codigoIntermedio.h"

extern int yyparse(void);
extern FILE *yyin;
extern Nodo *raiz;

static int abrirEntrada(int argc, char **argv) {
    if (argc < 2) {
        yyin = stdin;
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Error: no se pudo abrir '%s'\n", argv[1]);
        return 0;
    }
    return 1;
}

static int abortar(void) {
    fprintf(stderr, "Compilacion abortada por errores.\n");
    return 1;
}

int main(int argc, char **argv) {
    if (!abrirEntrada(argc, argv)) return 1;

    if (yyparse() != 0) return abortar();

    printf("\n--- ARBOL ---\n");
    imprimirArbol(raiz, 0);

    printf("\n--- ANALISIS SEMANTICO ---\n");
    resolverNombres(raiz);
    if (hayErrores) return abortar();

    chequearTipos(raiz);
    if (hayErrores) return abortar();
    printf("Sin errores semanticos.\n");

    printf("\n--- ARBOL ANOTADO ---\n");
    imprimirArbol(raiz, 0);

    evaluar(raiz);
    imprimirTabla();

    generarCodigo(raiz);
    imprimirCodigo();

    return 0;
}
