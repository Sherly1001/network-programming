#include <sys/socket.h> // gethost
#include <netdb.h>      // struct hostent
#include <arpa/inet.h>  // inet_ntoa

#include "account.h"

void show_choices() {
    printf("\n----------------USER MANAGEMENT PROGRAM-------------------\n"
           "1. Register\n2. Activate\n3. Sign in\n4. Search\n5. Change password\n"
           "6. Sign out\n7. Homepage with domain name\n8. Homepage with IP address\n"
           "Your choice (1-8, other to quit): ");
}

static char stt_map[][10] = { "BLOCKED", "ACTIVE", "IDLE" };

struct hostent *get_homepage_info(account *acc) {
    struct hostent *host = NULL;
    struct in_addr addr;
    if (inet_aton(acc->homepage, &addr)) {
        host = gethostbyaddr(&addr, 4, AF_INET);
    } else {
        host = gethostbyname2(acc->homepage, AF_INET);
    }
    return host;
}

void show_domain_info(struct hostent *host) {
    if (host == NULL) {
        printf("Not found info.\n");
        return;
    }
    printf("Official name: %s\n", host->h_name);
    printf("Alias name:\n");
    for (char **alias = host->h_aliases; *alias != NULL; ++alias) {
        printf("%s\n", *alias);
    }
}

void show_ip_info(struct hostent *host) {
    if (host == NULL) {
        printf("Not found info.\n");
        return;
    }
    struct in_addr addr;
    printf("ip:\n");
    if (host->h_addrtype == AF_INET) {
        for (char **paddr = host->h_addr_list; *paddr != NULL; ++paddr) {
            addr.s_addr = *(in_addr_t*)(*paddr);
            printf("%s\n", inet_ntoa(addr));
        }
    }
}

int main() {
    linklist *l = NULL;
    account *curr_acc = NULL;
    account *found = NULL;
    account tmp_acc;

    char username[20];
    char old_passwd[20];
    char new_passwd[20];

    char active_code[10];

    struct hostent *host = NULL;

    load_acounts(&l, ACC_FILE);

    int choice = 0;
    while (1) {
        choice = 0;
        show_choices();
        scanf(" %d", &choice);
        if (choice < 1 || choice > 8) break;

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
                printf("Enter homepage: ");
                scanf(" %s", found->homepage);
                printf("Created account, active required.\n");
                save_acounts(&l, ACC_FILE);
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

            while (1) {
                printf("Enter password: ");
                scanf(" %s", tmp_acc.password);
                if (!strcmp(found->password, tmp_acc.password)) break;

                found->wrong_pass++;
                printf("Wrong password %d time(s).\n", found->wrong_pass);
                save_acounts(&l, ACC_FILE);

                if (found->wrong_pass > 3) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;
            found->wrong_pass = 0;
            save_acounts(&l, ACC_FILE);

            while (1) {
                printf("Enter active code: ");
                scanf(" %s", active_code);
                if (active_account(found, active_code) != NULL) break;

                printf("Wrong active code %d time(s).\n", found->wrong_active);
                save_acounts(&l, ACC_FILE);

                if (found->wrong_active > 4) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;
            found->wrong_active = 0;
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

            while (1) {
                printf("Enter password: ");
                scanf(" %s", tmp_acc.password);
                if (!strcmp(found->password, tmp_acc.password)) break;

                found->wrong_pass++;
                printf("Wrong password %d time(s).\n", found->wrong_pass);
                save_acounts(&l, ACC_FILE);

                if (found->wrong_pass > 3) {
                    printf("Blocked account.\n");
                    block_account(found);
                    save_acounts(&l, ACC_FILE);
                    found = NULL;
                    break;
                }
            }
            if (found == NULL) break;
            found->wrong_pass = 0;
            save_acounts(&l, ACC_FILE);

            curr_acc = found;
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
            break;
        case 7:
            if (curr_acc == NULL) {
                printf("You are not singed in.\n");
                break;
            }
            printf("Homepage with domain name: ");
            host = get_homepage_info(curr_acc);
            show_domain_info(host);
            break;
        case 8:
            if (curr_acc == NULL) {
                printf("You are not singed in.\n");
                break;
            }
            printf("Homepage with IP address: ");
            host = get_homepage_info(curr_acc);
            show_ip_info(host);
        }
    };

    save_acounts(&l, ACC_FILE);
}
