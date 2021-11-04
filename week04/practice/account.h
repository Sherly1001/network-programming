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

typedef struct {
    char username[20];
    char password[20];
    status stt;
    int wrong_pass;
    int wrong_active;
    char homepage[20];
} account;

typedef struct linklist_ {
    account acc;
    struct linklist_ *next;
} linklist;


account *insert(linklist **l, account acc);
void load_acounts(linklist **l, char *filename);
void save_acounts(linklist **l, char *filename);
void show_accounts(linklist **l);

account *register_account(linklist **l, account *acc);
account *active_account(account *acc, char *active_code);
account *block_account(account *acc);
account *singin(linklist **l, account *acc);
account *find_account(linklist **l, char *username);
account *change_passwd(account *acc, char *old_passwd, char *new_passwd);
int signout(account *acc, char *username);
