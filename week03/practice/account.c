#include "account.h"

void show_accounts(linklist **l) {
    for (linklist *node = *l; node != NULL; node = node->next) {
        printf("%s\t%s\t%d\t%s\n", node->acc.username, node->acc.password, node->acc.stt, node->acc.homepage);
    }
    printf("\n");
}

account *insert(linklist **l, account acc) {
    linklist *node = *l;
    linklist *prev = NULL;
    while (node != NULL) {
        if (!strcmp(node->acc.username, acc.username)) {
            break;
        }
        prev = node;
        node = node->next;
    }

    if (prev == NULL) { // list is empty
        *l = malloc(sizeof(linklist));
        (*l)->acc = acc;
        (*l)->next = NULL;
        return &(*l)->acc;
    } else if (node != NULL) { // username existed
        return NULL;
    } else {
        prev->next = malloc(sizeof(linklist));
        prev->next->acc = acc;
        prev->next->next = NULL;
        return &prev->next->acc;
    }
}

void load_acounts(linklist **l, char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("File %s not found.", filename);
        exit(1);
    }

    account acc = {};
    char line[50];
    while(fscanf(f, " %[^\r\n]s", line) > 0) {
        int code = 0;
        if (sscanf(line, "%s %s %d %s", acc.username, acc.password, &code, acc.homepage) == 4) {
            acc.stt = code % 10;
            code /= 10;
            acc.wrong_pass = code % 10;
            code /= 10;
            acc.wrong_active = code % 10;
            insert(l, acc);
        }
    }

    fclose(f);
}

void save_acounts(linklist **l, char *filename) {
    FILE *f = fopen(filename, "w");
    for (linklist *node = *l; node != NULL; node = node->next) {
        int code = (int)node->acc.stt + node->acc.wrong_pass * 10 + node->acc.wrong_active * 100;
        fprintf(f, "%s %s %d %s\n", node->acc.username, node->acc.password, code, node->acc.homepage);
    }
    fclose(f);
}


account *register_account(linklist **l, account *acc) {
    acc->stt = IDLE;
    return insert(l, *acc);
}

account *active_account(account *acc, char *active_code) {
    if (strcmp(active_code, ACTIVE_CODE) != 0) return NULL;
    acc->stt = ACTIVE;
    return acc;
}

account *block_account(account *acc) {
    acc->stt = BLOCKED;
    return acc;
}

account *singin(linklist **l, account *acc) {
    account *found = find_account(l, acc->username);
    if (found != NULL && !strcmp(found->password, acc->password)) return found;
    return NULL;
}

account *find_account(linklist **l, char *username) {
    for (linklist *node = *l; node != NULL; node = node->next) {
        if (!strcmp(node->acc.username, username)) return &node->acc;
    }
    return NULL;
}

account *change_passwd(account *acc, char *old_passwd, char *new_passwd) {
    if (!strcmp(acc->password, old_passwd)) {
        strcpy(acc->password, new_passwd);
        return acc;
    }
    return NULL;
}

int signout(account *acc, char *username) {
    return strcmp(acc->username, username);
}
