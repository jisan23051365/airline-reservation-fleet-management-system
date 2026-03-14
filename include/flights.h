#ifndef FLIGHTS_H
#define FLIGHTS_H

#include "types.h"

int  load_flights(Flight flights[], int *count);
int  save_flights(const Flight flights[], int count);

void add_flight(Flight flights[], int *count,
                Aircraft aircraft[], int aircraft_count);
void view_all_flights(const Flight flights[], int count);
void search_flights(const Flight flights[], int count);
void update_flight(Flight flights[], int count);
void update_flight_status(Flight flights[], int count);
void delete_flight(Flight flights[], int *count);

Flight *find_flight_by_id(Flight flights[], int count, int id);

void flight_management_menu(Flight flights[], int *count,
                             Aircraft aircraft[], int aircraft_count);

#endif /* FLIGHTS_H */
