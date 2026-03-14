#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "flights.h"
#include "aircraft.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* File I/O                                                             */
/* ------------------------------------------------------------------ */

int load_flights(Flight flights[], int *count)
{
    FILE *fp = fopen(FLIGHTS_FILE, "rb");
    if (!fp) { *count = 0; return 0; }
    *count = (int)fread(flights, sizeof(Flight), MAX_FLIGHTS, fp);
    fclose(fp);
    return 1;
}

int save_flights(const Flight flights[], int count)
{
    ensure_data_dir();
    FILE *fp = fopen(FLIGHTS_FILE, "wb");
    if (!fp) { printf("  [ERROR] Cannot write flights file.\n"); return 0; }
    fwrite(flights, sizeof(Flight), (size_t)count, fp);
    fclose(fp);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Lookup                                                               */
/* ------------------------------------------------------------------ */

Flight *find_flight_by_id(Flight flights[], int count, int id)
{
    for (int i = 0; i < count; i++) {
        if (flights[i].id == id) return &flights[i];
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Display helpers                                                      */
/* ------------------------------------------------------------------ */

static void print_flight_row(const Flight *f)
{
    printf("  %-5d %-8s %-14s %-14s %-11s %-8s %-11s %-8s %5d/%5d %8.2f  %s\n",
           f->id, f->flight_number,
           f->origin, f->destination,
           f->departure_date, f->departure_time,
           f->arrival_date,   f->arrival_time,
           f->total_seats - f->available_seats, f->total_seats,
           f->base_price,
           flight_status_str(f->status));
}

static void print_flight_header(void)
{
    printf("  %-5s %-8s %-14s %-14s %-11s %-8s %-11s %-8s %-11s %-10s %s\n",
           "ID", "Flight", "Origin", "Destination",
           "Dep.Date", "Dep.Time", "Arr.Date", "Arr.Time",
           "Booked/Cap", "Price($)", "Status");
    print_separator();
}

/* ------------------------------------------------------------------ */
/* Add flight                                                           */
/* ------------------------------------------------------------------ */

void add_flight(Flight flights[], int *count,
                Aircraft aircraft[], int aircraft_count)
{
    if (*count >= MAX_FLIGHTS) {
        printf("  [ERROR] Maximum flight limit reached.\n");
        return;
    }

    print_header("ADD NEW FLIGHT");

    Flight f;
    memset(&f, 0, sizeof(f));

    /* Determine next id. */
    int max_id = 0;
    for (int i = 0; i < *count; i++)
        if (flights[i].id > max_id) max_id = flights[i].id;
    f.id = max_id + 1;

    get_string_input("  Flight number    : ", f.flight_number, MAX_STRING);
    get_string_input("  Origin           : ", f.origin, MAX_STRING);
    get_string_input("  Destination      : ", f.destination, MAX_STRING);
    get_string_input("  Departure date   : ", f.departure_date, MAX_DATE);
    get_string_input("  Departure time   : ", f.departure_time, MAX_DATE);
    get_string_input("  Arrival date     : ", f.arrival_date, MAX_DATE);
    get_string_input("  Arrival time     : ", f.arrival_time, MAX_DATE);

    /* Optionally assign aircraft. */
    if (aircraft_count > 0) {
        printf("\n  Available aircraft:\n");
        for (int i = 0; i < aircraft_count; i++) {
            if (aircraft[i].status == AIRCRAFT_ACTIVE) {
                printf("    [%d] %s  %s %s  (cap %d)\n",
                       aircraft[i].id, aircraft[i].registration,
                       aircraft[i].manufacturer, aircraft[i].model,
                       aircraft[i].capacity);
            }
        }
        int aid = get_int_input("  Aircraft ID (0 = unassigned): ");
        Aircraft *ac = find_aircraft_by_id(aircraft, aircraft_count, aid);
        if (ac && ac->status == AIRCRAFT_ACTIVE) {
            f.aircraft_id   = ac->id;
            f.total_seats   = ac->capacity;
        } else if (aid != 0) {
            printf("  [WARN] Aircraft not found or not active - unassigned.\n");
        }
    }

    if (f.total_seats == 0) {
        f.total_seats = get_int_input("  Total seats      : ");
    }
    f.available_seats = f.total_seats;
    f.base_price  = get_double_input("  Base price ($)   : ");
    f.status      = FLIGHT_SCHEDULED;

    flights[*count] = f;
    (*count)++;
    save_flights(flights, *count);
    printf("  Flight %s (ID %d) added successfully.\n", f.flight_number, f.id);
}

/* ------------------------------------------------------------------ */
/* View all flights                                                     */
/* ------------------------------------------------------------------ */

void view_all_flights(const Flight flights[], int count)
{
    print_header("ALL FLIGHTS");
    if (count == 0) { printf("  No flights found.\n"); return; }
    print_flight_header();
    for (int i = 0; i < count; i++) print_flight_row(&flights[i]);
    printf("\n  Total: %d flight(s)\n", count);
}

/* ------------------------------------------------------------------ */
/* Search flights                                                       */
/* ------------------------------------------------------------------ */

void search_flights(const Flight flights[], int count)
{
    print_header("SEARCH FLIGHTS");
    printf("  1. Search by route\n");
    printf("  2. Search by date\n");
    printf("  3. Search by flight number\n");
    printf("  0. Back\n");
    print_separator();
    int opt = get_int_input("  Select option: ");

    int found = 0;
    print_flight_header();

    if (opt == 1) {
        char origin[MAX_STRING], dest[MAX_STRING];
        get_string_input("  Origin      : ", origin, MAX_STRING);
        get_string_input("  Destination : ", dest, MAX_STRING);
        for (int i = 0; i < count; i++) {
            if (strcasecmp(flights[i].origin, origin) == 0 &&
                strcasecmp(flights[i].destination, dest) == 0 &&
                flights[i].status != FLIGHT_CANCELLED) {
                print_flight_row(&flights[i]);
                found++;
            }
        }
    } else if (opt == 2) {
        char date[MAX_DATE];
        get_string_input("  Departure date (YYYY-MM-DD): ", date, MAX_DATE);
        for (int i = 0; i < count; i++) {
            if (strcmp(flights[i].departure_date, date) == 0 &&
                flights[i].status != FLIGHT_CANCELLED) {
                print_flight_row(&flights[i]);
                found++;
            }
        }
    } else if (opt == 3) {
        char fnum[MAX_STRING];
        get_string_input("  Flight number: ", fnum, MAX_STRING);
        for (int i = 0; i < count; i++) {
            if (strcasecmp(flights[i].flight_number, fnum) == 0) {
                print_flight_row(&flights[i]);
                found++;
            }
        }
    } else {
        return;
    }

    if (found == 0) printf("  No matching flights found.\n");
    else            printf("\n  Found %d flight(s).\n", found);
}

/* ------------------------------------------------------------------ */
/* Update flight details (admin)                                        */
/* ------------------------------------------------------------------ */

void update_flight(Flight flights[], int count)
{
    print_header("UPDATE FLIGHT");
    view_all_flights(flights, count);
    if (count == 0) return;

    int id = get_int_input("\n  Enter flight ID to update: ");
    Flight *f = find_flight_by_id(flights, count, id);
    if (!f) { printf("  [ERROR] Flight ID %d not found.\n", id); return; }

    printf("  Leave field blank to keep current value.\n\n");

    char buf[MAX_STRING];

    get_string_input("  Flight number  : ", buf, MAX_STRING);
    if (buf[0]) strncpy(f->flight_number, buf, MAX_STRING - 1);

    get_string_input("  Origin         : ", buf, MAX_STRING);
    if (buf[0]) strncpy(f->origin, buf, MAX_STRING - 1);

    get_string_input("  Destination    : ", buf, MAX_STRING);
    if (buf[0]) strncpy(f->destination, buf, MAX_STRING - 1);

    get_string_input("  Departure date : ", buf, MAX_DATE);
    if (buf[0]) strncpy(f->departure_date, buf, MAX_DATE - 1);

    get_string_input("  Departure time : ", buf, MAX_DATE);
    if (buf[0]) strncpy(f->departure_time, buf, MAX_DATE - 1);

    get_string_input("  Arrival date   : ", buf, MAX_DATE);
    if (buf[0]) strncpy(f->arrival_date, buf, MAX_DATE - 1);

    get_string_input("  Arrival time   : ", buf, MAX_DATE);
    if (buf[0]) strncpy(f->arrival_time, buf, MAX_DATE - 1);

    get_string_input("  Base price ($) : ", buf, MAX_STRING);
    if (buf[0]) f->base_price = atof(buf);

    save_flights(flights, count);
    printf("  Flight %d updated successfully.\n", id);
}

/* ------------------------------------------------------------------ */
/* Update flight status (admin)                                         */
/* ------------------------------------------------------------------ */

void update_flight_status(Flight flights[], int count)
{
    print_header("UPDATE FLIGHT STATUS");
    view_all_flights(flights, count);
    if (count == 0) return;

    int id = get_int_input("\n  Enter flight ID: ");
    Flight *f = find_flight_by_id(flights, count, id);
    if (!f) { printf("  [ERROR] Flight ID %d not found.\n", id); return; }

    printf("  Status options:\n");
    printf("    0 = Scheduled\n");
    printf("    1 = Delayed\n");
    printf("    2 = Cancelled\n");
    printf("    3 = Completed\n");
    int s = get_int_input("  New status: ");
    if (s < 0 || s > 3) { printf("  [ERROR] Invalid status.\n"); return; }

    f->status = s;
    save_flights(flights, count);
    printf("  Flight %d status set to '%s'.\n", id, flight_status_str(s));
}

/* ------------------------------------------------------------------ */
/* Delete flight (admin)                                                */
/* ------------------------------------------------------------------ */

void delete_flight(Flight flights[], int *count)
{
    print_header("DELETE FLIGHT");
    view_all_flights(flights, *count);
    if (*count == 0) return;

    int id = get_int_input("\n  Enter flight ID to delete: ");
    int idx = -1;
    for (int i = 0; i < *count; i++) {
        if (flights[i].id == id) { idx = i; break; }
    }
    if (idx < 0) { printf("  [ERROR] Flight ID %d not found.\n", id); return; }

    /* Shift array left. */
    for (int i = idx; i < *count - 1; i++) flights[i] = flights[i + 1];
    (*count)--;
    save_flights(flights, *count);
    printf("  Flight %d deleted.\n", id);
}

/* ------------------------------------------------------------------ */
/* Flight management menu (admin)                                       */
/* ------------------------------------------------------------------ */

void flight_management_menu(Flight flights[], int *count,
                             Aircraft aircraft[], int aircraft_count)
{
    int choice;
    do {
        print_header("FLIGHT MANAGEMENT");
        printf("  1. Add Flight\n");
        printf("  2. View All Flights\n");
        printf("  3. Search Flights\n");
        printf("  4. Update Flight Details\n");
        printf("  5. Update Flight Status\n");
        printf("  6. Delete Flight\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: add_flight(flights, count, aircraft, aircraft_count);  break;
        case 2: view_all_flights(flights, *count);
                printf("\nPress Enter to continue..."); getchar();       break;
        case 3: search_flights(flights, *count);
                printf("\nPress Enter to continue..."); getchar();       break;
        case 4: update_flight(flights, *count);                         break;
        case 5: update_flight_status(flights, *count);                  break;
        case 6: delete_flight(flights, count);                          break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
