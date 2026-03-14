# Airline Reservation & Fleet Management System

## Overview
The Airline Reservation & Fleet Management System is a console-based application written in C that simulates key airline operations such as flight management, passenger booking, aircraft tracking, and revenue analytics.

The system includes authentication, activity logging, and file-based data storage to maintain airline records and booking information.

## Features

### Authentication System
- Admin login with username and password
- Password hashing for security
- Automatic admin account initialization

### Flight Management
- Add new flights
- View available flights
- Track seat availability and ticket pricing

### Passenger Booking
- Book seats on available flights
- Automatically update available seats
- Store booking records with passenger details

### Aircraft Management
- Add aircraft information
- Track aircraft capacity and maintenance costs

### Revenue Analytics
The system analyzes booking data and generates statistics including:
- Average fare
- Variance
- Standard deviation

### Activity Logging
All important system operations are recorded in a log file including:
- Admin login
- Flight creation
- Flight booking
- Analytics generation
- System exit

## Technologies Used
- C Programming Language
- Standard C Libraries
- File-based persistent storage

## System Data Files

| File | Purpose |
|-----|-----|
| flights.dat | Flight records |
| bookings.dat | Passenger booking records |
| aircrafts.dat | Aircraft fleet data |
| airline_users.dat | User authentication data |
| airline_logs.txt | System activity logs |

## Default Admin Login

When the program runs for the first time, a default admin account is created.

Username: admin  
Password: admin123

## Program Menu

1. Add Flight  
2. View Flights  
3. Book Flight  
4. Add Aircraft  
5. Revenue Analytics  
6. Exit

## Learning Objectives

This project demonstrates:

- File handling in C
- Data structures
- Authentication using hashing
- Logging systems
- Airline booking simulation
- Basic statistical analysis
- Console-based management systems

## Author
Jisan
