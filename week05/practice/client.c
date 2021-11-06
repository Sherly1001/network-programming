#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int ag, char **av) {
    if (ag < 3) {
        printf("missing %d param\nUsage: %s <ip addr> <port>\n", 3 - ag, av[0]);
        exit(1);
    }

    struct in_addr addr;
    if (!inet_aton(av[1], &addr)) {
        printf("wrong ip address format\n");
        exit(1);
    }

    char *end = NULL;
    long port = strtol(av[2], &end, 10);
    if (end == av[2] || *end != '\0' || port < 0 || port > 65535) {
        printf("wrong port format\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = addr;
    server_addr.sin_port = htons(port);

    int sdf = socket(AF_INET, SOCK_STREAM, 0);
    if (sdf < 0) {
        perror("Create socket error: ");
        exit(1);
    }

    if (connect(sdf, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect error: ");
        exit(1);
    }

    char buff[1024];
    int res = 0, quit = 0;

    while (!quit) {
        printf("send to server: ");
        scanf(" %[^\n]s", buff);
        if (!strcmp(buff, "bye")) quit = 1;

        if (send(sdf, buff, strlen(buff), 0) < 0) {
            perror("\nSend error: ");
            exit(1);
        }

        if ((res = recv(sdf, buff, 1024, 0)) < 0) {
            perror("\nRecv error: ");
            exit(1);
        }
        buff[res] = '\0';
        printf("\ngot a msg from server:\n%s\n\n", buff);
    }
}
