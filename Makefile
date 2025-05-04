# Makefile

MyBash: main.o parser.o executor.o utils.o
	gcc -o MyBash main.o parser.o executor.o utils.o

main.o: shell.h main.c
	gcc -c -o main.o main.c

parser.o: shell.h parser.c
	gcc -c -o parser.o parser.c

executor.o: shell.h executor.c
	gcc -c -o executor.o executor.c

utils.o: shell.h utils.c
	gcc -c -o utils.o utils.c

