
CFLAGS = -Wall -g
CC = gcc

all:	coolc

coolc:  coolc.tab.o lex.yy.o symbol.o nodes.o codegen.o inter.o
	gcc $(CFLAGS) -o $@ $^


lex.yy.c:	lex.l coolc.tab.h
		flex lex.l

coolc.tab.c:	coolc.y
		bison -v -d $^

nodes.o:	nodes.c nodes.h
symbol.o:	symbol.c symbol.h
codegen.o: 	codegen.c codegen.h
inter.o:  	inter.c inter.h

clean:
	rm -f lex.yy.c coolc.tab.[hc] *.o coolc coolc.output out-int.txt *.c~ *.h~
