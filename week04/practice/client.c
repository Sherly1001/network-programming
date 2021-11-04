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

    int port = atoi(av[2]);

    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(av[1]);
    server_addr.sin_port = htons(port);

    int sdf = socket(AF_INET, SOCK_DGRAM, 0);
    if (sdf < 0) {
        perror("Error: ");
        exit(1);
    }

    char buff[1024];
    int res = 0, quit = 0;

    while (!quit) {
        printf("send to server: ");
        scanf(" %[^\n]s", buff);
        if (!strcmp(buff, "bye")) quit = 1;

        if (sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("\nSend error: ");
        }

        if ((res = recvfrom(sdf, buff, 1024, 0, (struct sockaddr*)&server_addr, &server_addr_len)) < 0) {
            perror("\nRecv error: ");
        }
        buff[res] = '\0';
        printf("\ngot a msg from server:\n%s\n\n", buff);
    }
}
