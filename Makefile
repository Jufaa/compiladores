#   make          compila (solo lo que cambio)
#   make test     compila y corre tests/interprete2.txt
#   make clean    borra todo lo generado
#   make run ARCHIVO=tests/otro.txt
#   make asm ARCHIVO=tests/otro.txt  genera, muestra y ejecuta assembly

CC     = gcc
CFLAGS = -Wall -Isrc -Ibuild

SRC   = src
BUILD = build
BIN   = compilador

# Modulos propios: uno por cada .c de src/
MODULOS = ast tablaSimbolos semantico interprete codigoIntermedio optimizador assembler main

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
ARCHIVO ?= tests/interprete2.txt

test: $(BIN)
	./$(BIN) "$(ARCHIVO)"

run: $(BIN)
	./$(BIN) $(ARCHIVO)

# Un main que devuelve, por ejemplo, 120 no es un fallo de compilacion.
# Se informa su codigo de salida sin hacer fallar make por ese valor.
asm: ARCHIVO = tests/interprete2.txt
asm: $(BIN) | $(BUILD)
	./$(BIN) "$(ARCHIVO)" --asm "$(BUILD)/programa.s"
	@printf '\n--- ASSEMBLER ---\n'
	@cat "$(BUILD)/programa.s"
	$(CC) "$(BUILD)/programa.s" -o "$(BUILD)/programa"
	@printf '\n--- EJECUCION ASSEMBLER ---\n'
	@./$(BUILD)/programa; resultado=$$?; printf 'Codigo de salida del programa: %s\n' "$$resultado"

clean:
	rm -rf $(BUILD) $(BIN)

.PHONY: test run asm clean
