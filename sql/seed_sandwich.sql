-- ============================================================================
-- Seed Data: The Crafted Bite (Sandwich Shop)
-- Requires: schema.sql must be run first
-- ============================================================================
--
-- Usage with psql:
--   psql -d <dbname> -f seed_sandwich.sql
--
-- Usage with pgAdmin:
--   Open Query Tool → paste this file → click Execute (F5)
--
-- Safe to re-run: uses TRUNCATE CASCADE to clear existing data first.
-- ============================================================================

BEGIN;

-- ── Clear any existing data (safe re-run) ───────────────────────────────
TRUNCATE TABLE order_item, orders, menu_item, category, app_user, restaurant RESTART IDENTITY CASCADE;

-- ── Restaurant ──────────────────────────────────────────────────────────
INSERT INTO restaurant (id, name, cuisine_type, description)
VALUES (1, 'The Crafted Bite', 'Sandwiches & More', 'Gourmet sandwiches, fresh salads, and espresso');

-- ── Categories ──────────────────────────────────────────────────────────
INSERT INTO category (id, name, sort_order, restaurant_id) VALUES
    (1, 'Sandwiches',        0, 1),
    (2, 'Salads',            1, 1),
    (3, 'Sides',             2, 1),
    (4, 'Drinks',            3, 1),
    (5, 'Espresso & Coffee', 4, 1);

-- ── Menu Items: Sandwiches ──────────────────────────────────────────────
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Classic Club',       'Turkey, bacon, lettuce, tomato on sourdough',                   10.99, TRUE, 1),
    ('Philly Cheesesteak', 'Shaved beef, peppers, onions, and provolone on hoagie',         12.99, TRUE, 1),
    ('Caprese Panini',     'Fresh mozzarella, tomato, basil, and balsamic on ciabatta',     10.49, TRUE, 1),
    ('Reuben',             'Corned beef, sauerkraut, Swiss, and Thousand Island on rye',    11.99, TRUE, 1),
    ('BBQ Pulled Pork',    'Slow-smoked pulled pork with coleslaw on brioche',              11.49, TRUE, 1),
    ('Veggie Wrap',        'Hummus, roasted vegetables, and feta in a spinach tortilla',     9.49, TRUE, 1),
    ('Turkey Avocado',     'Smoked turkey, avocado, sprouts, and aioli on wheat',           10.99, TRUE, 1),
    ('Grilled Chicken BLT','Grilled chicken breast with bacon, lettuce, tomato',            11.49, TRUE, 1);

-- ── Menu Items: Salads ──────────────────────────────────────────────────
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Caesar Salad',       'Romaine, parmesan, croutons, and Caesar dressing',               8.99, TRUE, 2),
    ('Greek Salad',        'Mixed greens, feta, olives, cucumber, tomato, red onion',        9.49, TRUE, 2),
    ('Cobb Salad',         'Chicken, bacon, egg, avocado, blue cheese, and tomato',         11.99, TRUE, 2),
    ('Asian Sesame Salad', 'Mixed greens, mandarin, almonds, crispy wontons, sesame',       10.49, TRUE, 2),
    ('Harvest Bowl',       'Quinoa, roasted sweet potato, kale, cranberries, goat cheese',  10.99, TRUE, 2);

-- ── Menu Items: Sides ───────────────────────────────────────────────────
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('French Fries',       'Crispy golden fries with sea salt',                              3.99, TRUE, 3),
    ('Onion Rings',        'Beer-battered onion rings',                                      4.49, TRUE, 3),
    ('Sweet Potato Fries', 'Crispy sweet potato fries with chipotle aioli',                  4.99, TRUE, 3),
    ('Cup of Soup',        'Daily rotating soup selection',                                  4.49, TRUE, 3);

-- ── Menu Items: Drinks ──────────────────────────────────────────────────
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Fresh Lemonade',    'House-made lemonade with real lemons',                            3.49, TRUE, 4),
    ('Iced Green Tea',    'Brewed green tea over ice',                                       2.99, TRUE, 4),
    ('Sparkling Water',   'San Pellegrino sparkling mineral water',                          2.49, TRUE, 4),
    ('Fresh OJ',          'Freshly squeezed orange juice',                                   4.49, TRUE, 4),
    ('Craft Soda',        'Rotating selection of artisan sodas',                             3.49, TRUE, 4);

-- ── Menu Items: Espresso & Coffee ───────────────────────────────────────
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Espresso',          'Double shot of locally roasted espresso',                         2.99, TRUE, 5),
    ('Americano',         'Espresso with hot water',                                         3.49, TRUE, 5),
    ('Cappuccino',        'Espresso with velvety steamed milk foam',                         4.49, TRUE, 5),
    ('Latte',             'Espresso with smooth steamed milk',                               4.49, TRUE, 5),
    ('Mocha',             'Espresso, chocolate, steamed milk, whipped cream',                4.99, TRUE, 5),
    ('Cold Brew',         'Slow-steeped cold brew coffee, served over ice',                  3.99, TRUE, 5),
    ('Flat White',        'Ristretto shots with micro-foam milk',                            4.49, TRUE, 5);

-- ── Default Users ───────────────────────────────────────────────────────
INSERT INTO app_user (username, display_name, role, restaurant_id) VALUES
    ('the_manager',   'The Crafted Bite Manager',    'Manager',    1),
    ('the_frontdesk', 'The Crafted Bite Front Desk', 'Front Desk', 1),
    ('the_kitchen',   'The Crafted Bite Kitchen',    'Kitchen',    1);

-- ── Sample Orders ───────────────────────────────────────────────────────
INSERT INTO orders (table_number, status, customer_name, notes, created_at, updated_at, total, restaurant_id) VALUES
    (1, 'Pending',     'Walk-In Guest', '',         NOW(), NOW(), 34.97, 1),
    (3, 'In Progress', 'Table 3',       'No spicy', NOW(), NOW(), 0.00,  1);

-- Order items for first order (1x Classic Club + 2x Philly Cheesesteak)
INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 1, mi.price, '', o.id, mi.id
FROM orders o
INNER JOIN menu_item mi ON mi.name = 'Classic Club' AND mi.category_id = 1
WHERE o.table_number = 1 AND o.restaurant_id = 1
LIMIT 1;

INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 2, mi.price, '', o.id, mi.id
FROM orders o
INNER JOIN menu_item mi ON mi.name = 'Philly Cheesesteak' AND mi.category_id = 1
WHERE o.table_number = 1 AND o.restaurant_id = 1
LIMIT 1;

-- ── Sync sequences after explicit-id inserts ────────────────────────────
-- Uses pg_get_serial_sequence() for portability across PostgreSQL versions
SELECT setval(pg_get_serial_sequence('restaurant', 'id'), COALESCE((SELECT MAX(id) FROM restaurant), 1));
SELECT setval(pg_get_serial_sequence('category',   'id'), COALESCE((SELECT MAX(id) FROM category),   1));
SELECT setval(pg_get_serial_sequence('menu_item',  'id'), COALESCE((SELECT MAX(id) FROM menu_item),  1));
SELECT setval(pg_get_serial_sequence('orders',     'id'), COALESCE((SELECT MAX(id) FROM orders),     1));
SELECT setval(pg_get_serial_sequence('order_item', 'id'), COALESCE((SELECT MAX(id) FROM order_item), 1));
SELECT setval(pg_get_serial_sequence('app_user',   'id'), COALESCE((SELECT MAX(id) FROM app_user),   1));

COMMIT;
