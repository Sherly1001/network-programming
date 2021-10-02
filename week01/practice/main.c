#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACC_FILE "accounts.txt"
#define ACTIVE_CODE "20184138"

typedef enum status_ {
    BLOCKED,
    ACTIVE,
    IDLE
} status;

static char stt_map[][10] = { "BLOCKED", "ACTIVE", "IDLE" };

typedef struct {
    char username[20];
    char password[20];
    status stt;
} account;

typedef struct linklist_ {
    account acc;
    struct linklist_ *next;
} linklist;


account *insert(linklist **l, account acc);
void load_acounts(linklist **l, char *filename);
void save_acounts(linklist **l, char *filename);
void show_accounts(linklist **l);

void show_choices();
account *register_account(linklist **l, account *acc);
account *active_account(account *acc, char *active_code);
account *block_account(account *acc);
account *singin(linklist **l, account *acc);
account *find_account(linklist **l, char *username);
account *change_passwd(account *acc, char *old_passwd, char *new_passwd);
int signout(account *acc, char *username);

int main() {
    linklist *l = NULL;
    account *curr_acc = NULL;
    account *found = NULL;
    account tmp_acc;

    char username[20];
    char old_passwd[20];
    char new_passwd[20];

    char active_code[10];
    int counter = 0;

    load_acounts(&l, ACC_FILE);

    int choice = 0;
    while (1) {
        choice = 0;
        show_choices();
        scanf(" %d", &choice);
        if (choice < 1 || choice > 6) break;

        switch (choice) {
        case 1:
            tmp_acc = (account){};
            printf("Enter new username: ");
            scanf(" %s", tmp_acc.username);
            found = find_account(&l, tmp_acc.username);
            if (found != NULL) {
                printf("This username existed.\n");
                break;
            }
            printf("Enter new password: ");
            scanf(" %s", tmp_acc.password);
            found = register_account(&l, &tmp_acc);
            if (found == NULL) {
                printf("This username existed.\n");
            } else {
                printf("Created account, active required.\n");
            }
            break;
        case 2:
            printf("Enter username: ");
            scanf(" %s", tmp_acc.username);
            found = find_account(&l, tmp_acc.username);
            if (found == NULL) {
                printf("Username '%s' not existed.\n", tmp_acc.username);
                break;
            }

            counter = 0;
            while (1) {
                printf("Enter password: ");
                scanf(" %s", tmp_acc.password);
                if (!strcmp(found->password, tmp_acc.password)) break;

                counter += 1;
                printf("Wrong password %d time(s).\n", counter);
                if (counter >= 3) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;

            counter = 0;
            while (1) {
                printf("Enter active code: ");
                scanf(" %s", active_code);
                if (active_account(found, active_code) != NULL) break;

                counter += 1;
                printf("Wrong active code %d time(s).\n", counter);
                if (counter >= 4) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;
            counter = 0;
            printf("Account actived.\n");
            save_acounts(&l, ACC_FILE);
            break;
        case 3:
            if (curr_acc != NULL) {
                printf("You are signed in.\n");
                break;
            }
            printf("Enter username: ");
            scanf(" %s", tmp_acc.username);
            found = find_account(&l, tmp_acc.username);
            if (found == NULL) {
                printf("Username '%s' not existed.\n", tmp_acc.username);
                break;
            }

            if (found->stt == BLOCKED) {
                printf("This account is blocked.\n");
                break;
            }

            counter = 0;
            while (1) {
                printf("Enter password: ");
                scanf(" %s", tmp_acc.password);
                if (!strcmp(found->password, tmp_acc.password)) break;

                counter += 1;
                printf("Wrong password %d time(s).\n", counter);
                if (counter >= 3) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;

            curr_acc = found;
            counter = 0;
            printf("Signed in.\nHello %s, your account status: %s\n", curr_acc->username, stt_map[curr_acc->stt]);
            break;
        case 4:
            if (curr_acc == NULL) {
                printf("You are not signed in.\n");
                break;
            }
            printf("Enter username to search: ");
            scanf(" %s", username);
            found = find_account(&l, username);
            if (found == NULL) {
                printf("Not found account with username '%s'.\n", username);
            } else {
                printf("Account: %s, status %s.\n", username, stt_map[found->stt]);
            }
            break;
        case 5:
            if (curr_acc == NULL) {
                printf("You are not signed in.\n");
                break;
            }
            printf("Enter old password: ");
            scanf(" %s", old_passwd);
            printf("Enter new password: ");
            scanf(" %s", new_passwd);
            found = change_passwd(curr_acc, old_passwd, new_passwd);
            if (found == NULL) {
                printf("Old password not match.\n");
                break;
            }
            printf("Password changed.\n");
            curr_acc = found;
            save_acounts(&l, ACC_FILE);
            break;
        case 6:
            if (curr_acc == NULL) {
                printf("You are not singed in.\n");
                break;
            }
            printf("Enter current username to sign out: ");
            scanf(" %s", username);
            if (signout(curr_acc, username)) {
                printf("Username not match.\n");
            } else {
                printf("Singed out. Goodby %s.\n", username);
                curr_acc = NULL;
            }
        }
    };

    save_acounts(&l, ACC_FILE);
}


void show_choices() {
    printf("\n----------------USER MANAGEMENT PROGRAM-------------------\n"
           "1. Register\n2. Activate\n3. Sign in\n4. Search\n5. Change password\n"
           "6. Sign out\nYour choice (1-6, other to quit): ");
}

void show_accounts(linklist **l) {
    for (linklist *node = *l; node != NULL; node = node->next) {
        printf("%s\t%s\t%d\n", node->acc.username, node->acc.password, node->acc.stt);
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

    account acc;
    char line[50];
    while(fscanf(f, " %[^\r\n]s", line) > 0) {
        if (sscanf(line, "%s %s %d", acc.username, acc.password, (int*)&acc.stt) == 3) {
            insert(l, acc);
        }
    }

    fclose(f);
}

void save_acounts(linklist **l, char *filename) {
    FILE *f = fopen(filename, "w");
    for (linklist *node = *l; node != NULL; node = node->next) {
        fprintf(f, "%s %s %d\n", node->acc.username, node->acc.password, node->acc.stt);
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
