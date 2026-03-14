#ifndef BOOKINGS_H
#define BOOKINGS_H

#include "types.h"

int  load_bookings(Booking bookings[], int *count);
int  save_bookings(const Booking bookings[], int count);

void book_flight(Booking bookings[], int *count,
                 Flight flights[], int flight_count,
                 const User *current_user);

void view_my_bookings(const Booking bookings[], int booking_count,
                      const Flight flights[], int flight_count,
                      int user_id);

void cancel_booking_by_user(Booking bookings[], int booking_count,
                             Flight flights[], int flight_count,
                             int user_id);

void view_all_bookings(const Booking bookings[], int booking_count,
                       const Flight flights[], int flight_count);

void search_booking(const Booking bookings[], int booking_count,
                    const Flight flights[], int flight_count);

void admin_cancel_booking(Booking bookings[], int booking_count,
                          Flight flights[], int flight_count);

void admin_booking_menu(Booking bookings[], int *count,
                        Flight flights[], int *flight_count);

void passenger_booking_menu(Booking bookings[], int *count,
                             Flight flights[], int *flight_count,
                             const User *current_user);

#endif /* BOOKINGS_H */
