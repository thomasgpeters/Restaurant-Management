-- ============================================================================
-- Restaurant POS System - PostgreSQL Schema
-- ============================================================================
-- Use with ApiLogicServer:
--   1. Run this script to create the schema:  psql -d <dbname> -f schema.sql
--   2. Seed data with one of:                 psql -d <dbname> -f seed_thai.sql
--                                              psql -d <dbname> -f seed_chinese.sql
--                                              psql -d <dbname> -f seed_sandwich.sql
--   3. Point ApiLogicServer at the database:  als create --db-url=postgresql://...
-- ============================================================================

-- Restaurant
CREATE TABLE IF NOT EXISTS restaurant (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(200)  NOT NULL,
    cuisine_type    VARCHAR(100)  NOT NULL,
    description     TEXT          NOT NULL DEFAULT ''
);

-- Category (menu sections, ordered per restaurant)
CREATE TABLE IF NOT EXISTS category (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(200)  NOT NULL,
    sort_order      INTEGER       NOT NULL DEFAULT 0,
    restaurant_id   INTEGER       NOT NULL REFERENCES restaurant(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_category_restaurant ON category(restaurant_id, sort_order);

-- MenuItem
CREATE TABLE IF NOT EXISTS menu_item (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(200)  NOT NULL,
    description     TEXT          NOT NULL DEFAULT '',
    price           NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    available       BOOLEAN       NOT NULL DEFAULT TRUE,
    category_id     INTEGER       NOT NULL REFERENCES category(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_menu_item_category ON menu_item(category_id);

-- Orders (table name "orders" to avoid SQL reserved word)
CREATE TABLE IF NOT EXISTS orders (
    id              SERIAL PRIMARY KEY,
    table_number    INTEGER       NOT NULL DEFAULT 0,
    status          VARCHAR(50)   NOT NULL DEFAULT 'Pending',
    customer_name   VARCHAR(200)  NOT NULL DEFAULT '',
    notes           TEXT          NOT NULL DEFAULT '',
    created_at      TIMESTAMP     NOT NULL DEFAULT NOW(),
    updated_at      TIMESTAMP     NOT NULL DEFAULT NOW(),
    total           NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    restaurant_id   INTEGER       NOT NULL REFERENCES restaurant(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_orders_restaurant_status ON orders(restaurant_id, status);

-- OrderItem (line items on an order)
CREATE TABLE IF NOT EXISTS order_item (
    id                    SERIAL PRIMARY KEY,
    quantity              INTEGER       NOT NULL DEFAULT 1,
    unit_price            NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    special_instructions  TEXT          NOT NULL DEFAULT '',
    order_id              INTEGER       NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    menu_item_id          INTEGER       NOT NULL REFERENCES menu_item(id) ON DELETE RESTRICT
);
CREATE INDEX IF NOT EXISTS idx_order_item_order ON order_item(order_id);

-- Users (POS operators)
CREATE TABLE IF NOT EXISTS app_user (
    id              SERIAL PRIMARY KEY,
    username        VARCHAR(100)  NOT NULL UNIQUE,
    display_name    VARCHAR(200)  NOT NULL DEFAULT '',
    role            VARCHAR(50)   NOT NULL DEFAULT 'Front Desk',
    restaurant_id   INTEGER       NOT NULL REFERENCES restaurant(id) ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS idx_app_user_restaurant ON app_user(restaurant_id);

-- Site configuration (key-value store for POS settings)
CREATE TABLE IF NOT EXISTS site_config (
    id              SERIAL PRIMARY KEY,
    config_key      VARCHAR(100)  NOT NULL UNIQUE,
    config_value    TEXT          NOT NULL DEFAULT ''
);

-- Default site config entries
INSERT INTO site_config (config_key, config_value) VALUES
    ('store_name',    ''),
    ('store_logo',    ''),
    ('api_base_url',  'http://localhost:5656/api')
ON CONFLICT (config_key) DO NOTHING;
