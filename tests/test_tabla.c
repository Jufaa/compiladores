#include <stdio.h>
#include "src/tablaSimbolos.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

static void testLlamadas(const char *compilador) {
    const char *casos[][2] = {
        {"int a; void main(){a();}", "'a' no es una funcion"},
        {"void main(){int a;a();}", "'a' no es una funcion"},
        {"void f(int a){a();} void main(){}", "'a' no es una funcion"},
        {"int f(){return 1;} void main(){int f;f();}", "'f' no es una funcion"},
        {"int f(int a){return a;} void main(){f();}", "espera 1 argumentos, recibio 0"},
        {"int f(){return 1;} void main(){f(1);}", "espera 0 argumentos, recibio 1"},
        {"int f(int a){return a;} void main(){f(true);}", "argumento 1"},
        {"int f(int a,boolean b){return a;} void main(){f(1,2);}", "argumento 2"},
        {"int f(int a,boolean b){return a;} void main(){f(true,1);}", "argumento 1"},
        {"int f(int a){return a;} void main(){f(1,2);}", "espera 1 argumentos, recibio 2"},
        {"void f(){} int g(int a){return a;} void main(){g(f());}", "argumento 1"},
        {"void main(){missing();}", "funcion 'missing' no declarada"},
        {"void first(){later();} void later(){}", NULL},
        {"int f(int a){return a;} void main(){f(missing());}", "funcion 'missing' no declarada"},
        {"void f(){} void main(){f();}", NULL},
        {"int f(int a){return a;} void main(){int x;x=f(2);}", NULL},
        {"int f(int a,boolean b){return a;} void main(){f(1,true);}", NULL},
        {"float f(float a){return a;} void main(){float x;x=f(1.5);}", NULL},
        {"int id(int a){return a;} int sum(int a,int b){return a+b;} void main(){sum(id(1),id(2));}", NULL},
        {"int f(int a){if(a==0){return 0;}return f(a-1);} void main(){f(2);}", NULL},
        {"int x; void f(int a){int y;y=a;} void main(){x=2+3*4;f(x);}", NULL},
        {"void main(){{int x;x=1;}x=2;}", "variable 'x' no declarada"},
        {"void main(){{int x;x=1;}{int y;y=x;}}", "variable 'x' no declarada"},
        {"void main(){if(true){int x;x=1;}x=2;}", "variable 'x' no declarada"},
        {"void main(){if(true){int x;x=1;}else{x=2;}}", "variable 'x' no declarada"},
        {"void main(){while(false){int x;x=1;}x=2;}", "variable 'x' no declarada"},
        {"void main(){int x;int x;}", "'x' ya fue declarada"},
        {"void f(int a,int a){} void main(){}", "'a' ya fue declarada"},
        {"void main(){int x;{boolean x;x=true;}x=1;}", NULL},
        {"void main(){{int x;x=1;}{boolean x;x=true;}}", NULL},
        {"int x; void main(){{boolean x;x=true;}x=2;}", NULL},
        {"void f(int a){boolean a;a=true;} void main(){f(1);}", NULL},
        {"void f(int a){{boolean a;a=true;}a=1;} void main(){f(1);}", NULL},
        {"int f(){return 1;} void main(){{int f;f=2;}f();}", NULL},
        {"int x; void main(){x=1;{int y;y=x;{y=x;}}}", NULL},
        {"void main(){{}{}while(false){}if(true){}else{}}", NULL},
        {"void main(){boolean b;b=1+2!=4+5;}", NULL},
        {"void main(){boolean b;b=1+2<=4+5;}", NULL},
        {"void main(){boolean b;b=1+2>=4+5;}", NULL},
        {"void main(){boolean b;b=1+2==4+5;}", NULL},
        {"void main(){boolean b;b=1+2<4+5;}", NULL},
        {"void main(){boolean b;b=1+2>4+5;}", NULL}
    };
    int fallos = 0;
    size_t cantidad = sizeof(casos) / sizeof(casos[0]);
    for (size_t i = 0; i < cantidad; i++) {
        char entrada[] = "/tmp/compiler-call-XXXXXX";
        int fd = mkstemp(entrada);
        FILE *salida = tmpfile();
        if (fd < 0 || salida == NULL) {
            perror("temporary file");
            exit(1);
        }
        FILE *fuente = fdopen(fd, "w");
        if (fuente == NULL) {
            perror("fdopen");
            exit(1);
        }
        fputs(casos[i][0], fuente);
        fclose(fuente);
        fflush(NULL);
        pid_t pid = fork();
        if (pid == 0) {
            if (dup2(fileno(salida), STDOUT_FILENO) < 0 ||
                dup2(fileno(salida), STDERR_FILENO) < 0)
                _exit(126);
            execl(compilador, compilador, entrada, (char *)NULL);
            _exit(127);
        }
        int estado;
        if (pid < 0 || waitpid(pid, &estado, 0) < 0) {
            perror("compiler process");
            exit(1);
        }
        unlink(entrada);
        rewind(salida);
        char texto[32768];
        size_t leidos = fread(texto, 1, sizeof(texto) - 1, salida);
        texto[leidos] = '\0';
        fclose(salida);
        int esperado = casos[i][1] ? 1 : 0;
        int correcto = WIFEXITED(estado) && WEXITSTATUS(estado) == esperado;
        if (esperado)
            correcto = correcto && strstr(texto, casos[i][1]) != NULL &&
                       strstr(texto, "--- CODIGO INTERMEDIO ---") == NULL;
        else
            correcto = correcto && strstr(texto, "Sin errores semanticos.") != NULL &&
                       strstr(texto, "--- CODIGO INTERMEDIO ---") != NULL;
        if (!correcto) {
            fprintf(stderr, "FAIL call case %zu: %s\n%s\n", i + 1, casos[i][0], texto);
            fallos++;
        }
    }
    printf("Call and scope cases: %zu, failures: %d\n", cantidad, fallos);
    if (fallos)
        exit(1);
}

