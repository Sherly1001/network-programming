CC = gcc
CFLAGS = -c -Wall

main: account.o main.o
	${CC} main.o account.o -o main

main.o: main.c
	${CC} ${CFLAGS} main.c -o main.o

account.o: account.c
	${CC} ${CFLAGS} account.c -o account.o

clean:
	rm -fr *.o
