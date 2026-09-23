# Cómo se arman los `SEQ` en el árbol

## 1. Qué es un `SEQ`

Un `SEQ` **no es una sentencia**. Es un **pegamento** que une dos cosas.

Significa: *"hacé esto, **y después** aquello"*.

Cada sentencia ya es su propio nodo:

| Código        | Nodo    |
|---------------|---------|
| `y = 4;`      | `=`     |
| `suma = 0;`   | `=`     |
| `while (...)` | `WHILE` |

El `SEQ` solo sirve para engancharlas.

## 2. Un `SEQ` tiene exactamente 2 hijos

Como un nodo tiene solo `izq` y `der`, un `SEQ` puede unir **dos cosas y nada más**.

Si hay más cosas, se meten `SEQ` adentro de otros `SEQ`.

Pensalo como un tren: las sentencias son los **vagones** y los `SEQ` son los **enganches**.

```
[y = 4] ─🔗─ [suma = 0] ─🔗─ [while]
```

3 vagones → 2 enganches. **Siempre hay un `SEQ` menos que sentencias.**

| Sentencias | `SEQ` |
|------------|-------|
| 1          | 0     |
| 2          | 1     |
| 3          | 2     |
| 4          | 3     |

## 3. Cuándo se crea un `SEQ`

La regla en `bison.y` es:

```
STATEMENTS: %empty                { $$ = NULL; }
          | STATEMENTS STATEMENT  { $$ = ($1 == NULL) ? $2 : crearNodo(N_SEQ, ..., $1, $2, ...); }
```

- `$1` = lo que ya se juntó hasta ahora
- `$2` = la sentencia que acaba de llegar

**Regla:** se crea un `SEQ` cada vez que llega una sentencia **y ya había algo antes**.

La primera sentencia no crea `SEQ`, porque no tiene con qué pegarse.

### Paso por paso

```c
y = 4;
suma = 0;
while (y > 0) { ... }
```

| Llega      | ¿Había algo antes? | Qué pasa                       | Queda juntado                      |
|------------|--------------------|--------------------------------|------------------------------------|
| (arranque) | —                  | —                              | `NULL`                             |
| `y = 4`    | No                 | No se crea `SEQ`               | `y = 4`                            |
| `suma = 0` | Sí                 | Se crea el **SEQ nº 1**        | `SEQ(y = 4, suma = 0)`             |
| `while`    | Sí                 | Se crea el **SEQ nº 2**        | `SEQ( SEQ(y = 4, suma = 0), while )` |

Lo nuevo siempre va a la **derecha**. Todo lo anterior queda envuelto a la **izquierda**.

## 4. Cuándo frena

Frena cuando llega la **`}`** que cierra el bloque.

Después de cada sentencia, bison mira el próximo token:

- Si es un id, `if`, `while`, `return`, `;` o `{` → puede empezar otra sentencia → **sigue**.
- Si es `}` → ninguna sentencia empieza con `}` → **la lista terminó**.

Ahí bison toma todo lo que juntó y lo usa como `$3` del bloque:

```
BLOCK: TLLAVEA VAR_DECLS STATEMENTS TLLAVEC
                           $3        └── la } que la cortó
```

Cada bloque junta **solo lo que está entre su `{` y su `}`**. Por eso lo que está adentro de un `while` tiene sus propios `SEQ`, separados de los del método.

## 5. Por qué en un `BLOQUE` aparecen dos `SEQ` uno al lado del otro

El `BLOQUE` guarda **dos listas separadas**:

```
BLOCK: TLLAVEA VAR_DECLS STATEMENTS TLLAVEC
                  $2         $3
```

| Hijo  | Qué guarda                                |
|-------|-------------------------------------------|
| `izq` | las **declaraciones** (`int y; int suma;`) |
| `der` | las **sentencias** (`y = 4;`, `while`...)  |

Cada lista tiene sus propios `SEQ`.

## 6. Ejemplo completo

Código (`tests/while.txt`):

```c
void main() {
int y;
int suma;
y = 4;
suma = 0;
while (y > 0) {
suma = suma + y;
y = dec(y);
}
}
```

Árbol:

```
BLOQUE
├── SEQ                    ← declaraciones (2 cosas → 1 SEQ)
│   ├── DECL (y)
│   └── DECL (suma)
└── SEQ                    ← sentencias (3 cosas → 2 SEQ)
    ├── SEQ
    │   ├── = (y)
    │   └── = (suma)
    └── WHILE
        ├── >              ← la condición
        └── BLOQUE         ← el cuerpo del while (su propia lista)
            └── SEQ        ← 2 sentencias → 1 SEQ
                ├── = (suma)
                └── = (y)
```

Las dos líneas de adentro del `while` **no cuentan** como sentencias del `main`. Todo el `while` es **una sola** sentencia del `main`.

## 7. Cómo contar los hijos en el árbol impreso

Cada hijo arranca con `├──` o con `└──`.

- Un `SEQ` siempre tiene **un** `├──` y **un** `└──`: sus 2 hijos.
- Lo que está más corrido a la derecha es hijo del nodo de arriba, no del de más afuera.

## Resumen

- `SEQ` = pegamento, significa "y después".
- Tiene siempre **2 hijos**.
- Se crea cuando llega una sentencia **y ya había algo antes**.
- **Un `SEQ` menos** que la cantidad de sentencias.
- Frena en la **`}`** del bloque.
- Cada bloque tiene **su propia** lista.