static void testFirmas(void) {
    inicializarTabla();
    abrirNivel();
    enum TipoDato tipos[] = {T_INT, T_BOOL};
    Simbolo *funcion = agregarSimbolo(V_FUNCION, T_INT, "f", 1, 2, tipos);
    assert(funcion != NULL && funcion->cantidadParametros == 2);
    assert(funcion->tipoParametros != tipos);
    tipos[0] = T_FLOAT;
    assert(funcion->tipoParametros[0] == T_INT);
    assert(funcion->tipoParametros[1] == T_BOOL);
    Simbolo *variable = agregarSimbolo(V_GLOBAL, T_INT, "x", 1, 0, NULL);
    assert(variable != NULL && variable->cantidadParametros == 0);
    assert(variable->tipoParametros == NULL);
    assert(agregarSimbolo(V_GLOBAL, T_INT, "x", 2, 0, NULL) == NULL);
    assert(agregarSimbolo(V_FUNCION, T_INT, "bad", 1, 1, NULL) == NULL);
    cerrarNivel();
    assert(vacia() && CANTSimbolos == 0);
    assert(funcion->cantidadParametros == 2);
    assert(funcion->tipoParametros[0] == T_INT);
    assert(buscarSimbolo("f") == NULL);
    liberarTabla();
}

static void testVidaAmbitos(void) {
    inicializarTabla();
    abrirNivel();
    Simbolo *global = agregarSimbolo(V_GLOBAL, T_INT, "x", 1, 0, NULL);
    assert(global != NULL && pila->nivel == 0);
    abrirNivel();
    Simbolo *parametro = agregarSimbolo(V_PARAM, T_BOOL, "a", 1, 0, NULL);
    assert(parametro != NULL && pila->nivel == 1);
    abrirNivel();
    Simbolo *local = agregarSimbolo(V_LOCAL, T_BOOL, "x", 2, 0, NULL);
    assert(local != NULL && pila->nivel == 2);
    assert(buscarSimbolo("x") == local && buscarSimbolo("a") == parametro);
    cerrarNivel();
    assert(buscarSimbolo("x") == global);
    assert(local->tipoDato == T_BOOL && strcmp(local->nombre, "x") == 0);
    abrirNivel();
    Simbolo *hermano = agregarSimbolo(V_LOCAL, T_FLOAT, "x", 3, 0, NULL);
    assert(hermano != NULL && hermano != local);
    assert(buscarSimbolo("x") == hermano && local->tipoDato == T_BOOL);
    cerrarNivel();
    cerrarNivel();
    assert(buscarSimbolo("a") == NULL && parametro->tipoDato == T_BOOL);
    assert(CANTSimbolos == 1);
    liberarTabla();
    assert(vacia() && CANTSimbolos == 0);
    liberarTabla();
    abrirNivel();
    agregarSimbolo(V_GLOBAL, T_INT, "reset", 1, 0, NULL);
    inicializarTabla();
    assert(vacia() && CANTSimbolos == 0);
}



void testGlobal2() {
    printf("Test: variables globales \n");

    abrirNivel(); 
    printf("Agregados 2 Juan\n");
    printf("Genera error: \n");
    agregarSimbolo(V_GLOBAL, T_INT, "Juan", 1, 0, NULL);
    agregarSimbolo(V_GLOBAL, T_INT, "Juan", 5, 0, NULL);

    imprimirTabla();
}

