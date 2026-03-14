#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "types.h"

int  load_aircraft(Aircraft aircraft[], int *count);
int  save_aircraft(const Aircraft aircraft[], int count);

void add_aircraft(Aircraft aircraft[], int *count);
void view_all_aircraft(const Aircraft aircraft[], int count);
void update_aircraft(Aircraft aircraft[], int count);
void update_aircraft_status(Aircraft aircraft[], int count);

Aircraft *find_aircraft_by_id(Aircraft aircraft[], int count, int id);

void aircraft_management_menu(Aircraft aircraft[], int *count);

#endif /* AIRCRAFT_H */
