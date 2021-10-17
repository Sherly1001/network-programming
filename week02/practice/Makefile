CC = gcc
CFLAGS = -c -Wall -l socket

main: main.o
	${CC} main.o -o resolver

main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o

clean:
	rm -fr *.o
