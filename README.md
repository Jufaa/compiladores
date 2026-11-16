# Compilador

Integrantes: 
Ammann Berruti, Sebastián
Cerutti Norris, Ignacio
Francitorra, Juan Cruz

Compilacion del preproyecto:

```
make                              compila unicamente nuevos cambios
make test                         ejecuta tests/interprete2.txt
make run ARCHIVO=tests/otro.txt   corre el archivo especificado
make asm ARCHIVO=tests/otro.txt   genera, muestra, compila y ejecuta assembly
make clean                        limpia la build
```

compilar y correr a mano un archivo:
```
make
./compilador tests/interprete2.txt
```

generar y ejecutar assembler

```bash
make asm ARCHIVO=tests/nombre.txt
```