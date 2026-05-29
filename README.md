# 🏨 Esplenin Hotel — Reservation & Payment System

A console-based hotel management system written in C for a school project.

---

## ✨ Features

- 🛏️ **Room Availability** — View all vacant rooms by category and price
- 📋 **Reservations** — Book a room with check-in/check-out dates, guest info, and room type selection
- 🔍 **Booking Lookup** — Search existing reservations by guest name
- 🧖 **Amenities** — Add convenience, pool, or spa amenities to a booking
- 💳 **Payment Processing** — Supports Cash, Card, and GCash with receipt generation
- 📒 **Registry** — View all bookings on record
- 🚪 **Checkout** — Check out guests and free up rooms
- ℹ️ **Inquiry** — Browse room rates, amenity prices, and availability

---

## 🏷️ Room Types

| Type | Description |
|------|-------------|
| Classic | Standard rooms |
| De Luxe | Upgraded comfort |
| Suite | Premium rooms |
| Imperial Grand | Top-tier rooms |

---

## ▶️ How to Run

1. Compile with GCC:
```
   gcc hotel.c -o hotel
```
2. Run the executable:
```
   ./hotel        # Linux/Mac
   hotel.exe      # Windows
```
3. Make sure `rooms.txt`, `bookings.txt`, and the `Amenities/` folder are in the same directory.

---

## 📁 Files

| File | Purpose |
|------|---------|
| `hotel.c` | Main source code |
| `rooms.txt` | Room data (availability, type, price) |
| `bookings.txt` | Persistent booking records |
| `Amenities/` | Amenity lists by category |
| `Receipts/` | Auto-generated payment receipts |

---

## 🔧 Built With

- C (standard libraries: stdio, stdlib, string, time)
- File I/O for persistent data storage
