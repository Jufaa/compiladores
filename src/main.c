#include "ast.h"
#include "assembler.h"
#include "codigoIntermedio.h"
#include "interprete.h"
#include "semantico.h"
#include "tablaSimbolos.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int yyparse(void);
extern FILE *yyin;
extern Nodo *raiz;

static int abrirEntrada(const char *archivo) {
  if (archivo == NULL) {
    yyin = stdin;
    return 1;
  }
  yyin = fopen(archivo, "r");
  if (yyin == NULL) {
    fprintf(stderr, "Error: no se pudo abrir '%s'\n", archivo);
    return 0;
  }
  return 1;
}

static int abortar(void) {
  fprintf(stderr, "Compilacion abortada por errores.\n");
  liberarTabla();
  return 1;
}

int main(int argc, char **argv) {
  const char *entrada = NULL, *assembly = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      printf("Uso: %s [archivo] [--asm salida.s]\n"
             "Sin archivo, lee desde stdin. --asm genera Linux x86-64 System V (int y boolean).\n", argv[0]);
      return 0;
    }
    if (strcmp(argv[i], "--asm") == 0) {
      if (assembly != NULL || i + 1 == argc || argv[i + 1][0] == '-') {
        fprintf(stderr, "Error: --asm requiere un unico archivo de salida\n");
        return 1;
      }
      assembly = argv[++i];
    } else if (argv[i][0] == '-' || entrada != NULL) {
      fprintf(stderr, "Error: argumento no reconocido '%s'; use --help\n", argv[i]);
      return 1;
    } else entrada = argv[i];
  }
  if (!abrirEntrada(entrada))
    return 1;

  if (assembly != NULL) {
    struct stat origen, destino;
    if (fstat(fileno(yyin), &origen) == 0 && stat(assembly, &destino) == 0 &&
        origen.st_dev == destino.st_dev && origen.st_ino == destino.st_ino) {
      fprintf(stderr, "Error: la salida assembly no puede reemplazar la entrada\n");
      return 1;
    }
  }

  if (yyparse() != 0)
    return abortar();

  printf("\n--- ARBOL ---\n");
  imprimirArbol(raiz, 0);

  printf("\n--- ANALISIS SEMANTICO ---\n");
  inicializarTabla();
  abrirNivel();
  resolverNombres(raiz);
  if (hayErrores)
    return abortar();

  chequearTipos(raiz);
  if (hayErrores)
    return abortar();
  printf("Sin errores semanticos.\n");

  printf("\n--- ARBOL ANOTADO ---\n");
  imprimirArbol(raiz, 0);

  if (assembly == NULL)
    evaluar(raiz);
  imprimirTabla();

  if (!generarCodigo(raiz)) {
    return abortar();
  }

  imprimirCodigo();

  if (assembly != NULL && !generarAssembly(raiz, assembly))
    return abortar();

  liberarTabla();

  return 0;
}
