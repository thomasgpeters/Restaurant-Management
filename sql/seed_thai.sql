-- ============================================================================
-- Seed Data: Siam Garden (Thai Restaurant)
-- Run after schema.sql:  psql -d <dbname> -f seed_thai.sql
-- ============================================================================

BEGIN;

-- Restaurant
INSERT INTO restaurant (id, name, cuisine_type, description)
VALUES (1, 'Siam Garden', 'Thai', 'Authentic Thai cuisine with bold flavors');

-- Categories
INSERT INTO category (id, name, sort_order, restaurant_id) VALUES
    (1, 'Appetizers',         0, 1),
    (2, 'Curries',            1, 1),
    (3, 'Stir Fry & Noodles', 2, 1),
    (4, 'Drinks',             3, 1),
    (5, 'Espresso & Coffee',  4, 1);

-- Menu Items: Appetizers
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Spring Rolls',      'Crispy vegetable spring rolls with sweet chili sauce',      6.99, TRUE, 1),
    ('Satay Chicken',     'Grilled chicken skewers with peanut dipping sauce',         8.99, TRUE, 1),
    ('Tom Yum Soup',      'Spicy and sour soup with shrimp and mushrooms',             7.99, TRUE, 1),
    ('Fresh Rolls',       'Rice paper rolls with shrimp, herbs, and peanut sauce',     7.49, TRUE, 1),
    ('Fried Tofu',        'Golden crispy tofu with sweet chili dip',                   5.99, TRUE, 1);

-- Menu Items: Curries
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Green Curry',       'Coconut green curry with bamboo shoots and basil',         13.99, TRUE, 2),
    ('Red Curry',         'Spicy red curry with bell peppers and Thai eggplant',      13.99, TRUE, 2),
    ('Massaman Curry',    'Rich peanut curry with potatoes and onions',               14.99, TRUE, 2),
    ('Panang Curry',      'Creamy panang curry with kaffir lime leaves',              14.49, TRUE, 2),
    ('Yellow Curry',      'Mild coconut curry with potatoes and carrots',             12.99, TRUE, 2);

-- Menu Items: Stir Fry & Noodles
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Pad Thai',           'Classic stir-fried rice noodles with shrimp and peanuts', 12.99, TRUE, 3),
    ('Pad See Ew',         'Wide rice noodles with Chinese broccoli and egg',         11.99, TRUE, 3),
    ('Drunken Noodles',    'Spicy wide noodles with basil and vegetables',            12.49, TRUE, 3),
    ('Pineapple Fried Rice','Fried rice with pineapple, cashews, and raisins',        11.99, TRUE, 3),
    ('Thai Basil Chicken', 'Stir-fried chicken with holy basil and chilies',          12.49, TRUE, 3);

-- Menu Items: Drinks
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Thai Iced Tea',     'Sweetened black tea with condensed milk',                   3.99, TRUE, 4),
    ('Coconut Water',     'Fresh young coconut water',                                 3.49, TRUE, 4),
    ('Lemongrass Tea',    'Hot lemongrass and ginger tea',                             2.99, TRUE, 4),
    ('Mango Smoothie',    'Fresh mango blended with ice',                              4.99, TRUE, 4);

-- Menu Items: Espresso & Coffee
INSERT INTO menu_item (name, description, price, available, category_id) VALUES
    ('Thai Coffee',       'Strong brewed coffee with sweetened condensed milk',         3.99, TRUE, 5),
    ('Espresso',          'Double shot of espresso',                                   2.99, TRUE, 5),
    ('Cappuccino',        'Espresso with steamed milk foam',                           4.49, TRUE, 5),
    ('Iced Latte',        'Espresso with cold milk over ice',                          4.49, TRUE, 5);

-- Default Users
INSERT INTO app_user (username, display_name, role, restaurant_id) VALUES
    ('sia_manager',   'Siam Garden Manager',    'Manager',    1),
    ('sia_frontdesk', 'Siam Garden Front Desk', 'Front Desk', 1),
    ('sia_kitchen',   'Siam Garden Kitchen',    'Kitchen',    1);

-- Sample Orders
INSERT INTO orders (table_number, status, customer_name, notes, created_at, updated_at, total, restaurant_id) VALUES
    (1, 'Pending',     'Walk-In Guest', '',         NOW(), NOW(), 24.97, 1),
    (3, 'In Progress', 'Table 3',       'No spicy', NOW(), NOW(), 0.00,  1);

-- Order items for first order (1x Spring Rolls + 2x Satay Chicken)
INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 1, mi.price, '', o.id, mi.id
FROM orders o, menu_item mi
WHERE o.table_number = 1 AND o.restaurant_id = 1 AND mi.name = 'Spring Rolls' AND mi.category_id = 1
LIMIT 1;

INSERT INTO order_item (quantity, unit_price, special_instructions, order_id, menu_item_id)
SELECT 2, mi.price, '', o.id, mi.id
FROM orders o, menu_item mi
WHERE o.table_number = 1 AND o.restaurant_id = 1 AND mi.name = 'Satay Chicken' AND mi.category_id = 1
LIMIT 1;

-- Reset sequences to avoid conflicts
SELECT setval('restaurant_id_seq', (SELECT MAX(id) FROM restaurant));
SELECT setval('category_id_seq', (SELECT MAX(id) FROM category));
SELECT setval('menu_item_id_seq', (SELECT MAX(id) FROM menu_item));
SELECT setval('orders_id_seq', (SELECT MAX(id) FROM orders));
SELECT setval('order_item_id_seq', (SELECT MAX(id) FROM order_item));
SELECT setval('app_user_id_seq', (SELECT MAX(id) FROM app_user));

COMMIT;
