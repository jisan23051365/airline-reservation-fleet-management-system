#ifndef UTILS_H
#define UTILS_H

void clear_screen(void);
void print_separator(void);
void print_header(const char *title);

void get_string_input(const char *prompt, char *buf, int size);
int  get_int_input(const char *prompt);
double get_double_input(const char *prompt);

void hash_password(const char *password, const char *salt, char *hash_out);
int  verify_password(const char *password, const char *salt,
                     const char *stored_hash);

void ensure_data_dir(void);
void get_current_date(char *date_buf); /* fills "YYYY-MM-DD" */

const char *flight_status_str(int status);
const char *aircraft_status_str(int status);
const char *seat_class_str(int cls);
const char *booking_status_str(int status);

#endif /* UTILS_H */
