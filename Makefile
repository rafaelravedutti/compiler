$DEPURA=1

compilador: lex.yy.c y.tab.c compilador.o compilador.h
	gcc lex.yy.c compilador.tab.c compilador.o -o compilador -lfl -ly -lc -Wall

lex.yy.c: compilador.l compilador.h
	flex compilador.l

y.tab.c: compilador.y compilador.h
	bison compilador.y -d -v

compilador.o : compilador.h compiladorF.c
	gcc -c compiladorF.c -o compilador.o

clean : 
	rm -f compilador.tab.* lex.yy.c compilador compilador.o 
