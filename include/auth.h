#ifndef AUTH_H
#define AUTH_H

#include "types.h"

int  load_users(User users[], int *count);
int  save_users(const User users[], int count);

int  login(User users[], int count,
           const char *username, const char *password,
           User *logged_in_out);

int  register_user(User users[], int *count,
                   const char *username, const char *password,
                   const char *full_name, const char *email,
                   int role);

int  change_password(User users[], int count, int user_id,
                     const char *old_password, const char *new_password);

void admin_user_menu(User users[], int *count, const User *current_user);
void passenger_profile_menu(User users[], int *count, const User *current_user);

#endif /* AUTH_H */
