# Restaurant POS System

A full-featured Restaurant Point-of-Sale system built with **C++17** and the **Wt (Witty) Web Framework**. The application provides role-based views for restaurant management, order taking, and kitchen operations.

## Architecture

```
Restaurant-Management/
├── CMakeLists.txt              # Build configuration
├── wt_config.xml               # Wt server configuration
├── resources/
│   └── style.css               # Responsive CSS stylesheet
└── src/
    ├── main.cpp                # Entry point & server bootstrap
    ├── models/
    │   └── Models.h            # Wt::Dbo ORM models
    ├── services/
    │   ├── ApiService.h        # Service layer interface
    │   └── ApiService.cpp      # Service implementation (ApiLogicServer compatible)
    ├── ui/
    │   ├── RestaurantApp.h     # Main application & layout
    │   └── RestaurantApp.cpp
    └── widgets/
        ├── ManagerView.h/cpp   # Restaurant Manager dashboard
        ├── FrontDeskView.h/cpp # Front Desk order entry
        └── KitchenView.h/cpp   # Kitchen display system
```

## Features

### Three Role-Based Views

- **Restaurant Manager** -- Dashboard with order stats and revenue, full order list with cancel/serve actions, menu management with availability toggling
- **Front Desk** -- Menu browsing by category, cart-based ordering system with quantity controls, active order tracking with status badges
- **Kitchen Operations** -- Split view of Pending and In-Progress orders, accept and mark-ready workflow, auto-refresh every 10 seconds

### Design

- Sticky header spanning the full width
- 50/50 split workspace panels
- Fixed footer that stays at the bottom regardless of screen size
- Fully responsive layout (desktop, tablet, mobile breakpoints)

### Service Layer

The `ApiService` class provides a modular abstraction over the database, with methods that mirror REST endpoints. It is designed to be swappable with an **ApiLogicServer** middleware backend:

| Method                    | REST Equivalent                    |
|---------------------------|------------------------------------|
| `getRestaurants()`        | `GET /api/restaurants`             |
| `getMenuItemsByCategory()`| `GET /api/menu_items?category=id`  |
| `createOrder()`           | `POST /api/orders`                 |
| `updateOrderStatus()`     | `PATCH /api/orders/:id`            |
| `getActiveOrders()`       | `GET /api/orders?status=active`    |

### Seeded Data

Three restaurants with full menus:

1. **Siam Garden** (Thai) -- Appetizers, Curries, Stir Fry & Noodles, Drinks, Espresso & Coffee
2. **Golden Dragon** (Chinese) -- Appetizers, Main Courses, Noodles & Rice, Drinks, Espresso & Coffee
3. **The Crafted Bite** (Sandwiches & More) -- Sandwiches, Salads, Sides, Drinks, Espresso & Coffee

## Prerequisites

- C++17 compiler (GCC 10+ or Clang 12+)
- CMake 3.16+
- Wt 4.x (with Wt::Dbo and Wt::Http)
- SQLite3
- Boost (filesystem, thread)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Run

```bash
cd build
./restaurant_pos --docroot . --http-listen 0.0.0.0:8080
```

Then open `http://localhost:8080` in your browser. Select a restaurant and role to begin.

## Data Model

```
Restaurant 1──* Category 1──* MenuItem
Restaurant 1──* Order 1──* OrderItem *──1 MenuItem
Restaurant 1──* User (role: Manager | Front Desk | Kitchen)
```
