#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

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
int handle_connection(void *args);

typedef struct {
    int sdf;
    sockaddr_in_t from;
    socklen_t fromlen;
    linklist **l;
    map_sockaddr_acc_t **map;
} handle_args;

typedef struct map_sockdf_args {
    int sdf;
    handle_args args;
    struct map_sockdf_args *next;
} map_sockdf_args_t;

// return 0 if ok, return 1 if replace
int msarg_insert(map_sockdf_args_t **map, int sdf, handle_args args);
// return 0 if ok, return 1 if not exist
int msarg_remove(map_sockdf_args_t **map, int sdf);
handle_args *msarg_get(map_sockdf_args_t *map, int sdf);

int lc_sdf = 0;
map_sockdf_args_t **lc = NULL;
void sigint_handler() {
    while (lc && *lc) {
        map_sockdf_args_t **pre = &(*lc)->next;
        close((*lc)->sdf);
        free(*lc);
        lc = pre;
    }
    close(lc_sdf);
    exit(0);
}

int main(int ag, char **av) {
    if (ag < 2) {
        printf("missing port param\n");
        exit(1);
    }

    signal(SIGCLD, SIG_IGN);
    signal(SIGINT, sigint_handler);

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

    // int opt = 1;
    // setsockopt(sdf, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    if (bind(sdf, (struct sockaddr*)&addr, sizeof(addr))) {
        printf("can't bind socket address to port %d\n", port);
        perror("Error: ");
        exit(1);
    };

    if (listen(sdf, 100)) {
        perror("Listen error: ");
        exit(1);
    }
    lc_sdf = sdf;

    printf("listening at port: %d\n", port);

    linklist *l = NULL;

    map_sockaddr_acc_t *map = NULL;
    map_sockdf_args_t *map_args = NULL;

    lc = &map_args;

    sockaddr_in_t from;
    socklen_t fromlen;

    int rc = 0, max_df = sdf;
    fd_set rdf;
    struct timeval timeout;

    FD_ZERO(&rdf);
    timeout.tv_sec  = 3 * 60;
    timeout.tv_usec = 0;

    int end_server = 0;
    while (!end_server) {
        FD_SET(sdf, &rdf);
        for (map_sockdf_args_t *m = map_args; m; m = m->next) {
            FD_SET(m->sdf, &rdf);
        }

        rc = select(max_df + 1, &rdf, NULL, NULL, &timeout);
        if (rc < 0) {
            perror("select");
            break;
        } else if (rc == 0) {
            perror("select timeout");
            break;
        }

        int desc_ready = rc;
        for (int df = 0; df <= max_df && desc_ready > 0; ++df) {
            if (!FD_ISSET(df, &rdf)) continue;
            desc_ready -= 1;

            if (df == sdf) {
                int new_sd = accept(sdf, (struct sockaddr*)&from, &fromlen);
                if (new_sd < 0) {
                    perror("accept failed");
                    end_server = 1;
                    break;
                }
                printf("new connection from client [%s:%d]\n", inet_ntoa(from.sin_addr), from.sin_port);
                FD_SET(new_sd, &rdf);
                if (new_sd > max_df) max_df = new_sd;

                handle_args args = { new_sd, from, fromlen, &l, &map };
                msarg_insert(&map_args, new_sd, args);
            } else if (handle_connection(msarg_get(map_args, df))) {
                close(df);
                FD_CLR(df, &rdf);
                msarg_remove(&map_args, df);
                if (df == max_df) max_df -= 1;
            }
        }
    }

    close(sdf);
}

// return 0 is ok
int handle_connection(void *args) {
    if (!args) return -1;

    handle_args *hargs = args;
    int sdf = hargs->sdf;
    sockaddr_in_t from = hargs->from;
    linklist **l = hargs->l;
    map_sockaddr_acc_t **map = hargs->map;

    char buff[1024];

    int res = 0;
    // while (1) {
    res = recv(sdf, buff, 1024, 0);
    if (res < 0) {
        perror("recv failed");
        return -1;
    } else if (res == 0) {
        return 0;
    }

    buff[res] = '\0';
    printf("got a msg from client [%s:%d]: %s\n", inet_ntoa(from.sin_addr), from.sin_port, buff);
    // send(sdf, buff, res, 0);

    load_acounts(l, ACC_FILE);
    map_sockaddr_acc_t *m = msa_get_ref(*map, from);

    if (!strcmp(buff, "bye")) {
        msa_remove(map, from);

        sprintf(buff, "Goodbye %s", m ? m->username : "");
        res = send(sdf, buff, strlen(buff), 0);
        return 1;
        // break;
    }

    if (m == NULL) {
        account *found = find_account(l, buff);

        if (found == NULL) {
            strcpy(buff, "Account not found");
            return send(sdf, buff, strlen(buff), 0) <= 0;
            // continue;
        }

        if (found->stt == BLOCKED) {
            strcpy(buff, "Account not ready");
            return send(sdf, buff, strlen(buff), 0) <= 0;
            // continue;
        }

        msa_insert(map, from, NULL, found);

        strcpy(buff, "Insert password");
        return send(sdf, buff, strlen(buff), 0) <= 0;
        // continue;
    }

    if (m->username == NULL) {
        if (strcmp(m->acc->password, buff)) {
            m->acc->wrong_pass++;
            if (m->acc->wrong_pass >= 3) {
                block_account(m->acc);
                save_acounts(l, ACC_FILE);

                msa_remove(map, from);

                strcpy(buff, "Account is blocked");
                return send(sdf, buff, strlen(buff), 0) <= 0;
                // continue;
            }
            save_acounts(l, ACC_FILE);

            return send(sdf, "Not OK", 6, 0) <= 0;
            // continue;
        }

        m->username = m->acc->username;
        return send(sdf, "OK", 2, 0) <= 0;
        // continue;
    }

    char digits[50], alphabets[50];
    if (split_passwd(buff, digits, alphabets)) {
        return send(sdf, "Error", 5, 0) <= 0;
        // continue;
    }

    change_passwd(m->acc, m->acc->password, buff);
    save_acounts(l, ACC_FILE);

    sprintf(buff, "%s\n%s", digits, alphabets);

    int len = strlen(buff);
    for (map_sockdf_args_t *m = *lc; m; m = m->next) {
        if (send(m->sdf, buff, len, 0) <= 0) return 1;
    }

    return 0;
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

int msarg_insert(map_sockdf_args_t **map, int sdf, handle_args args) {
    if (*map == NULL) {
        *map = malloc(sizeof(map_sockdf_args_t));
        (*map)->sdf = sdf;
        (*map)->args = args;
        (*map)->next = NULL;
        return 0;
    }

    map_sockdf_args_t *m = *map, *r = NULL;
    while (m != NULL && m->sdf != sdf) {
        r = m;
        m = m->next;
    }

    if (m != NULL) {
        m->args = args;
        return 1;
    }

    m = r->next = malloc(sizeof(map_sockdf_args_t));
    m->sdf = sdf;
    m->args = args;
    m->next = NULL;
    return 0;
}

int msarg_remove(map_sockdf_args_t **map, int sdf) {
    map_sockdf_args_t *m = *map, *r = NULL;
    while (m != NULL && m->sdf != sdf) {
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

handle_args *msarg_get(map_sockdf_args_t *map, int sdf) {
    while (map != NULL && map->sdf != sdf) {
        map = map->next;
    }
    return map ? &map->args : NULL;
}
