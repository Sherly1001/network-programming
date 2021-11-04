#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "account.h"

typedef struct sockaddr_in sockaddr_in_t;

typedef struct map_sockaddr_acc {
    sockaddr_in_t addr;
    char *username;
    account *acc;
    struct map_sockaddr_acc *next;
} map_sockaddr_acc_t;

// return 0 if ok, return 1 if replace
int msa_insert(map_sockaddr_acc_t **map, sockaddr_in_t addr, char *username, account *acc);
// return 0 if ok, return 1 if not exist
int msa_remove(map_sockaddr_acc_t **map, sockaddr_in_t addr);
account *msa_get(map_sockaddr_acc_t *map, sockaddr_in_t addr);
map_sockaddr_acc_t *msa_get_ref(map_sockaddr_acc_t *map, sockaddr_in_t addr);

// return 1 if error
int split_passwd(const char *passwd, char *digits, char *alphabets) {
    while (*passwd++) {
        if (isdigit(passwd[-1])) *digits++ = passwd[-1];
        else if (isalpha(passwd[-1])) *alphabets++ = passwd[-1];
        else return 1;
    }
    *digits = '\0';
    *alphabets = '\0';
    return 0;
}

int main(int ag, char **av) {
    if (ag < 2) {
        printf("missing port param\n");
        exit(1);
    }

    int port = atoi(av[1]);

    sockaddr_in_t addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(port);

    int sdf = socket(AF_INET, SOCK_DGRAM, 0);
    if (sdf < 0) {
        perror("Error: ");
        exit(1);
    }
    if (bind(sdf, (struct sockaddr*)&addr, sizeof(addr))) {
        printf("can't bind socket address to port %d\n", port);
        exit(1);
    };

    printf("listening at port: %d\n", port);

    linklist *l = NULL;
    load_acounts(&l, ACC_FILE);

    map_sockaddr_acc_t *map = NULL;

    int res = 0;
    char buff[1024];
    sockaddr_in_t from;
    socklen_t fromlen;

    while ((res = recvfrom(sdf, buff, 1024, 0, (struct sockaddr*)&from, &fromlen)) >= 0) {
        buff[res] = '\0';
        printf("got a msg from client [%s:%d]: %s\n", inet_ntoa(from.sin_addr), from.sin_port, buff);
        // sendto(sdf, buff, res, 0, (struct sockaddr*)&from, fromlen);

        map_sockaddr_acc_t *m = msa_get_ref(map, from);

        if (!strcmp(buff, "bye")) {
            msa_remove(&map, from);
            sprintf(buff, "Goodbye %s", m ? m->username : "");
            sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
            continue;
        }

        if (m == NULL) {
            account *found = find_account(&l, buff);
            if (found == NULL) {
                strcpy(buff, "Account not found");
                sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
                continue;
            }

            if (found->stt == BLOCKED) {
                strcpy(buff, "Account not ready");
                sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
                continue;
            }

            msa_insert(&map, from, NULL, found);
            strcpy(buff, "Insert password");
            sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
            continue;
        }

        if (m->username == NULL) {
            if (strcmp(m->acc->password, buff)) {
                m->acc->wrong_pass++;
                if (m->acc->wrong_pass >= 3) {
                    block_account(m->acc);
                    save_acounts(&l, ACC_FILE);
                    msa_remove(&map, from);
                    strcpy(buff, "Account is blocked");
                    sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
                    continue;
                }
                save_acounts(&l, ACC_FILE);
                sendto(sdf, "Not OK", 6, 0, (struct sockaddr*)&from, fromlen);
                continue;
            }

            m->username = m->acc->username;
            sendto(sdf, "OK", 2, 0, (struct sockaddr*)&from, fromlen);
            continue;
        }

        char digits[50], alphabets[50];
        if (split_passwd(buff, digits, alphabets)) {
            sendto(sdf, "Error", 5, 0, (struct sockaddr*)&from, fromlen);
            continue;
        }

        change_passwd(m->acc, m->acc->password, buff);
        save_acounts(&l, ACC_FILE);
        sprintf(buff, "%s\n%s", digits, alphabets);
        sendto(sdf, buff, strlen(buff), 0, (struct sockaddr*)&from, fromlen);
    }

    perror("Error: ");
}



int is_same_addr(sockaddr_in_t a, sockaddr_in_t b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

// return 0 if ok, return 1 if replace
int msa_insert(map_sockaddr_acc_t **map, sockaddr_in_t addr, char *username, account *acc) {
    if (*map == NULL) {
        *map = malloc(sizeof(map_sockaddr_acc_t));
        (*map)->addr = addr;
        (*map)->username = username;
        (*map)->acc = acc;
        (*map)->next = NULL;
        return 0;
    }

    map_sockaddr_acc_t *m = *map, *r = NULL;
    while (m != NULL && !is_same_addr(m->addr, addr)) {
        r = m;
        m = m->next;
    }

    if (m != NULL) {
        m->username = username;
        m->acc = acc;
        return 1;
    }

    m = r->next = malloc(sizeof(map_sockaddr_acc_t));
    m->addr = addr;
    m->username = username;
    m->acc = acc;
    m->next = NULL;
    return 0;
}

int msa_remove(map_sockaddr_acc_t **map, sockaddr_in_t addr) {
    map_sockaddr_acc_t *m = *map, *r = NULL;
    while (m != NULL && !is_same_addr(m->addr, addr)) {
        r = m;
        m = m->next;
    }

    if (m == NULL) return 1;
    if (r == NULL) {
        free(m);
        *map = NULL;
        return 0;
    }

    r->next = m->next;
    free(m);
    return 0;
}

account *msa_get(map_sockaddr_acc_t *map, sockaddr_in_t addr) {
    map_sockaddr_acc_t *rs = msa_get_ref(map, addr);
    if (rs == NULL) return NULL;
    return rs->acc;
}

map_sockaddr_acc_t *msa_get_ref(map_sockaddr_acc_t *map, sockaddr_in_t addr) {
    while (map != NULL && !is_same_addr(map->addr, addr)) {
        map = map->next;
    }
    return map;
}
