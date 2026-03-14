#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "auth.h"
#include "flights.h"
#include "aircraft.h"
#include "bookings.h"
#include "analytics.h"

/* ------------------------------------------------------------------ */
/* Global in-memory data stores                                         */
/* ------------------------------------------------------------------ */

static User     g_users[MAX_USERS];
static Flight   g_flights[MAX_FLIGHTS];
static Aircraft g_aircraft[MAX_AIRCRAFT];
static Booking  g_bookings[MAX_BOOKINGS];

static int g_user_count    = 0;
static int g_flight_count  = 0;
static int g_aircraft_count = 0;
static int g_booking_count  = 0;

/* ------------------------------------------------------------------ */
/* Load all data from disk                                              */
/* ------------------------------------------------------------------ */

static void load_all_data(void)
{
    load_users(g_users, &g_user_count);
    load_flights(g_flights, &g_flight_count);
    load_aircraft(g_aircraft, &g_aircraft_count);
    load_bookings(g_bookings, &g_booking_count);
}

/* ------------------------------------------------------------------ */
/* Seed a default admin account when no users exist                    */
/* ------------------------------------------------------------------ */

static void seed_default_admin(void)
{
    if (g_user_count > 0) return;

    /* SECURITY NOTE: The default credentials below are only used on the very
     * first run when no user database exists.  Change them immediately after
     * first login using the "User Management -> Change Password" option, or
     * delete data/users.dat to re-seed with fresh credentials. */
    printf("\n  No users found. Creating default admin account...\n");
    register_user(g_users, &g_user_count,
                  "admin", "admin123",
                  "System Administrator", "admin@airline.com",
                  ROLE_ADMIN);
    save_users(g_users, g_user_count);
    printf("  Default admin created.  Username: admin  Password: admin123\n");
    printf("  ** IMPORTANT: Change this password immediately after login. **\n\n");
}

/* ------------------------------------------------------------------ */
/* Admin main menu                                                      */
/* ------------------------------------------------------------------ */

static void admin_menu(const User *current_user)
{
    int choice;
    do {
        print_header("ADMIN MENU");
        printf("  Logged in as: %s (%s)\n\n",
               current_user->full_name, current_user->username);
        printf("  1. Flight Management\n");
        printf("  2. Aircraft Management\n");
        printf("  3. Booking Management\n");
        printf("  4. Analytics & Reports\n");
        printf("  5. User Management\n");
        printf("  0. Logout\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1:
            flight_management_menu(g_flights, &g_flight_count,
                                   g_aircraft, g_aircraft_count);
            break;
        case 2:
            aircraft_management_menu(g_aircraft, &g_aircraft_count);
            break;
        case 3:
            admin_booking_menu(g_bookings, &g_booking_count,
                               g_flights, &g_flight_count);
            break;
        case 4:
            analytics_menu(g_bookings, g_booking_count,
                           g_flights, g_flight_count);
            break;
        case 5:
            admin_user_menu(g_users, &g_user_count, current_user);
            break;
        case 0:
            printf("  Logging out...\n");
            break;
        default:
            printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

/* ------------------------------------------------------------------ */
/* Passenger main menu                                                  */
/* ------------------------------------------------------------------ */

static void passenger_menu(const User *current_user)
{
    int choice;
    do {
        print_header("PASSENGER MENU");
        printf("  Welcome, %s!\n\n", current_user->full_name);
        printf("  1. Flights & Bookings\n");
        printf("  2. My Profile\n");
        printf("  0. Logout\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1:
            passenger_booking_menu(g_bookings, &g_booking_count,
                                   g_flights, &g_flight_count,
                                   current_user);
            break;
        case 2:
            passenger_profile_menu(g_users, &g_user_count, current_user);
            break;
        case 0:
            printf("  Logging out...\n");
            break;
        default:
            printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

/* ------------------------------------------------------------------ */
/* Authentication screen                                                */
/* ------------------------------------------------------------------ */

static void auth_screen(void)
{
    int choice;
    do {
        print_header("AIRLINE RESERVATION & FLEET MANAGEMENT SYSTEM");
        printf("  1. Login\n");
        printf("  2. Register (Passenger)\n");
        printf("  0. Exit\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: {
            char username[MAX_STRING], password[MAX_STRING];
            get_string_input("  Username : ", username, MAX_STRING);
            get_string_input("  Password : ", password, MAX_STRING);

            User logged_in;
            if (login(g_users, g_user_count, username, password, &logged_in)) {
                printf("\n  Login successful. Welcome, %s!\n\n",
                       logged_in.full_name);
                if (logged_in.role == ROLE_ADMIN)
                    admin_menu(&logged_in);
                else
                    passenger_menu(&logged_in);
            } else {
                printf("  [ERROR] Invalid username or password.\n\n");
            }
            break;
        }

        case 2: {
            char username[MAX_STRING], password[MAX_STRING];
            char confirm[MAX_STRING], full_name[MAX_STRING], email[MAX_STRING];
            print_header("PASSENGER REGISTRATION");
            get_string_input("  Username         : ", username,  MAX_STRING);
            get_string_input("  Password         : ", password,  MAX_STRING);
            get_string_input("  Confirm password : ", confirm,   MAX_STRING);
            if (strcmp(password, confirm) != 0) {
                printf("  [ERROR] Passwords do not match.\n");
                break;
            }
            get_string_input("  Full name        : ", full_name, MAX_STRING);
            get_string_input("  Email            : ", email,     MAX_STRING);

            if (register_user(g_users, &g_user_count, username, password,
                              full_name, email, ROLE_PASSENGER)) {
                save_users(g_users, g_user_count);
                printf("  Registration successful! You can now log in.\n\n");
            }
            break;
        }

        case 0:
            printf("  Thank you for using the Airline System. Goodbye!\n");
            break;

        default:
            printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

/* ------------------------------------------------------------------ */
/* Entry point                                                          */
/* ------------------------------------------------------------------ */

int main(void)
{
    ensure_data_dir();
    load_all_data();
    seed_default_admin();
    auth_screen();
    return 0;
}
