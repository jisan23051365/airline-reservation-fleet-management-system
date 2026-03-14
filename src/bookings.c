#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "bookings.h"
#include "flights.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* File I/O                                                             */
/* ------------------------------------------------------------------ */

int load_bookings(Booking bookings[], int *count)
{
    FILE *fp = fopen(BOOKINGS_FILE, "rb");
    if (!fp) { *count = 0; return 0; }
    *count = (int)fread(bookings, sizeof(Booking), MAX_BOOKINGS, fp);
    fclose(fp);
    return 1;
}

int save_bookings(const Booking bookings[], int count)
{
    ensure_data_dir();
    FILE *fp = fopen(BOOKINGS_FILE, "wb");
    if (!fp) { printf("  [ERROR] Cannot write bookings file.\n"); return 0; }
    fwrite(bookings, sizeof(Booking), (size_t)count, fp);
    fclose(fp);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Display helpers                                                      */
/* ------------------------------------------------------------------ */

static void print_booking_header(void)
{
    printf("  %-6s %-8s %-25s %-14s %-8s %-12s %-10s %-12s %s\n",
           "BkgID", "FlightID", "Passenger", "Passport",
           "Seat", "Class", "Price($)", "Date", "Status");
    print_separator();
}

static void print_booking_row(const Booking *b, const Flight *f)
{
    char fnum[MAX_STRING];
    if (f) snprintf(fnum, MAX_STRING, "%s", f->flight_number);
    else   snprintf(fnum, MAX_STRING, "ID:%d", b->flight_id);

    printf("  %-6d %-8s %-25s %-14s %-8s %-12s %-10.2f %-12s %s\n",
           b->id, fnum, b->passenger_name, b->passport_number,
           b->seat_number, seat_class_str(b->seat_class),
           b->price, b->booking_date, booking_status_str(b->status));
}

static const Flight *find_flight_const(const Flight flights[], int count, int id)
{
    for (int i = 0; i < count; i++)
        if (flights[i].id == id) return &flights[i];
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Generate a seat number (naive sequential per class)                  */
/* ------------------------------------------------------------------ */

static void assign_seat(const Booking bookings[], int booking_count,
                         int flight_id, int seat_class, char *seat_out)
{
    const char *prefix = (seat_class == CLASS_FIRST)    ? "F" :
                         (seat_class == CLASS_BUSINESS)  ? "B" : "E";
    int max_seat = 0;
    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].flight_id == flight_id &&
            bookings[i].seat_class == seat_class &&
            bookings[i].status == BOOKING_CONFIRMED) {
            /* Skip prefix character(s) and parse the numeric suffix.
             * The seat_number format is one alpha prefix + digits (e.g. "E12").
             * strtol with endptr validates that digits were actually present. */
            const char *num_start = bookings[i].seat_number;
            while (*num_start && (*num_start < '0' || *num_start > '9'))
                num_start++;
            if (*num_start != '\0') {
                char *end;
                long n = strtol(num_start, &end, 10);
                if (end != num_start && n > max_seat) max_seat = (int)n;
            }
        }
    }
    snprintf(seat_out, MAX_STRING, "%s%d", prefix, max_seat + 1);
}

/* ------------------------------------------------------------------ */
/* Book a flight                                                        */
/* ------------------------------------------------------------------ */

