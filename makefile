CC=gcc
CFLAGS= -Wall -g

default:
	$(CC) $(CFLAGS) -o ifj.out codegen.c expressionParse.c origin.c semantic.c stack.c symtable.c synCheck.c tokenizer.c

clean:
	-@rm *.out
	-@rm *.o

