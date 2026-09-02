Como funciona
ante tenia se parseaba mientras se calculaban onda E: E TSUMA E
sumaba ahi y devolvia el nuemro ahi. el problema que se pierde

el ast lo qe hace es que el parser deja de decidir y ahora arma.

el nodo
enum TipoNodo tipoNodo; // q clase de nodo es

enum TipoDato tipoDato; // solo se asgina cunado se crea un, N_DECL,y se crea antes  para llevar el dato desde el parser hasta la tabla de símbolos.
osea escribo "int" en el archivo - 
1.el parser lo mete en DECL 
2.resolverNombres lo saca de la caja 
3.lo mete en tablaSimbolos[i].tipoDato 
4.de ahí en adelante, todos consultan LA TABLA

int indiceEnLaTablaSimbolos; // en que indice de la tabla esta

char* nombre; // el identificador, sin resolver

int valor; // el numero de un lteral

struct Nodo *izq, *der; // los hijos

int linea; // para los mensajes de error


data importante
tipoNodo - qe clase de nodo es
tipoDato - que tipo de dato es


como expliqe antes tipodato existe nomas por el n_decl, porqe un n_num siempre es int, el n_id lo saco del tabla con el indice, n_suma pregunta a los hijos, encamio el int de int x solo existe en el texto, si no lo guardo lo pierdo. por eso se usa
y en chequeatTipos usamos el campo ese para anotar el tipo qe va calculando en cada expresion
 
 Como hace eso?
 tiene qe responder al q tipo es esta expresion, para el + de 2+3 le pregunta a los hijos, veo qe son enteros => entero

 y qe hace con esa respuesta? la devuelve, y la dejo anotada en el nodo


el ejemplo godeto seria

int x;
x = 2 + 3;
el parse contruye el arbol. osea la primer pasada
── [tipoDato: T_INT ]  DECL (x)
└── [tipoDato: T_ERROR]  = (x)
    └── [tipoDato: T_ERROR]  +
        ├── [tipoDato: T_ERROR]  NUM (2)
        └── [tipoDato: T_ERROR]  NUM (3)

nomas el decl tiene el algo
ahora la segunda pasada resolverNombres
recorro el arbol y hace 2 cosas 
cuando llega al decl saca el tipo, y lo usa para añadir la variable a la tabla de simbolos
agregarSimbolo(nodo->tipoDato, nodo->nombre, nodo->linea); aca.
y la tabla seria por ahora
pos 0 - x : int


tercer recorrido - chequearTipos
terminar de validar

chequeatTipos recorre y calcula el tipo d cada expresion de las hojas para arriba 
NUM(2) es un N_NUM entonces T_INT
NUM 3 lo mismo
+ pregunta a los hijos, son int etnonces la usma es int
= (x) el hijo dio int, la tabla dice qe x es int coincide todo piola
y el resultado lo deja en el nodo
entonces el arbol qeda 
├── [tipoDato: T_INT]  DECL (x)      ← no se toca
└── [tipoDato: T_INT]  = (x)         ← calculado
    └── [tipoDato: T_INT]  +         ← calculado
        ├── [tipoDato: T_INT]  NUM (2)   ← calculado
        └── [tipoDato: T_INT]  NUM (3)   ← calculado


y dsp el interprete
recorre el arbol y calcula 