void book_flight(Booking bookings[], int *count,
                 Flight flights[], int flight_count,
                 const User *current_user)
{
    if (*count >= MAX_BOOKINGS) {
        printf("  [ERROR] Booking limit reached.\n");
        return;
    }

    print_header("BOOK A FLIGHT");
    search_flights(flights, flight_count);

    int flight_id = get_int_input("\n  Enter flight ID to book (0 = cancel): ");
    if (flight_id == 0) return;

    Flight *f = find_flight_by_id(flights, flight_count, flight_id);
    if (!f) { printf("  [ERROR] Flight ID %d not found.\n", flight_id); return; }
    if (f->status == FLIGHT_CANCELLED) {
        printf("  [ERROR] That flight is cancelled.\n");
        return;
    }
    if (f->available_seats <= 0) {
        printf("  [ERROR] No seats available on that flight.\n");
        return;
    }

    /* Seat class selection. */
    printf("\n  Seat class:\n");
    printf("    0 = Economy   ($%.2f)\n", f->base_price);
    printf("    1 = Business  ($%.2f)\n", f->base_price * 2.0);
    printf("    2 = First Class ($%.2f)\n", f->base_price * 3.5);
    int cls = get_int_input("  Choose class: ");
    if (cls < 0 || cls > 2) { printf("  [ERROR] Invalid class.\n"); return; }

    double price = f->base_price;
    if (cls == CLASS_BUSINESS) price *= 2.0;
    else if (cls == CLASS_FIRST) price *= 3.5;

    Booking b;
    memset(&b, 0, sizeof(b));

    int max_id = 0;
    for (int i = 0; i < *count; i++)
        if (bookings[i].id > max_id) max_id = bookings[i].id;
    b.id        = max_id + 1;
    b.flight_id = flight_id;
    b.user_id   = current_user->id;
    b.seat_class = cls;
    b.price     = price;
    b.status    = BOOKING_CONFIRMED;
    get_current_date(b.booking_date);

    get_string_input("  Passenger name     : ", b.passenger_name, MAX_STRING);
    get_string_input("  Passport number    : ", b.passport_number, MAX_STRING);
    assign_seat(bookings, *count, flight_id, cls, b.seat_number);

    printf("  Assigned seat: %s\n", b.seat_number);

    /* Decrease available seats. */
    f->available_seats--;

    bookings[*count] = b;
    (*count)++;

    save_bookings(bookings, *count);
    save_flights(flights, flight_count);

    printf("\n  *** Booking confirmed! ***\n");
    printf("  Booking ID : %d\n", b.id);
    printf("  Flight     : %s  %s -> %s\n",
           f->flight_number, f->origin, f->destination);
    printf("  Date       : %s %s\n", f->departure_date, f->departure_time);
    printf("  Seat       : %s  (%s)\n", b.seat_number, seat_class_str(cls));
    printf("  Price      : $%.2f\n", price);
}

/* ------------------------------------------------------------------ */
/* View bookings for current passenger                                  */
/* ------------------------------------------------------------------ */

void view_my_bookings(const Booking bookings[], int booking_count,
                      const Flight flights[], int flight_count,
                      int user_id)
{
    print_header("MY BOOKINGS");
    int found = 0;
    print_booking_header();
    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].user_id == user_id) {
            const Flight *f = find_flight_const(flights, flight_count,
                                                bookings[i].flight_id);
            print_booking_row(&bookings[i], f);
            found++;
        }
    }
    if (found == 0) printf("  No bookings found.\n");
    else            printf("\n  Total: %d booking(s)\n", found);
}

/* ------------------------------------------------------------------ */
/* Cancel booking (passenger)                                           */
/* ------------------------------------------------------------------ */

void cancel_booking_by_user(Booking bookings[], int booking_count,
                              Flight flights[], int flight_count,
                              int user_id)
{
    print_header("CANCEL BOOKING");
    view_my_bookings(bookings, booking_count, flights, flight_count, user_id);

    int bid = get_int_input("\n  Enter booking ID to cancel (0 = back): ");
    if (bid == 0) return;

    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].id == bid && bookings[i].user_id == user_id) {
            if (bookings[i].status == BOOKING_CANCELLED) {
                printf("  [ERROR] Booking already cancelled.\n");
                return;
            }
            bookings[i].status = BOOKING_CANCELLED;
            /* Restore the available seat. */
            Flight *f = find_flight_by_id(flights, flight_count,
                                          bookings[i].flight_id);
            if (f) f->available_seats++;
            save_bookings(bookings, booking_count);
            save_flights(flights, flight_count);
            printf("  Booking %d cancelled successfully.\n", bid);
            return;
        }
    }
    printf("  [ERROR] Booking ID %d not found or does not belong to you.\n", bid);
}

/* ------------------------------------------------------------------ */
/* Admin: view all bookings                                             */
/* ------------------------------------------------------------------ */

void view_all_bookings(const Booking bookings[], int booking_count,
                       const Flight flights[], int flight_count)
{
    print_header("ALL BOOKINGS");
    if (booking_count == 0) { printf("  No bookings yet.\n"); return; }
    print_booking_header();
    for (int i = 0; i < booking_count; i++) {
        const Flight *f = find_flight_const(flights, flight_count,
                                            bookings[i].flight_id);
        print_booking_row(&bookings[i], f);
    }
    printf("\n  Total: %d booking(s)\n", booking_count);
}

/* ------------------------------------------------------------------ */
/* Admin: search booking                                                */
/* ------------------------------------------------------------------ */

