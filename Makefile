CC = gcc
CFLAGS = -O2

main: main.o src.o
	gcc -o main main.o src.o

main.o: main.c
	gcc -c -o main.o main.c

src.o: src.c
	gcc -c -o src.o src.c
