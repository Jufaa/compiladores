
bison -d bison.y
flex flex.l 
gcc bison.tab.c lex.yy.c -o compilador -lfl //// modulado == gcc lex.yy.c bison.tab.c ast.c ts.c -o mi_compilador
./compilador prueba.txt /// modulado === ./mi_compilador prueba.txt     