void search_booking(const Booking bookings[], int booking_count,
                    const Flight flights[], int flight_count)
{
    print_header("SEARCH BOOKING");
    printf("  1. By booking ID\n");
    printf("  2. By passport number\n");
    printf("  3. By passenger name\n");
    print_separator();
    int opt = get_int_input("  Select option: ");

    int found = 0;
    print_booking_header();

    if (opt == 1) {
        int bid = get_int_input("  Booking ID: ");
        for (int i = 0; i < booking_count; i++) {
            if (bookings[i].id == bid) {
                const Flight *f = find_flight_const(flights, flight_count,
                                                    bookings[i].flight_id);
                print_booking_row(&bookings[i], f);
                found++;
            }
        }
    } else if (opt == 2) {
        char passport[MAX_STRING];
        get_string_input("  Passport number: ", passport, MAX_STRING);
        for (int i = 0; i < booking_count; i++) {
            if (strcmp(bookings[i].passport_number, passport) == 0) {
                const Flight *f = find_flight_const(flights, flight_count,
                                                    bookings[i].flight_id);
                print_booking_row(&bookings[i], f);
                found++;
            }
        }
    } else if (opt == 3) {
        char name[MAX_STRING];
        get_string_input("  Passenger name: ", name, MAX_STRING);
        for (int i = 0; i < booking_count; i++) {
            if (strcasecmp(bookings[i].passenger_name, name) == 0) {
                const Flight *f = find_flight_const(flights, flight_count,
                                                    bookings[i].flight_id);
                print_booking_row(&bookings[i], f);
                found++;
            }
        }
    } else {
        return;
    }

    if (found == 0) printf("  No bookings found.\n");
    else            printf("\n  Found %d booking(s).\n", found);
}

/* ------------------------------------------------------------------ */
/* Admin: cancel any booking                                            */
/* ------------------------------------------------------------------ */

void admin_cancel_booking(Booking bookings[], int booking_count,
                           Flight flights[], int flight_count)
{
    print_header("CANCEL BOOKING (ADMIN)");
    view_all_bookings(bookings, booking_count, flights, flight_count);
    if (booking_count == 0) return;

    int bid = get_int_input("\n  Enter booking ID to cancel (0 = back): ");
    if (bid == 0) return;

    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].id == bid) {
            if (bookings[i].status == BOOKING_CANCELLED) {
                printf("  [ERROR] Already cancelled.\n");
                return;
            }
            bookings[i].status = BOOKING_CANCELLED;
            Flight *f = find_flight_by_id(flights, flight_count,
                                          bookings[i].flight_id);
            if (f) f->available_seats++;
            save_bookings(bookings, booking_count);
            save_flights(flights, flight_count);
            printf("  Booking %d cancelled.\n", bid);
            return;
        }
    }
    printf("  [ERROR] Booking ID %d not found.\n", bid);
}

/* ------------------------------------------------------------------ */
/* Admin booking menu                                                   */
/* ------------------------------------------------------------------ */

void admin_booking_menu(Booking bookings[], int *count,
                        Flight flights[], int *flight_count)
{
    int choice;
    do {
        print_header("BOOKING MANAGEMENT");
        printf("  1. View All Bookings\n");
        printf("  2. Search Booking\n");
        printf("  3. Cancel Booking\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: view_all_bookings(bookings, *count, flights, *flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 2: search_booking(bookings, *count, flights, *flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 3: admin_cancel_booking(bookings, *count, flights, *flight_count);
                break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}

/* ------------------------------------------------------------------ */
/* Passenger booking menu                                               */
/* ------------------------------------------------------------------ */

void passenger_booking_menu(Booking bookings[], int *count,
                             Flight flights[], int *flight_count,
                             const User *current_user)
{
    int choice;
    do {
        print_header("MY FLIGHTS & BOOKINGS");
        printf("  1. Search Available Flights\n");
        printf("  2. Book a Flight\n");
        printf("  3. View My Bookings\n");
        printf("  4. Cancel a Booking\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: search_flights(flights, *flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 2: book_flight(bookings, count, flights, *flight_count,
                            current_user);
                break;
        case 3: view_my_bookings(bookings, *count, flights, *flight_count,
                                 current_user->id);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 4: cancel_booking_by_user(bookings, *count, flights,
                                       *flight_count, current_user->id);
                break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
