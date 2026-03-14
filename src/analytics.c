#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Total revenue                                                        */
/* ------------------------------------------------------------------ */

void show_total_revenue(const Booking bookings[], int booking_count)
{
    double total = 0.0;
    int confirmed = 0, cancelled = 0;

    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].status == BOOKING_CONFIRMED) {
            total += bookings[i].price;
            confirmed++;
        } else {
            cancelled++;
        }
    }

    print_header("TOTAL REVENUE SUMMARY");
    printf("  Total bookings    : %d\n", booking_count);
    printf("  Confirmed         : %d\n", confirmed);
    printf("  Cancelled         : %d\n", cancelled);
    printf("  Total Revenue     : $%.2f\n", total);
    if (confirmed > 0)
        printf("  Avg. Ticket Price : $%.2f\n", total / confirmed);
}

/* ------------------------------------------------------------------ */
/* Revenue by flight                                                    */
/* ------------------------------------------------------------------ */

void show_revenue_by_flight(const Booking bookings[], int booking_count,
                             const Flight flights[], int flight_count)
{
    print_header("REVENUE BY FLIGHT");

    if (flight_count == 0) { printf("  No flights.\n"); return; }

    printf("  %-5s %-8s %-14s %-14s %-10s %-10s %-12s\n",
           "ID", "Flight", "Origin", "Destination",
           "Bookings", "Revenue($)", "Occupancy");
    print_separator();

    for (int i = 0; i < flight_count; i++) {
        const Flight *f = &flights[i];
        double rev = 0.0;
        int cnt = 0;
        for (int j = 0; j < booking_count; j++) {
            if (bookings[j].flight_id == f->id &&
                bookings[j].status == BOOKING_CONFIRMED) {
                rev += bookings[j].price;
                cnt++;
            }
        }
        double occ = (f->total_seats > 0)
                     ? (100.0 * cnt / f->total_seats)
                     : 0.0;
        printf("  %-5d %-8s %-14s %-14s %-10d %-10.2f %.1f%%\n",
               f->id, f->flight_number, f->origin, f->destination,
               cnt, rev, occ);
    }
}

/* ------------------------------------------------------------------ */
/* Occupancy rates                                                      */
/* ------------------------------------------------------------------ */

void show_occupancy_rates(const Booking bookings[], int booking_count,
                           const Flight flights[], int flight_count)
{
    print_header("FLIGHT OCCUPANCY RATES");

    if (flight_count == 0) { printf("  No flights.\n"); return; }

    printf("  %-5s %-8s %-14s %-14s %-6s %-8s %-8s  %s\n",
           "ID", "Flight", "Origin", "Destination",
           "Cap.", "Booked", "Avail.", "Occupancy");
    print_separator();

    for (int i = 0; i < flight_count; i++) {
        const Flight *f = &flights[i];
        /* Count confirmed bookings directly from the bookings array
         * for accuracy (cancellations restore available_seats, but
         * using the authoritative source is more robust). */
        int booked = 0;
        for (int j = 0; j < booking_count; j++) {
            if (bookings[j].flight_id == f->id &&
                bookings[j].status == BOOKING_CONFIRMED) {
                booked++;
            }
        }
        int avail = f->total_seats - booked;
        double occ = (f->total_seats > 0)
                     ? (100.0 * booked / f->total_seats)
                     : 0.0;
        printf("  %-5d %-8s %-14s %-14s %-6d %-8d %-8d  %.1f%%\n",
               f->id, f->flight_number, f->origin, f->destination,
               f->total_seats, booked, avail, occ);
    }
}

/* ------------------------------------------------------------------ */
/* Popular routes                                                       */
/* ------------------------------------------------------------------ */

/* Simple pair-counting: O(n^2) over unique routes — fine for demo. */
#define MAX_ROUTES 256

typedef struct {
    char origin[MAX_STRING];
    char destination[MAX_STRING];
    int  booking_count;
    double revenue;
} RouteStats;

