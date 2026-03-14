#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aircraft.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* File I/O                                                             */
/* ------------------------------------------------------------------ */

int load_aircraft(Aircraft aircraft[], int *count)
{
    FILE *fp = fopen(AIRCRAFT_FILE, "rb");
    if (!fp) { *count = 0; return 0; }
    *count = (int)fread(aircraft, sizeof(Aircraft), MAX_AIRCRAFT, fp);
    fclose(fp);
    return 1;
}

int save_aircraft(const Aircraft aircraft[], int count)
{
    ensure_data_dir();
    FILE *fp = fopen(AIRCRAFT_FILE, "wb");
    if (!fp) { printf("  [ERROR] Cannot write aircraft file.\n"); return 0; }
    fwrite(aircraft, sizeof(Aircraft), (size_t)count, fp);
    fclose(fp);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Lookup                                                               */
/* ------------------------------------------------------------------ */

Aircraft *find_aircraft_by_id(Aircraft aircraft[], int count, int id)
{
    for (int i = 0; i < count; i++) {
        if (aircraft[i].id == id) return &aircraft[i];
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Display helpers                                                      */
/* ------------------------------------------------------------------ */

static void print_aircraft_header(void)
{
    printf("  %-5s %-12s %-20s %-20s %-8s %-14s %-12s %-12s\n",
           "ID", "Reg.", "Manufacturer", "Model",
           "Cap.", "Status", "FlightHours", "LastMaint.");
    print_separator();
}

static void print_aircraft_row(const Aircraft *a)
{
    printf("  %-5d %-12s %-20s %-20s %-8d %-14s %-12.1f %-12s\n",
           a->id, a->registration, a->manufacturer, a->model,
           a->capacity, aircraft_status_str(a->status),
           a->total_flight_hours, a->last_maintenance);
}

/* ------------------------------------------------------------------ */
/* Add aircraft                                                         */
/* ------------------------------------------------------------------ */

void add_aircraft(Aircraft aircraft[], int *count)
{
    if (*count >= MAX_AIRCRAFT) {
        printf("  [ERROR] Aircraft fleet limit reached.\n");
        return;
    }

    print_header("ADD AIRCRAFT");

    Aircraft a;
    memset(&a, 0, sizeof(a));

    int max_id = 0;
    for (int i = 0; i < *count; i++)
        if (aircraft[i].id > max_id) max_id = aircraft[i].id;
    a.id = max_id + 1;

    get_string_input("  Registration     : ", a.registration, MAX_STRING);
    get_string_input("  Manufacturer     : ", a.manufacturer, MAX_STRING);
    get_string_input("  Model            : ", a.model, MAX_STRING);
    a.capacity = get_int_input("  Passenger capacity: ");
    a.total_flight_hours = get_double_input("  Total flight hours: ");
    get_string_input("  Last maintenance  : ", a.last_maintenance, MAX_DATE);
    a.status = AIRCRAFT_ACTIVE;

    aircraft[*count] = a;
    (*count)++;
    save_aircraft(aircraft, *count);
    printf("  Aircraft %s (ID %d) added successfully.\n",
           a.registration, a.id);
}

/* ------------------------------------------------------------------ */
/* View all aircraft                                                    */
/* ------------------------------------------------------------------ */

void view_all_aircraft(const Aircraft aircraft[], int count)
{
    print_header("AIRCRAFT FLEET");
    if (count == 0) { printf("  No aircraft registered.\n"); return; }
    print_aircraft_header();
    for (int i = 0; i < count; i++) print_aircraft_row(&aircraft[i]);
    printf("\n  Total: %d aircraft\n", count);
}

/* ------------------------------------------------------------------ */
/* Update aircraft details                                              */
/* ------------------------------------------------------------------ */

void update_aircraft(Aircraft aircraft[], int count)
{
    print_header("UPDATE AIRCRAFT");
    view_all_aircraft(aircraft, count);
    if (count == 0) return;

    int id = get_int_input("\n  Enter aircraft ID to update: ");
    Aircraft *a = find_aircraft_by_id(aircraft, count, id);
    if (!a) { printf("  [ERROR] Aircraft ID %d not found.\n", id); return; }

    printf("  Leave blank to keep current value.\n\n");
    char buf[MAX_STRING];

    get_string_input("  Registration  : ", buf, MAX_STRING);
    if (buf[0]) strncpy(a->registration, buf, MAX_STRING - 1);

    get_string_input("  Manufacturer  : ", buf, MAX_STRING);
    if (buf[0]) strncpy(a->manufacturer, buf, MAX_STRING - 1);

    get_string_input("  Model         : ", buf, MAX_STRING);
    if (buf[0]) strncpy(a->model, buf, MAX_STRING - 1);

    get_string_input("  Capacity      : ", buf, MAX_STRING);
    if (buf[0]) a->capacity = atoi(buf);

    get_string_input("  Flight hours  : ", buf, MAX_STRING);
    if (buf[0]) a->total_flight_hours = atof(buf);

    get_string_input("  Last maint.   : ", buf, MAX_DATE);
    if (buf[0]) strncpy(a->last_maintenance, buf, MAX_DATE - 1);

    save_aircraft(aircraft, count);
    printf("  Aircraft %d updated.\n", id);
}

/* ------------------------------------------------------------------ */
/* Update aircraft status                                               */
/* ------------------------------------------------------------------ */

void update_aircraft_status(Aircraft aircraft[], int count)
{
    print_header("UPDATE AIRCRAFT STATUS");
    view_all_aircraft(aircraft, count);
    if (count == 0) return;

    int id = get_int_input("\n  Enter aircraft ID: ");
    Aircraft *a = find_aircraft_by_id(aircraft, count, id);
    if (!a) { printf("  [ERROR] Aircraft ID %d not found.\n", id); return; }

    printf("  Status options:\n");
    printf("    0 = Active\n");
    printf("    1 = Maintenance\n");
    printf("    2 = Retired\n");
    int s = get_int_input("  New status: ");
    if (s < 0 || s > 2) { printf("  [ERROR] Invalid status.\n"); return; }

    a->status = s;
    if (s == AIRCRAFT_MAINTENANCE) {
        get_current_date(a->last_maintenance);
        printf("  Last maintenance date set to %s.\n", a->last_maintenance);
    }
    save_aircraft(aircraft, count);
    printf("  Aircraft %d status set to '%s'.\n", id, aircraft_status_str(s));
}

/* ------------------------------------------------------------------ */
/* Aircraft management menu (admin)                                     */
/* ------------------------------------------------------------------ */

void aircraft_management_menu(Aircraft aircraft[], int *count)
{
    int choice;
    do {
        print_header("AIRCRAFT MANAGEMENT");
        printf("  1. Add Aircraft\n");
        printf("  2. View Fleet\n");
        printf("  3. Update Aircraft Details\n");
        printf("  4. Update Aircraft Status\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: add_aircraft(aircraft, count);                     break;
        case 2: view_all_aircraft(aircraft, *count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 3: update_aircraft(aircraft, *count);                 break;
        case 4: update_aircraft_status(aircraft, *count);          break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
