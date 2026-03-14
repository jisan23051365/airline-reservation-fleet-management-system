#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "types.h"

void show_total_revenue(const Booking bookings[], int booking_count);

void show_revenue_by_flight(const Booking bookings[], int booking_count,
                             const Flight flights[], int flight_count);

void show_occupancy_rates(const Booking bookings[], int booking_count,
                           const Flight flights[], int flight_count);

void show_popular_routes(const Booking bookings[], int booking_count,
                          const Flight flights[], int flight_count);

void show_booking_class_breakdown(const Booking bookings[], int booking_count);

void analytics_menu(const Booking bookings[], int booking_count,
                     const Flight flights[], int flight_count);

#endif /* ANALYTICS_H */