void show_popular_routes(const Booking bookings[], int booking_count,
                          const Flight flights[], int flight_count)
{
    print_header("POPULAR ROUTES");

    RouteStats routes[MAX_ROUTES];
    int route_count = 0;

    for (int j = 0; j < booking_count; j++) {
        if (bookings[j].status != BOOKING_CONFIRMED) continue;

        /* Find the flight for this booking. */
        const Flight *f = NULL;
        for (int k = 0; k < flight_count; k++) {
            if (flights[k].id == bookings[j].flight_id) {
                f = &flights[k];
                break;
            }
        }
        if (!f) continue;

        /* Check if route already tracked. */
        int found = 0;
        for (int r = 0; r < route_count; r++) {
            if (strcmp(routes[r].origin,      f->origin) == 0 &&
                strcmp(routes[r].destination, f->destination) == 0) {
                routes[r].booking_count++;
                routes[r].revenue += bookings[j].price;
                found = 1;
                break;
            }
        }
        if (!found && route_count < MAX_ROUTES) {
            strncpy(routes[route_count].origin,
                    f->origin, MAX_STRING - 1);
            strncpy(routes[route_count].destination,
                    f->destination, MAX_STRING - 1);
            routes[route_count].booking_count = 1;
            routes[route_count].revenue       = bookings[j].price;
            route_count++;
        }
    }

    if (route_count == 0) {
        printf("  No booking data available.\n");
        return;
    }

    /* Bubble-sort descending by booking_count. */
    for (int i = 0; i < route_count - 1; i++) {
        for (int k = 0; k < route_count - i - 1; k++) {
            if (routes[k].booking_count < routes[k + 1].booking_count) {
                RouteStats tmp = routes[k];
                routes[k]      = routes[k + 1];
                routes[k + 1]  = tmp;
            }
        }
    }

    printf("  %-20s %-20s %-12s %s\n",
           "Origin", "Destination", "Bookings", "Revenue($)");
    print_separator();
    for (int i = 0; i < route_count; i++) {
        printf("  %-20s %-20s %-12d %.2f\n",
               routes[i].origin, routes[i].destination,
               routes[i].booking_count, routes[i].revenue);
    }
}

/* ------------------------------------------------------------------ */
/* Seat-class breakdown                                                 */
/* ------------------------------------------------------------------ */

void show_booking_class_breakdown(const Booking bookings[], int booking_count)
{
    print_header("BOOKING CLASS BREAKDOWN");

    int eco_cnt = 0, bus_cnt = 0, fst_cnt = 0;
    double eco_rev = 0, bus_rev = 0, fst_rev = 0;

    for (int i = 0; i < booking_count; i++) {
        if (bookings[i].status != BOOKING_CONFIRMED) continue;
        switch (bookings[i].seat_class) {
            case CLASS_ECONOMY:  eco_cnt++; eco_rev += bookings[i].price; break;
            case CLASS_BUSINESS: bus_cnt++; bus_rev += bookings[i].price; break;
            case CLASS_FIRST:    fst_cnt++; fst_rev += bookings[i].price; break;
        }
    }

    int total = eco_cnt + bus_cnt + fst_cnt;
    printf("  %-14s %-10s %-12s %s\n",
           "Class", "Bookings", "Revenue($)", "Share");
    print_separator();
    printf("  %-14s %-10d %-12.2f %.1f%%\n",
           "Economy",    eco_cnt, eco_rev,
           total ? 100.0 * eco_cnt / total : 0.0);
    printf("  %-14s %-10d %-12.2f %.1f%%\n",
           "Business",   bus_cnt, bus_rev,
           total ? 100.0 * bus_cnt / total : 0.0);
    printf("  %-14s %-10d %-12.2f %.1f%%\n",
           "First Class", fst_cnt, fst_rev,
           total ? 100.0 * fst_cnt / total : 0.0);
}

/* ------------------------------------------------------------------ */
/* Analytics menu                                                       */
/* ------------------------------------------------------------------ */

void analytics_menu(const Booking bookings[], int booking_count,
                     const Flight flights[], int flight_count)
{
    int choice;
    do {
        print_header("ANALYTICS & REPORTS");
        printf("  1. Total Revenue Summary\n");
        printf("  2. Revenue by Flight\n");
        printf("  3. Flight Occupancy Rates\n");
        printf("  4. Popular Routes\n");
        printf("  5. Booking Class Breakdown\n");
        printf("  0. Back\n");
        print_separator();
        choice = get_int_input("  Select option: ");

        switch (choice) {
        case 1: show_total_revenue(bookings, booking_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 2: show_revenue_by_flight(bookings, booking_count,
                                       flights, flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 3: show_occupancy_rates(bookings, booking_count,
                                     flights, flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 4: show_popular_routes(bookings, booking_count,
                                    flights, flight_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 5: show_booking_class_breakdown(bookings, booking_count);
                printf("\nPress Enter to continue..."); getchar();  break;
        case 0: break;
        default: printf("  Invalid option.\n");
        }
    } while (choice != 0);
}
