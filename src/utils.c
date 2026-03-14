#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"
#include "types.h"

void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_separator(void)
{
    printf("================================================================\n");
}

void print_header(const char *title)
{
    print_separator();
    printf("  %s\n", title);
    print_separator();
}

/* Safe string input: reads a line, strips trailing newline. */
void get_string_input(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    /* Strip trailing newline / carriage-return. */
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }
}

int get_int_input(const char *prompt)
{
    char buf[32];
    get_string_input(prompt, buf, (int)sizeof(buf));
    return atoi(buf);
}

double get_double_input(const char *prompt)
{
    char buf[32];
    get_string_input(prompt, buf, (int)sizeof(buf));
    return atof(buf);
}

/* FNV-1a 64-bit hash — better avalanche than djb2.
 * NOTE: This is not a cryptographic hash. For production systems use
 * bcrypt, scrypt or Argon2. For this demo we combine a per-user salt
 * (username) with the password to resist basic rainbow-table attacks. */
static unsigned long long fnv1a_64(const unsigned char *data, size_t len)
{
    unsigned long long hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (unsigned long long)data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* Store salted hash as 16-char hex string.
 * The salt parameter should be the stored username. */
void hash_password(const char *password, const char *salt, char *hash_out)
{
    /* Concatenate salt + ':' + password into a single buffer before hashing. */
    char buf[MAX_STRING * 2];
    int n = snprintf(buf, sizeof(buf), "%s:%s", salt, password);
    if (n < 0) n = 0;
    unsigned long long h = fnv1a_64((unsigned char *)buf, (size_t)n);
    snprintf(hash_out, MAX_STRING, "%016llx", h);
}

int verify_password(const char *password, const char *salt,
                    const char *stored_hash)
{
    char computed[MAX_STRING];
    hash_password(password, salt, computed);
    return strcmp(computed, stored_hash) == 0;
}

/* Create the data directory if it does not exist. */
void ensure_data_dir(void)
{
#ifdef _WIN32
    _mkdir(DATA_DIR);
#else
    mkdir(DATA_DIR, 0755);
#endif
}

/* Fill date_buf with today's date in "YYYY-MM-DD" format. */
void get_current_date(char *date_buf)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date_buf, MAX_DATE, "%Y-%m-%d", tm_info);
}

/* ------------------------------------------------------------------ */
/* Label helpers                                                        */
/* ------------------------------------------------------------------ */

const char *flight_status_str(int status)
{
    switch (status) {
        case FLIGHT_SCHEDULED: return "Scheduled";
        case FLIGHT_DELAYED:   return "Delayed";
        case FLIGHT_CANCELLED: return "Cancelled";
        case FLIGHT_COMPLETED: return "Completed";
        default:               return "Unknown";
    }
}

const char *aircraft_status_str(int status)
{
    switch (status) {
        case AIRCRAFT_ACTIVE:      return "Active";
        case AIRCRAFT_MAINTENANCE: return "Maintenance";
        case AIRCRAFT_RETIRED:     return "Retired";
        default:                   return "Unknown";
    }
}

const char *seat_class_str(int cls)
{
    switch (cls) {
        case CLASS_ECONOMY:  return "Economy";
        case CLASS_BUSINESS: return "Business";
        case CLASS_FIRST:    return "First Class";
        default:             return "Unknown";
    }
}

const char *booking_status_str(int status)
{
    switch (status) {
        case BOOKING_CONFIRMED: return "Confirmed";
        case BOOKING_CANCELLED: return "Cancelled";
        default:                return "Unknown";
    }
}
