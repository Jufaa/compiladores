#   make          compila (solo lo que cambio)
#   make test     compila y corre tests/prueba.txt
#   make clean    borra todo lo generado
#   make run ARCHIVO=tests/otro.txt

CC     = gcc
CFLAGS = -Wall -Isrc -Ibuild

SRC   = src
BUILD = build
BIN   = compilador

# Modulos propios: uno por cada .c de src/
MODULOS = ast tablaSimbolos semantico interprete codigoIntermedio main

# Objetos: los modulos propios mas los dos generados
OBJETOS = $(MODULOS:%=$(BUILD)/%.o) $(BUILD)/bison.tab.o $(BUILD)/flex.o

$(BIN): $(OBJETOS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJETOS)

# --- Generados por bison y flex ---
# bison -d produce el .c y el .h a la vez.
$(BUILD)/bison.tab.c $(BUILD)/bison.tab.h: $(SRC)/bison.y | $(BUILD)
	bison -d -o $(BUILD)/bison.tab.c $(SRC)/bison.y

# flex.l incluye bison.tab.h, asi que bison tiene que correr antes.
$(BUILD)/flex.c: $(SRC)/flex.l $(BUILD)/bison.tab.h | $(BUILD)
	flex -o $(BUILD)/flex.c $(SRC)/flex.l

# --- Compilacion a objetos ---
# Fuentes propias. Dependen del header de bison para forzar el orden.
$(BUILD)/%.o: $(SRC)/%.c $(BUILD)/bison.tab.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Fuentes generadas, que ya viven en build/.
$(BUILD)/%.o: $(BUILD)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

# --- Utilidades ---
ARCHIVO ?= tests/prueba.txt

# El guion inicial: prueba.txt tiene errores a proposito, y el compilador
# devuelve codigo != 0. Sin el guion, make lo tomaria como fallo del build.
test: $(BIN)
	-./$(BIN) $(ARCHIVO)

run: $(BIN)
	./$(BIN) $(ARCHIVO)

clean:
	rm -rf $(BUILD) $(BIN)

.PHONY: test run clean
