CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -I./include
TARGET  = airline_system

SRCS    = src/main.c \
          src/auth.c \
          src/flights.c \
          src/aircraft.c \
          src/bookings.c \
          src/analytics.c \
          src/utils.c

OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
