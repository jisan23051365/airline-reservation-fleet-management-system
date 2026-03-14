#ifndef TYPES_H
#define TYPES_H

/* String / array limits */
#define MAX_STRING   64
#define MAX_DATE     20
#define MAX_USERS    200
#define MAX_FLIGHTS  500
#define MAX_AIRCRAFT 100
#define MAX_BOOKINGS 2000

/* User roles */
#define ROLE_ADMIN     1
#define ROLE_PASSENGER 2

/* Flight status codes */
#define FLIGHT_SCHEDULED 0
#define FLIGHT_DELAYED   1
#define FLIGHT_CANCELLED 2
#define FLIGHT_COMPLETED 3

/* Aircraft status codes */
#define AIRCRAFT_ACTIVE      0
#define AIRCRAFT_MAINTENANCE 1
#define AIRCRAFT_RETIRED     2

/* Seat class codes */
#define CLASS_ECONOMY  0
#define CLASS_BUSINESS 1
#define CLASS_FIRST    2

/* Booking status codes */
#define BOOKING_CONFIRMED 0
#define BOOKING_CANCELLED 1

/* Persistent storage file paths */
#define DATA_DIR      "data"
#define USERS_FILE    "data/users.dat"
#define FLIGHTS_FILE  "data/flights.dat"
#define AIRCRAFT_FILE "data/aircraft.dat"
#define BOOKINGS_FILE "data/bookings.dat"

/* ------------------------------------------------------------------ */
/* Data structures                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    int  id;
    char username[MAX_STRING];
    char password_hash[MAX_STRING]; /* djb2 stored as hex string */
    char full_name[MAX_STRING];
    char email[MAX_STRING];
    int  role;
    int  active;
} User;

typedef struct {
    int    id;
    char   flight_number[MAX_STRING];
    char   origin[MAX_STRING];
    char   destination[MAX_STRING];
    char   departure_date[MAX_DATE];
    char   departure_time[MAX_DATE];
    char   arrival_date[MAX_DATE];
    char   arrival_time[MAX_DATE];
    int    aircraft_id;       /* 0 = unassigned */
    int    total_seats;
    int    available_seats;
    double base_price;
    int    status;
} Flight;

typedef struct {
    int    id;
    char   registration[MAX_STRING];
    char   model[MAX_STRING];
    char   manufacturer[MAX_STRING];
    int    capacity;
    int    status;
    double total_flight_hours;
    char   last_maintenance[MAX_DATE];
} Aircraft;

typedef struct {
    int    id;
    int    flight_id;
    int    user_id;
    char   passenger_name[MAX_STRING];
    char   passport_number[MAX_STRING];
    char   seat_number[MAX_STRING];
    int    seat_class;
    double price;
    char   booking_date[MAX_DATE];
    int    status;
} Booking;

#endif /* TYPES_H */
