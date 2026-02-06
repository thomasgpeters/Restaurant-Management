-- ============================================================================
-- Seed Data: Golden Dragon (Chinese Restaurant)
-- Run after schema.sql:  psql -d <dbname> -f seed_chinese.sql
-- ============================================================================

BEGIN;

-- Restaurant
INSERT INTO restaurant (id, name, cuisine_type, description)
VALUES (1, 'Golden Dragon', 'Chinese', 'Traditional Chinese dishes from multiple regions');

-- Categories
INSERT INTO category (id, name, sort_order, restaurant_id) VALUES
    (1, 'Appetizers',       0, 1),
    (2, 'Main Courses',     1, 1),
    (3, 'Noodles & Rice',   2, 1),
    (4, 'Drinks',           3, 1),
    (5, 'Espresso & Coffee',4, 1);

-- Menu Items: Appetizers
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Wonton Soup',       'Pork and shrimp wontons in clear broth',                    6.99, TRUE, 1),
    ('Egg Rolls',         'Crispy pork and vegetable egg rolls',                        5.99, TRUE, 1),
    ('Potstickers',       'Pan-fried pork dumplings with soy dipping sauce',           7.99, TRUE, 1),
    ('Hot & Sour Soup',   'Classic spicy and tangy soup with tofu and mushrooms',      6.49, TRUE, 1),
    ('Crab Rangoon',      'Fried wonton with cream cheese and crab filling',           7.49, TRUE, 1);

-- Menu Items: Main Courses
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Kung Pao Chicken',   'Spicy diced chicken with peanuts and dried chilies',      13.99, TRUE, 2),
    ('Sweet & Sour Pork',  'Crispy pork in tangy sweet and sour sauce',               12.99, TRUE, 2),
    ('Beef & Broccoli',    'Tender beef and broccoli in savory brown sauce',           14.49, TRUE, 2),
    ('Mapo Tofu',          'Soft tofu in spicy Sichuan chili bean sauce',              11.99, TRUE, 2),
    ('General Tso Chicken','Crispy chicken in a sweet and mildly spicy sauce',         13.49, TRUE, 2),
    ('Mongolian Beef',     'Sliced beef with scallions in sweet soy sauce',            14.99, TRUE, 2);

-- Menu Items: Noodles & Rice
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Lo Mein',            'Soft egg noodles with vegetables and choice of protein',   11.99, TRUE, 3),
    ('Chow Fun',           'Wide rice noodles stir-fried with beef and bean sprouts',  12.49, TRUE, 3),
    ('Yang Chow Fried Rice','Fried rice with shrimp, pork, and egg',                   10.99, TRUE, 3),
    ('Dan Dan Noodles',    'Spicy Sichuan noodles with ground pork and peanuts',       11.49, TRUE, 3);

-- Menu Items: Drinks
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Jasmine Tea',       'Fragrant hot jasmine green tea',                             2.49, TRUE, 4),
    ('Chinese Iced Tea',  'Chilled chrysanthemum tea',                                  2.99, TRUE, 4),
    ('Lychee Juice',      'Sweet lychee fruit juice',                                   3.49, TRUE, 4),
    ('Plum Juice',        'Traditional sour plum drink',                                3.49, TRUE, 4);

-- Menu Items: Espresso & Coffee
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Yuan Yang',         'Hong Kong-style coffee and tea blend',                        3.99, TRUE, 5),
    ('Espresso',          'Double shot of espresso',                                     2.99, TRUE, 5),
    ('Mocha',             'Espresso with chocolate and steamed milk',                    4.99, TRUE, 5),
    ('Iced Americano',    'Espresso with cold water over ice',                           3.49, TRUE, 5);

-- Default Users
INSERT INTO app_user (username, display_name, role, restaurant_id) VALUES
    ('gol_manager',   'Golden Dragon Manager',    'Manager',    1),
    ('gol_frontdesk', 'Golden Dragon Front Desk', 'Front Desk', 1),
    ('gol_kitchen',   'Golden Dragon Kitchen',    'Kitchen',    1);

-- Sample Orders
INSERT INTO orders (table_number, status, customer_name, notes, created_at, updated_at, total, restaurant_id) VALUES
    (1, 'Pending',     'Walk-In Guest', '',         NOW(), NOW(), 18.97, 1),
    (3, 'In Progress', 'Table 3',       'No spicy', NOW(), NOW(), 0.00,  1);

-- Order items for first order (1x Wonton Soup + 2x Egg Rolls)
INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 1, mi.price, '', o.id, mi.id
FROM orders o, menu_item mi
WHERE o.table_number = 1 AND o.restaurant_id = 1 AND mi.name = 'Wonton Soup' AND mi.category_id = 1
LIMIT 1;

INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 2, mi.price, '', o.id, mi.id
FROM orders o, menu_item mi
WHERE o.table_number = 1 AND o.restaurant_id = 1 AND mi.name = 'Egg Rolls' AND mi.category_id = 1
LIMIT 1;

-- Reset sequences to avoid conflicts
SELECT setval('restaurant_id_seq', (SELECT MAX(id) FROM restaurant));
SELECT setval('category_id_seq', (SELECT MAX(id) FROM category));
SELECT setval('menu_item_id_seq', (SELECT MAX(id) FROM menu_item));
SELECT setval('orders_id_seq', (SELECT MAX(id) FROM orders));
SELECT setval('order_item_id_seq', (SELECT MAX(id) FROM order_item));
SELECT setval('app_user_id_seq', (SELECT MAX(id) FROM app_user));

COMMIT;