void testGlobal() {
    printf("Test: variables globales \n");

    abrirNivel(); // nivel 0 
    agregarSimbolo(V_GLOBAL, T_INT, "x", 1, 0, NULL);
    agregarSimbolo(V_GLOBAL, T_INT, "y", 2, 0, NULL);
    printf("Agregados x e y como globales en nivel 0\n");

    imprimirTabla();
}

void testFuncionSinParams() {
    printf("\nTest: funcion sin parametros \n");

    // Nivel 1 - nivel de param (vacio en este parte)
    abrirNivel();
    printf("Nivel 1 abierto (parametros - vacio)\n");

    // Nivel 2 - nivel para el cuerpo de la funcion
    abrirNivel();
    printf("Nivel 2 abierto cuerpo de la funcion\n");

    agregarSimbolo(V_LOCAL, T_INT, "resultado", 10, 0, NULL);
    printf("Agregado resultado en nivel 2 cuerpo\n");

    imprimirTabla();

    // cerrar body funcion
    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo)\n");

    // cerrar param
    cerrarNivel();
    printf("Nivel 1 cerrado (parametros)\n");

    imprimirTabla();
}

void testFuncionConParams() {
    printf("\nTest: funcion con parametros \n");

    // Nivel 1 - nivel de parametros
    abrirNivel();
    printf("Nivel 1 abierto (parametros)\n");

    agregarSimbolo(V_PARAM, T_INT, "a", 20, 0, NULL);
    agregarSimbolo(V_PARAM, T_INT, "b", 21, 0, NULL);
    printf("Agregados parametros a y b en nivel 1\n");

    imprimirTabla();

    // Nivel 2 - nivel del cuerpo de la funcion
    abrirNivel();
    printf("Nivel 2 abierto cuerpo de la funcion\n");

    agregarSimbolo(V_LOCAL, T_INT, "resultado", 22, 0, NULL);
    printf("Agregado resultado en nivel 2 cuerpo\n");

    // shadowing: a existe en nivel 1 (param), pero se permite en nivel 2 (cuerpo)
    agregarSimbolo(V_LOCAL, T_INT, "a", 23, 0, NULL);
    printf("Agregado a en nivel 2 (shadowing del param)\n");

    imprimirTabla();

    // verificar que a existe en ambos niveles (shadowing)
    int a_cuerpo = buscarSimboloEnUnNivel("a", 2);
    int a_param = buscarSimboloEnUnNivel("a", 1);
    printf("\nVerificacion de shadowing:\n");
    printf("a en nivel 2 (cuerpo): %s\n", a_cuerpo == 1 ? "encontrado" : "no encontrado");
    printf("a en nivel 1 (param):  %s\n", a_param == 1 ? "encontrado" : "no encontrado");

    // duplicado en nivel 2: resultado ya existe, deberia fallar
    printf("\nIntentando declarar resultado de nuevo en nivel 2 (deberia fallar):\n");
    Simbolo *res = agregarSimbolo(V_LOCAL, T_INT, "resultado", 24, 0, NULL);
    if (res == NULL) {
        printf("OK: duplicado detectado en nivel 2\n");
    }

    // cerrar cuerpo
    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo)\n");

    // cerrar parametros
    cerrarNivel();
    printf("Nivel 1 cerrado (parametros)\n");
}

void testMain() {
    printf("\n Test: funcion main \n");

    // Nivel 1 - parametros de main (vacio)
    abrirNivel();
    printf("Nivel 1 abierto (parametros de main - vacio)\n");

    // Nivel 2 - cuerpo de main
    abrirNivel();
    printf("Nivel 2 abierto (cuerpo de main)\n");

    agregarSimbolo(V_LOCAL, T_INT, "z", 30, 0, NULL);
    printf("Agregado 'z' en nivel 2 (cuerpo de main)\n");

    // buscar variable global desde main
    Simbolo *global = buscarSimbolo("x");
    printf("\nBuscando global 'x' desde main: %s\n", global != NULL ? "encontrado" : "NULL");

    // buscar parametro de otra funcion (no deberia encontrar)
    Simbolo *param = buscarSimbolo("a");
    printf("Buscando param 'a' de otra funcion: %s\n", param != NULL ? "encontrado (incorrecto)" : "NULL (correcto)");

    imprimirTabla();

    cerrarNivel();
    printf("Nivel 2 cerrado (cuerpo de main)\n");

    cerrarNivel();
    printf("Nivel 1 cerrado (parametros de main)\n");
}

int main(int argc, char **argv) {
    if (argc > 1)
        testLlamadas(argv[1]);
    testFirmas();
    testVidaAmbitos();
    testGlobal2();
    testGlobal();
    testFuncionSinParams();
    testFuncionConParams();
    testMain();

    printf("\n Estado final de la tabla \n");
    imprimirTabla();

    printf("\n Todos los tests completados \n");
    liberarTabla();
    return 0;
}
