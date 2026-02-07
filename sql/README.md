# SQL Files -- PostgreSQL for ApiLogicServer

These SQL files set up a PostgreSQL database for use with [ApiLogicServer](https://apilogicserver.github.io/Docs/), which auto-generates a JSON:API REST backend from the schema. The POS application connects to this API in **Enterprise (ALS) mode**.

## Files

| File | Description |
|------|-------------|
| `schema.sql` | Creates all tables, indexes, and default config rows |
| `seed_thai.sql` | Seed data for **Siam Garden** (Thai restaurant) |
| `seed_chinese.sql` | Seed data for **Golden Dragon** (Chinese restaurant) |
| `seed_sandwich.sql` | Seed data for **The Crafted Bite** (sandwich shop) |

## Schema

```
restaurant
├── category          (menu sections, sorted per restaurant)
│   └── menu_item     (dishes with price and availability)
├── orders            (customer orders with status tracking)
│   └── order_item    (line items referencing menu_item)
├── app_user          (POS operators: Manager, Front Desk, Kitchen)
└── site_config       (key-value store for POS settings)
```

## Quick Start

### 1. Create the database

```bash
createdb restaurant_pos
```

### 2. Run the schema

```bash
psql -d restaurant_pos -f schema.sql
```

### 3. Seed a restaurant

Pick one seed file to populate the database:

```bash
psql -d restaurant_pos -f seed_thai.sql       # Siam Garden
psql -d restaurant_pos -f seed_chinese.sql    # Golden Dragon
psql -d restaurant_pos -f seed_sandwich.sql   # The Crafted Bite
```

Each seed file is safe to re-run -- it uses `TRUNCATE CASCADE` to clear existing data first.

### 4. Generate the ApiLogicServer project

```bash
als create --project-name=restaurant_pos \
    --db-url=postgresql://user:password@localhost/restaurant_pos
```

### 5. Start ApiLogicServer

```bash
cd restaurant_pos
python api_logic_server_run.py
```

The API will be available at `http://localhost:5656/api` by default.

### 6. Point the POS app at the API

```bash
DATA_SOURCE_TYPE=ALS ./restaurant_pos --docroot . --http-listen 0.0.0.0:8080
```

Or set the values in `data/site-config.json`:

```json
{
  "data_source_type": "ALS",
  "api_base_url": "http://localhost:5656/api"
}
```

## pgAdmin Usage

All files are compatible with pgAdmin's Query Tool:

1. Open pgAdmin and connect to your server
2. Select your database
3. Open **Tools > Query Tool**
4. Paste the contents of `schema.sql` and click **Execute (F5)**
5. Paste a seed file and execute

## Tables

| Table | Purpose | Key Columns |
|-------|---------|-------------|
| `restaurant` | Restaurant profiles | `name`, `cuisine_type`, `description` |
| `category` | Menu sections per restaurant | `name`, `sort_order`, FK `restaurant_id` |
| `menu_item` | Individual dishes | `name`, `price`, `available`, FK `category_id` |
| `orders` | Customer orders | `table_number`, `status`, `total`, FK `restaurant_id` |
| `order_item` | Line items on an order | `quantity`, `unit_price`, FK `order_id`, FK `menu_item_id` |
| `app_user` | POS operator accounts | `username`, `role`, FK `restaurant_id` |
| `site_config` | Key-value POS settings | `config_key`, `config_value` |

## Seed Data Summary

Each seed file creates one restaurant with:
- 5 menu categories
- 15-20 menu items with prices
- 3 users (one per role: Manager, Front Desk, Kitchen)
- 3 sample orders in various statuses (Pending, In Progress, Ready)

| Seed File | Restaurant | Cuisine | Example Items |
|-----------|-----------|---------|---------------|
| `seed_thai.sql` | Siam Garden | Thai | Pad Thai, Green Curry, Tom Yum |
| `seed_chinese.sql` | Golden Dragon | Chinese | Kung Pao Chicken, Dim Sum, Fried Rice |
| `seed_sandwich.sql` | The Crafted Bite | American | Club Sandwich, Caesar Salad, Fries |
