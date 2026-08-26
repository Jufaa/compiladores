typedef struct Nodo{
    char *tipoToken;
    char *tipo;
    int valor;
    struct Nodo *izq;
    struct Nodo *der;
} Nodo;

Nodo *crearNodo(char *tipo, int valor, Nodo *izq, Nodo *der);
void imprimirAST(Nodo *n, int nivel);

%union {
    int numero;
    Nodo *nodo;
}



P: E TPUNTOCOMA {imprimirAST($1, 0); }
 ;
E: E TSUMA E        { $$ = crearNodo("SUMA", 0, $1, $3);imprimirAST($$, 0); }
 | E TMULTIPLICACION E  { $$ = crearNodo("MULT", 0, $1, $3);  imprimirAST($$, 0); }
 | TPA E TPC        { $$ = crearNodo("PARENTESIS", 0, $2, NULL);imprimirAST($$, 0); }
 | TNUM             { $$ = crearNodo("NUM", $1, NULL, NULL);  imprimirAST($$, 0); }
 ;


Nodo *crearNodo(char *tipo, int valor, Nodo *izq, Nodo *der) {
    Nodo *n = malloc(sizeof(Nodo));

    n->tipo = tipo;
    n->valor = valor;
    n->izq = izq;
    n->der = der;

    return n;
}


void imprimirAST(Nodo *n, int nivel) {
    if (n == NULL)
        return;

    for (int i = 0; i < nivel; i++)
        printf("  ");

    if (strcmp(n->tipo, "NUM") == 0)
        printf("%d\n", n->valor);
    else
        printf("%s\n", n->tipo);

    imprimirAST(n->izq, nivel + 1);
    imprimirAST(n->der, nivel + 1);
}
