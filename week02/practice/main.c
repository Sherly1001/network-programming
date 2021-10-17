#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // gethost
#include <netdb.h>      // struct hostent
#include <arpa/inet.h>  // inet_ntoa

void show_ip_info(struct hostent *host) {
    printf("Official name: %s\n", host->h_name);
    printf("Alias name:\n");
    for (char **alias = host->h_aliases; *alias != NULL; ++alias) {
        printf("%s\n", *alias);
    }
}

void show_domain_info(struct hostent *host) {
    struct in_addr addr;
    printf("ip:\n");
    if (host->h_addrtype == AF_INET) {
        for (char **paddr = host->h_addr_list; *paddr != NULL; ++paddr) {
            addr.s_addr = *(in_addr_t*)(*paddr);
            printf("%s\n", inet_ntoa(addr));
        }
    }
}

int main(int ac, char **av) {
    if (ac < 3) {
        printf("Miss %d param.\nUsage: %s (1|2) (ip|domain name)\n", 3 - ac, av[0]);
        exit(1);
    }

    int option = 0;
    struct hostent *host = NULL;
    struct in_addr addr;

    int is_ip_addr = inet_aton(av[2], &addr);

    if (sscanf(av[1], "%d", &option) == 1) {
        switch (option) {
        case 1:
            if (!is_ip_addr) {
                printf("Wrong parameter.\n");
                exit(1);
            }
            host = gethostbyaddr(&addr, 4, AF_INET);
            if (host == NULL) {
                printf("Not found infomation.\n");
                exit(1);
            }
            show_ip_info(host);
            break;
        case 2:
            if (is_ip_addr) {
                printf("Wrong parameter.\n");
                exit(1);
            }
            host = gethostbyname2(av[2], AF_INET);
            if (host == NULL) {
                printf("Not found infomation.\n");
                exit(1);
            }
            show_domain_info(host);
            break;
        default:
            printf("First param must be 1 or 2.\n");
            exit(1);
        }
    } else {
            printf("First param must be 1 or 2.\n");
            exit(1);
    }
}
