#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

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
int split_passwd(const char *passwd, char *digits, char *alphabets);
void *handle_connection(void *args);

typedef struct {
    int sdf;
    sockaddr_in_t from;
    socklen_t fromlen;
    linklist *l;
    pthread_mutex_t *mutex_list;
    map_sockaddr_acc_t *map;
    pthread_mutex_t *mutex_map;
} handle_args;

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

    int sdf = socket(AF_INET, SOCK_STREAM, 0);
    if (sdf < 0) {
        perror("Create socket error: ");
        exit(1);
    }

    if (bind(sdf, (struct sockaddr*)&addr, sizeof(addr))) {
        printf("can't bind socket address to port %d\n", port);
        perror("Error: ");
        exit(1);
    };

    if (listen(sdf, 100)) {
        perror("Listen error: ");
        exit(1);
    }

    printf("listening at port: %d\n", port);

    linklist *l = NULL;
    load_acounts(&l, ACC_FILE);

    map_sockaddr_acc_t *map = NULL;

    pthread_mutex_t mutex_list = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_map = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tid[100];

    sockaddr_in_t from;
    socklen_t fromlen;

    for (int i = 0;1;) {
        int new_client_sdf = accept(sdf, (struct sockaddr*)&from, &fromlen);
        if (new_client_sdf < 0) {
            perror("accept error: ");
            continue;
        }

        handle_args args = { new_client_sdf, from, fromlen, l, &mutex_list, map, &mutex_map };
        if (pthread_create(tid + i++, NULL, handle_connection, &args)) {
            printf("failed to create new thread\n");
        }

        if (i >= 100) {
            for (int t = 0; t < 100; ++t) pthread_join(tid[t], NULL);
            i = 0;
        }
    }

    perror("Error: ");
    close(sdf);
}

void *handle_connection(void *args) {
    handle_args *hargs = args;
    int sdf = hargs->sdf;
    sockaddr_in_t from = hargs->from;
    linklist *l = hargs->l;
    map_sockaddr_acc_t *map = hargs->map;

    char buff[1024];

    int res = 0;
    while ((res = recv(sdf, buff, 1024, 0)) >= 0) {
        buff[res] = '\0';
        printf("got a msg from client [%s:%d]: %s\n", inet_ntoa(from.sin_addr), from.sin_port, buff);
        // send(sdf, buff, res, 0);

        pthread_mutex_lock(hargs->mutex_map);
        map_sockaddr_acc_t *m = msa_get_ref(map, from);
        pthread_mutex_unlock(hargs->mutex_map);

        if (!strcmp(buff, "bye")) {
            pthread_mutex_lock(hargs->mutex_map);
            msa_remove(&map, from);
            pthread_mutex_unlock(hargs->mutex_map);

            sprintf(buff, "Goodbye %s", m ? m->username : "");
            send(sdf, buff, strlen(buff), 0);
            break;
        }

        if (m == NULL) {
            pthread_mutex_lock(hargs->mutex_list);
            account *found = find_account(&l, buff);
            pthread_mutex_unlock(hargs->mutex_list);

            if (found == NULL) {
                strcpy(buff, "Account not found");
                send(sdf, buff, strlen(buff), 0);
                continue;
            }

            if (found->stt == BLOCKED) {
                strcpy(buff, "Account not ready");
                send(sdf, buff, strlen(buff), 0);
                continue;
            }

            pthread_mutex_lock(hargs->mutex_map);
            msa_insert(&map, from, NULL, found);
            pthread_mutex_unlock(hargs->mutex_map);

            strcpy(buff, "Insert password");
            send(sdf, buff, strlen(buff), 0);
            continue;
        }

        if (m->username == NULL) {
            if (strcmp(m->acc->password, buff)) {
                m->acc->wrong_pass++;
                if (m->acc->wrong_pass >= 3) {
                    pthread_mutex_lock(hargs->mutex_list);
                    block_account(m->acc);
                    save_acounts(&l, ACC_FILE);
                    pthread_mutex_unlock(hargs->mutex_list);

                    pthread_mutex_lock(hargs->mutex_map);
                    msa_remove(&map, from);
                    pthread_mutex_unlock(hargs->mutex_map);

                    strcpy(buff, "Account is blocked");
                    send(sdf, buff, strlen(buff), 0);
                    continue;
                }
                pthread_mutex_lock(hargs->mutex_list);
                save_acounts(&l, ACC_FILE);
                pthread_mutex_unlock(hargs->mutex_list);

                send(sdf, "Not OK", 6, 0);
                continue;
            }

            m->username = m->acc->username;
            send(sdf, "OK", 2, 0);
            continue;
        }

        char digits[50], alphabets[50];
        if (split_passwd(buff, digits, alphabets)) {
            send(sdf, "Error", 5, 0);
            continue;
        }

        pthread_mutex_lock(hargs->mutex_list);
        change_passwd(m->acc, m->acc->password, buff);
        save_acounts(&l, ACC_FILE);
        pthread_mutex_unlock(hargs->mutex_list);

        sprintf(buff, "%s\n%s", digits, alphabets);
        send(sdf, buff, strlen(buff), 0);
    }

    close(sdf);
    pthread_exit(NULL);
}


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
