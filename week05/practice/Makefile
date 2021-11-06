CC = gcc
CFLAGS = -c -Wall -l socket

all: server client

server: server.o account.o
	${CC} server.o account.o -pthread -o server

server.o: server.c
	${CC} ${CFLAGS} server.c -o server.o

account.o: account.c
	${CC} ${CFLAGS} account.c -o account.o

client: client.o
	${CC} client.o -o client

client.o: client.c
	${CC} ${CFLAGS} client.c -o client.o

clean:
	rm -fr *.o ./server ./client
