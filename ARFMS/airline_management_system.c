#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* =========================================================
   AIRLINE RESERVATION & FLEET MANAGEMENT SYSTEM
   ========================================================= */

typedef struct {
    int id;
    char username[30];
    unsigned long password_hash;
} User;

typedef struct {
    int flight_id;
    char origin[50];
    char destination[50];
    int total_seats;
    int available_seats;
    float ticket_price;
} Flight;

typedef struct {
    int booking_id;
    int flight_id;
    char passenger_name[50];
    int seats_booked;
    float total_fare;
    char timestamp[30];
} Booking;

typedef struct {
    int aircraft_id;
    char model[50];
    int capacity;
    float maintenance_cost;
} Aircraft;

/* ===================== UTILITIES ===================== */

unsigned long hashPassword(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void logActivity(const char *msg) {
    FILE *fp = fopen("airline_logs.txt", "a");
    if (!fp) return;
    time_t now = time(NULL);
    fprintf(fp, "%s - %s\n", ctime(&now), msg);
    fprintf(fp, "--------------------------------\n");
    fclose(fp);
}

/* ===================== AUTH ===================== */

void initializeAdmin() {
    FILE *fp = fopen("airline_users.dat", "rb");
    if (fp) { fclose(fp); return; }

    fp = fopen("airline_users.dat", "wb");
    User admin = {1, "admin", hashPassword("admin123")};
    fwrite(&admin, sizeof(User), 1, fp);
    fclose(fp);
}

int login() {
    char username[30], password[30];
    unsigned long hash;
    User u;

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    hash = hashPassword(password);

    FILE *fp = fopen("airline_users.dat", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.username, username) == 0 && u.password_hash == hash) {
            fclose(fp);
            logActivity("Admin login successful.");
            return 1;
        }
    }

    fclose(fp);
    printf("Invalid credentials.\n");
    return 0;
}

/* ===================== FLIGHT ===================== */

void addFlight() {
    FILE *fp = fopen("flights.dat", "ab");
    if (!fp) return;

    Flight f;
    printf("Flight ID: "); scanf("%d", &f.flight_id);
    printf("Origin: "); scanf(" %[^\n]", f.origin);
    printf("Destination: "); scanf(" %[^\n]", f.destination);
    printf("Total Seats: "); scanf("%d", &f.total_seats);
    printf("Ticket Price: "); scanf("%f", &f.ticket_price);

    f.available_seats = f.total_seats;

    fwrite(&f, sizeof(Flight), 1, fp);
    fclose(fp);
    logActivity("Flight added.");
}

void viewFlights() {
    FILE *fp = fopen("flights.dat", "rb");
    if (!fp) return;

    Flight f;
    printf("\n--- Flights ---\n");
    while (fread(&f, sizeof(Flight), 1, fp)) {
        printf("ID:%d | %s -> %s | Seats:%d | Price:%.2f\n",
               f.flight_id, f.origin, f.destination,
               f.available_seats, f.ticket_price);
    }
    fclose(fp);
}

/* ===================== BOOKING ===================== */

void bookFlight() {
    FILE *ffp = fopen("flights.dat", "rb+");
    FILE *bfp = fopen("bookings.dat", "ab");
    if (!ffp || !bfp) return;

    int id, seats;
    Flight f;

    printf("Flight ID: "); scanf("%d", &id);
    printf("Passenger Name: ");
    char name[50];
    scanf(" %[^\n]", name);
    printf("Seats to Book: "); scanf("%d", &seats);

    while (fread(&f, sizeof(Flight), 1, ffp)) {
        if (f.flight_id == id) {
            if (f.available_seats >= seats) {
                f.available_seats -= seats;
                fseek(ffp, -sizeof(Flight), SEEK_CUR);
                fwrite(&f, sizeof(Flight), 1, ffp);

                Booking b;
                b.booking_id = rand() % 100000;
                b.flight_id = id;
                strcpy(b.passenger_name, name);
                b.seats_booked = seats;
                b.total_fare = seats * f.ticket_price;
                strcpy(b.timestamp, ctime(&(time_t){time(NULL)}));

                fwrite(&b, sizeof(Booking), 1, bfp);
                printf("Booking successful. Total Fare: %.2f\n", b.total_fare);
                logActivity("Flight booked.");
            } else {
                printf("Not enough seats available.\n");
            }
            break;
        }
    }

    fclose(ffp);
    fclose(bfp);
}

/* ===================== AIRCRAFT ===================== */

void addAircraft() {
    FILE *fp = fopen("aircrafts.dat", "ab");
    if (!fp) return;

    Aircraft a;
    printf("Aircraft ID: "); scanf("%d", &a.aircraft_id);
    printf("Model: "); scanf(" %[^\n]", a.model);
    printf("Capacity: "); scanf("%d", &a.capacity);
    printf("Maintenance Cost: "); scanf("%f", &a.maintenance_cost);

    fwrite(&a, sizeof(Aircraft), 1, fp);
    fclose(fp);
    logActivity("Aircraft added.");
}

/* ===================== ANALYTICS ===================== */

void revenueAnalytics() {
    FILE *fp = fopen("bookings.dat", "rb");
    if (!fp) return;

    Booking b;
    float arr[500];
    int n = 0;

    while (fread(&b, sizeof(Booking), 1, fp) && n < 500) {
        arr[n++] = b.total_fare;
    }
    fclose(fp);

    if (n == 0) {
        printf("No booking data.\n");
        return;
    }

    float sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    float mean = sum / n;

    float variance = 0;
    for (int i = 0; i < n; i++)
        variance += pow(arr[i] - mean, 2);
    variance /= n;

    float std = sqrt(variance);

    printf("\n--- Revenue Analytics ---\n");
    printf("Total Bookings: %d\n", n);
    printf("Average Fare: %.2f\n", mean);
    printf("Variance: %.2f\n", variance);
    printf("Standard Deviation: %.2f\n", std);

    logActivity("Revenue analytics generated.");
}

/* ===================== MAIN ===================== */

int main() {
    initializeAdmin();

    printf("=== AIRLINE MANAGEMENT SYSTEM ===\n");

    if (!login()) return 0;

    int choice;

    while (1) {
        printf("\n1.Add Flight\n");
        printf("2.View Flights\n");
        printf("3.Book Flight\n");
        printf("4.Add Aircraft\n");
        printf("5.Revenue Analytics\n");
        printf("6.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addFlight(); break;
            case 2: viewFlights(); break;
            case 3: bookFlight(); break;
            case 4: addAircraft(); break;
            case 5: revenueAnalytics(); break;
            case 6:
                logActivity("System exited.");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}