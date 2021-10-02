CC = gcc
CFLAGS = -c -Wall

main: main.o
	${CC} main.o -o main

main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o

clean:
	rm -fr *.o
