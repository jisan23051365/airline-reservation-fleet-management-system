#include <stdio.h>
#include <string.h>

#include "auth.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* File I/O                                                             */
/* ------------------------------------------------------------------ */

int load_users(User users[], int *count)
{
    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) {
        *count = 0;
        return 0; /* No file yet — not an error on first run. */
    }
    *count = (int)fread(users, sizeof(User), MAX_USERS, fp);
    fclose(fp);
    return 1;
}

int save_users(const User users[], int count)
{
    ensure_data_dir();
    FILE *fp = fopen(USERS_FILE, "wb");
    if (!fp) {
        printf("  [ERROR] Could not open users file for writing.\n");
        return 0;
    }
    fwrite(users, sizeof(User), (size_t)count, fp);
    fclose(fp);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Authentication                                                       */
/* ------------------------------------------------------------------ */

int login(User users[], int count,
          const char *username, const char *password,
          User *logged_in_out)
{
    for (int i = 0; i < count; i++) {
        if (users[i].active &&
            strcmp(users[i].username, username) == 0 &&
            verify_password(password, users[i].username, users[i].password_hash)) {
            *logged_in_out = users[i];
            return 1;
        }
    }
    return 0;
}

int register_user(User users[], int *count,
                  const char *username, const char *password,
                  const char *full_name, const char *email,
                  int role)
{
    if (*count >= MAX_USERS) {
        printf("  [ERROR] User limit reached.\n");
        return 0;
    }
    /* Check for duplicate username. */
    for (int i = 0; i < *count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("  [ERROR] Username '%s' is already taken.\n", username);
            return 0;
        }
    }

    /* Find next available id. */
    int max_id = 0;
    for (int i = 0; i < *count; i++) {
        if (users[i].id > max_id) max_id = users[i].id;
    }

    User *u = &users[*count];
    u->id   = max_id + 1;
    strncpy(u->username,  username,  MAX_STRING - 1);
    strncpy(u->full_name, full_name, MAX_STRING - 1);
    strncpy(u->email,     email,     MAX_STRING - 1);
    u->username[MAX_STRING - 1]  = '\0';
    u->full_name[MAX_STRING - 1] = '\0';
    u->email[MAX_STRING - 1]     = '\0';
    hash_password(password, username, u->password_hash);
    u->role   = role;
    u->active = 1;
    (*count)++;
    return 1;
}

int change_password(User users[], int count, int user_id,
                    const char *old_password, const char *new_password)
{
    for (int i = 0; i < count; i++) {
        if (users[i].id == user_id) {
            if (!verify_password(old_password, users[i].username,
                                  users[i].password_hash)) {
                printf("  [ERROR] Current password is incorrect.\n");
                return 0;
            }
            hash_password(new_password, users[i].username,
                          users[i].password_hash);
            return 1;
        }
    }
    printf("  [ERROR] User not found.\n");
    return 0;
}

/* ------------------------------------------------------------------ */
/* Admin: user management menu                                          */
/* ------------------------------------------------------------------ */

void admin_user_menu(User users[], int *count, const User *current_user)
{
    int choice;
    do {
        print_header("USER MANAGEMENT");
        printf("  1. View All Users\n");
        printf("  2. Add Admin User\n");
        printf("  3. Deactivate User\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1:
            print_header("ALL USERS");
            printf("  %-5s %-20s %-25s %-30s %-12s %-8s\n",
                   "ID", "Username", "Full Name", "Email", "Role", "Status");
            print_separator();
            for (int i = 0; i < *count; i++) {
                printf("  %-5d %-20s %-25s %-30s %-12s %-8s\n",
                       users[i].id, users[i].username, users[i].full_name,
                       users[i].email,
                       users[i].role == ROLE_ADMIN ? "Admin" : "Passenger",
                       users[i].active ? "Active" : "Inactive");
            }
            printf("\nPress Enter to continue...");
            getchar();
            break;

        case 2: {
            char username[MAX_STRING], password[MAX_STRING];
            char full_name[MAX_STRING], email[MAX_STRING];
            get_string_input("  New admin username : ", username, MAX_STRING);
            get_string_input("  Password           : ", password, MAX_STRING);
            get_string_input("  Full name          : ", full_name, MAX_STRING);
            get_string_input("  Email              : ", email, MAX_STRING);
            if (register_user(users, count, username, password,
                              full_name, email, ROLE_ADMIN)) {
                save_users(users, *count);
                printf("  Admin user '%s' created successfully.\n", username);
            }
            break;
        }

        case 3: {
            int uid = get_int_input("  Enter user ID to deactivate: ");
            int found = 0;
            for (int i = 0; i < *count; i++) {
                if (users[i].id == uid) {
                    if (users[i].id == current_user->id) {
                        printf("  [ERROR] Cannot deactivate yourself.\n");
                    } else {
                        users[i].active = 0;
                        save_users(users, *count);
                        printf("  User %d deactivated.\n", uid);
                    }
                    found = 1;
                    break;
                }
            }
            if (!found) printf("  [ERROR] User ID %d not found.\n", uid);
            break;
        }

        case 0:
            break;

        default:
            printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

/* ------------------------------------------------------------------ */
/* Passenger: profile / password menu                                   */
/* ------------------------------------------------------------------ */

void passenger_profile_menu(User users[], int *count, const User *current_user)
{
    int choice;
    do {
        print_header("MY PROFILE");
        printf("  1. View My Profile\n");
        printf("  2. Change Password\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1:
            print_header("PROFILE DETAILS");
            for (int i = 0; i < *count; i++) {
                if (users[i].id == current_user->id) {
                    printf("  ID        : %d\n",  users[i].id);
                    printf("  Username  : %s\n",  users[i].username);
                    printf("  Full Name : %s\n",  users[i].full_name);
                    printf("  Email     : %s\n",  users[i].email);
                    break;
                }
            }
            printf("\nPress Enter to continue...");
            getchar();
            break;

        case 2: {
            char old_pw[MAX_STRING], new_pw[MAX_STRING], confirm[MAX_STRING];
            get_string_input("  Current password    : ", old_pw, MAX_STRING);
            get_string_input("  New password        : ", new_pw, MAX_STRING);
            get_string_input("  Confirm new password: ", confirm, MAX_STRING);
            if (strcmp(new_pw, confirm) != 0) {
                printf("  [ERROR] Passwords do not match.\n");
            } else if (change_password(users, *count,
                                       current_user->id, old_pw, new_pw)) {
                save_users(users, *count);
                printf("  Password changed successfully.\n");
            }
            break;
        }

        case 0:
            break;

        default:
            printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
