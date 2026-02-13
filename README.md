# Restaurant POS System

A full-featured Restaurant Point-of-Sale system built with **C++17** and the **Wt (Witty) Web Framework**. Supports two operating modes: **Local** (embedded SQLite) for standalone use, or **Enterprise** (ApiLogicServer) for a shared PostgreSQL backend.

## Architecture

```
Restaurant-Management/
├── CMakeLists.txt                  # Build configuration (Wt, SQLite3, libcurl, Boost)
├── wt_config.xml                   # Wt server configuration
├── resources/
│   └── style.css                   # Responsive CSS with dark/light theme support
├── sql/
│   ├── schema.sql                  # PostgreSQL schema for ApiLogicServer
│   ├── seed_thai.sql               # Siam Garden seed data
│   ├── seed_chinese.sql            # Golden Dragon seed data
│   └── seed_sandwich.sql           # The Crafted Bite seed data
└── src/
    ├── main.cpp                    # Entry point, data source selection, server bootstrap
    ├── models/
    │   ├── Models.h                # Wt::Dbo ORM models (local mode)
    │   └── Dto.h                   # Plain C++ data transfer objects
    ├── services/
    │   ├── IApiService.h           # Abstract service interface (20 methods)
    │   ├── LocalApiService.h/cpp   # SQLite implementation via Wt::Dbo
    │   ├── RestApiService.h/cpp    # ApiLogicServer implementation via libcurl
    │   ├── ApiService.h/cpp        # Low-level Wt::Dbo session & queries
    │   └── SiteConfig.h/cpp        # JSON config persistence + env variable support
    ├── ui/
    │   ├── RestaurantApp.h/cpp     # Main app shell, routing, header, theme toggle
    └── widgets/
        ├── ManagerView.h/cpp       # Manager dashboard, orders, menu, settings
        ├── FrontDeskView.h/cpp     # Desktop order entry with split panels
        ├── MobileFrontDeskView.h/cpp # Mobile/tablet order entry
        └── KitchenView.h/cpp       # Kitchen display system
```

## Features

### Role-Based Views

- **Restaurant Manager** -- Dashboard with order stats and revenue, full order list with cancel/serve actions, menu management with availability toggling, site configuration settings
- **Front Desk** -- Menu browsing by category, cart-based ordering with quantity controls, active order tracking with status badges
- **Kitchen Operations** -- Split view of Pending and In-Progress orders, accept and mark-ready workflow, auto-refresh every 10 seconds

### Mobile & Tablet Support

- Automatic device detection (User-Agent + JavaScript touch/screen probing)
- **Phones**: sequential screen flow (categories -> items -> cart -> orders)
- **Tablets**: split-panel layout with side-by-side browsing and cart
- Header cart bubble and logout controls adapt to screen size

### Theme System

- Light and dark themes via CSS custom properties
- Dark theme uses Plex-inspired amber/gold accent palette
- Toggle in the header, persisted to localStorage
- `data-theme` attribute on `<html>` drives all overrides

### Service Abstraction Layer

All views program against `IApiService`, a pure virtual interface returning plain DTO structs. Two implementations are provided:

| Implementation | Backend | Use Case |
|----------------|---------|----------|
| `LocalApiService` | SQLite via Wt::Dbo | Standalone / development |
| `RestApiService` | ApiLogicServer via HTTP | Enterprise / shared database |

The REST implementation uses libcurl for synchronous HTTP and parses JSON:API responses (`application/vnd.api+json`) with relationship inclusion and sparse fieldsets.

### Data Source Configuration

The active data source is controlled by `data_source_type`:

| Method | Example | Priority |
|--------|---------|----------|
| **Environment variable** | `DATA_SOURCE_TYPE=ALS` | Highest |
| **Config file** | `"data_source_type": "ALS"` in `data/site-config.json` | Medium |
| **Manager Settings UI** | Data Source dropdown in Settings tab | Saves to config file |

Values: `LOCAL` (SQLite, default) or `ALS` (ApiLogicServer).

### Site Configuration

Managed through `data/site-config.json` and the Manager Settings tab:

```json
{
  "store_name": "Siam Garden",
  "store_logo": "resources/logo.png",
  "api_base_url": "http://localhost:5656/api",
  "data_source_type": "LOCAL"
}
```

Store name and logo appear in the header branding across all views.

### Seeded Data

Three restaurants with full menus (auto-seeded in Local mode):

1. **Siam Garden** (Thai) -- Appetizers, Curries, Stir Fry & Noodles, Drinks, Espresso & Coffee
2. **Golden Dragon** (Chinese) -- Appetizers, Main Courses, Noodles & Rice, Drinks, Espresso & Coffee
3. **The Crafted Bite** (Sandwiches & More) -- Sandwiches, Salads, Sides, Drinks, Espresso & Coffee

For Enterprise mode, see [`sql/README.md`](sql/README.md) for PostgreSQL setup.

## Prerequisites

- C++17 compiler (GCC 10+ or Clang 12+)
- CMake 3.16+
- Wt 4.x (with Wt::Dbo and Wt::Http)
- SQLite3
- libcurl (for ApiLogicServer mode)
- Boost (filesystem, thread)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Run

### Local Mode (default -- embedded SQLite)

```bash
cd build
./restaurant_pos --docroot . --http-listen 0.0.0.0:8080
```

### Enterprise Mode (ApiLogicServer)

```bash
cd build
DATA_SOURCE_TYPE=ALS ./restaurant_pos --docroot . --http-listen 0.0.0.0:8080
```

Or set `"data_source_type": "ALS"` in `data/site-config.json`.

Then open `http://localhost:8080` in your browser. Select a restaurant and role to begin.

## Data Model

```
Restaurant 1──* Category 1──* MenuItem
Restaurant 1──* Order 1──* OrderItem *──1 MenuItem
Restaurant 1──* User (role: Manager | Front Desk | Kitchen)
```

## ApiLogicServer Integration

To connect to a PostgreSQL database via ApiLogicServer:

1. Create the schema: `psql -d <dbname> -f sql/schema.sql`
2. Seed one restaurant: `psql -d <dbname> -f sql/seed_thai.sql`
3. Generate the API: `als create --project-name=restaurant_pos --db-url=postgresql://user:pw@host/dbname`
4. Start ApiLogicServer (default port 5656)
5. Start this app with `DATA_SOURCE_TYPE=ALS` and `api_base_url` pointing to the ALS instance
