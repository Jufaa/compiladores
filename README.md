
bison -d bison.y
flex flex.l 
gcc bison.tab.c lex.yy.c -o compilador -lfl
./compilador prueba.txt




