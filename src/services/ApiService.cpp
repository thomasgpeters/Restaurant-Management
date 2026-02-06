#include "ApiService.h"
#include <iostream>
#include <iomanip>
#include <chrono>

ApiService::ApiService(const std::string& dbPath) {
    connection_ = std::make_unique<Wt::Dbo::backend::Sqlite3>(dbPath);
    connection_->setProperty("show-queries", "false");
    session_.setConnection(std::move(connection_));

    session_.mapClass<Restaurant>("restaurant");
    session_.mapClass<Category>("category");
    session_.mapClass<MenuItem>("menu_item");
    session_.mapClass<Order>("orders");
    session_.mapClass<OrderItem>("order_item");
    session_.mapClass<User>("app_user");
}

void ApiService::initializeDatabase() {
    try {
        session_.createTables();
        std::cout << "[ApiService] Database tables created." << std::endl;
    } catch (Wt::Dbo::Exception& e) {
        std::cout << "[ApiService] Tables may already exist: " << e.what() << std::endl;
    }
}

std::string ApiService::getNowTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// ─── Seed helpers ────────────────────────────────────────────────────────────

void ApiService::seedRestaurant(
    const std::string& name, const std::string& cuisine,
    const std::string& desc,
    const std::vector<std::pair<std::string,
        std::vector<std::tuple<std::string,std::string,double>>>>& menu)
{
    Wt::Dbo::Transaction t(session_);

    auto restaurant = session_.add(std::make_unique<Restaurant>());
    restaurant.modify()->name = name;
    restaurant.modify()->cuisine_type = cuisine;
    restaurant.modify()->description = desc;

    int sortOrder = 0;
    for (auto& [catName, items] : menu) {
        auto cat = session_.add(std::make_unique<Category>());
        cat.modify()->name = catName;
        cat.modify()->sort_order = sortOrder++;
        cat.modify()->restaurant = restaurant;

        for (auto& [iName, iDesc, iPrice] : items) {
            auto item = session_.add(std::make_unique<MenuItem>());
            item.modify()->name = iName;
            item.modify()->description = iDesc;
            item.modify()->price = iPrice;
            item.modify()->available = true;
            item.modify()->category = cat;
        }
    }

    // Create default users for each restaurant
    auto mgr = session_.add(std::make_unique<User>());
    mgr.modify()->username = name.substr(0, 3) + "_manager";
    mgr.modify()->display_name = name + " Manager";
    mgr.modify()->role = "Manager";
    mgr.modify()->restaurant = restaurant;

    auto fd = session_.add(std::make_unique<User>());
    fd.modify()->username = name.substr(0, 3) + "_frontdesk";
    fd.modify()->display_name = name + " Front Desk";
    fd.modify()->role = "Front Desk";
    fd.modify()->restaurant = restaurant;

    auto kit = session_.add(std::make_unique<User>());
    kit.modify()->username = name.substr(0, 3) + "_kitchen";
    kit.modify()->display_name = name + " Kitchen";
    kit.modify()->role = "Kitchen";
    kit.modify()->restaurant = restaurant;

    t.commit();
}

void ApiService::seedDatabase() {
    {
        Wt::Dbo::Transaction t(session_);
        int count = session_.query<int>("select count(1) from restaurant");
        t.commit();
        if (count > 0) {
            std::cout << "[ApiService] Database already seeded." << std::endl;
            return;
        }
    }

    std::cout << "[ApiService] Seeding database..." << std::endl;

    // ── Thai Restaurant ──────────────────────────────────────────────────
    seedRestaurant("Siam Garden", "Thai", "Authentic Thai cuisine with bold flavors",
    {
        {"Appetizers", {
            {"Spring Rolls",      "Crispy vegetable spring rolls with sweet chili sauce",     6.99},
            {"Satay Chicken",     "Grilled chicken skewers with peanut dipping sauce",        8.99},
            {"Tom Yum Soup",      "Spicy and sour soup with shrimp and mushrooms",            7.99},
            {"Fresh Rolls",       "Rice paper rolls with shrimp, herbs, and peanut sauce",    7.49},
            {"Fried Tofu",        "Golden crispy tofu with sweet chili dip",                  5.99},
        }},
        {"Curries", {
            {"Green Curry",       "Coconut green curry with bamboo shoots and basil",        13.99},
            {"Red Curry",         "Spicy red curry with bell peppers and Thai eggplant",     13.99},
            {"Massaman Curry",    "Rich peanut curry with potatoes and onions",              14.99},
            {"Panang Curry",      "Creamy panang curry with kaffir lime leaves",             14.49},
            {"Yellow Curry",      "Mild coconut curry with potatoes and carrots",            12.99},
        }},
        {"Stir Fry & Noodles", {
            {"Pad Thai",          "Classic stir-fried rice noodles with shrimp and peanuts", 12.99},
            {"Pad See Ew",        "Wide rice noodles with Chinese broccoli and egg",         11.99},
            {"Drunken Noodles",   "Spicy wide noodles with basil and vegetables",            12.49},
            {"Pineapple Fried Rice","Fried rice with pineapple, cashews, and raisins",       11.99},
            {"Thai Basil Chicken","Stir-fried chicken with holy basil and chilies",          12.49},
        }},
        {"Drinks", {
            {"Thai Iced Tea",     "Sweetened black tea with condensed milk",                   3.99},
            {"Coconut Water",     "Fresh young coconut water",                                 3.49},
            {"Lemongrass Tea",    "Hot lemongrass and ginger tea",                             2.99},
            {"Mango Smoothie",    "Fresh mango blended with ice",                              4.99},
        }},
        {"Espresso & Coffee", {
            {"Thai Coffee",       "Strong brewed coffee with sweetened condensed milk",         3.99},
            {"Espresso",          "Double shot of espresso",                                   2.99},
            {"Cappuccino",        "Espresso with steamed milk foam",                           4.49},
            {"Iced Latte",        "Espresso with cold milk over ice",                          4.49},
        }},
    });

    // ── Chinese Restaurant ───────────────────────────────────────────────
    seedRestaurant("Golden Dragon", "Chinese", "Traditional Chinese dishes from multiple regions",
    {
        {"Appetizers", {
            {"Wonton Soup",       "Pork and shrimp wontons in clear broth",                   6.99},
            {"Egg Rolls",         "Crispy pork and vegetable egg rolls",                       5.99},
            {"Potstickers",       "Pan-fried pork dumplings with soy dipping sauce",          7.99},
            {"Hot & Sour Soup",   "Classic spicy and tangy soup with tofu and mushrooms",     6.49},
            {"Crab Rangoon",      "Fried wonton with cream cheese and crab filling",          7.49},
        }},
        {"Main Courses", {
            {"Kung Pao Chicken",  "Spicy diced chicken with peanuts and dried chilies",      13.99},
            {"Sweet & Sour Pork", "Crispy pork in tangy sweet and sour sauce",               12.99},
            {"Beef & Broccoli",   "Tender beef and broccoli in savory brown sauce",          14.49},
            {"Mapo Tofu",         "Soft tofu in spicy Sichuan chili bean sauce",             11.99},
            {"General Tso Chicken","Crispy chicken in a sweet and mildly spicy sauce",       13.49},
            {"Mongolian Beef",    "Sliced beef with scallions in sweet soy sauce",           14.99},
        }},
        {"Noodles & Rice", {
            {"Lo Mein",           "Soft egg noodles with vegetables and choice of protein",  11.99},
            {"Chow Fun",          "Wide rice noodles stir-fried with beef and bean sprouts", 12.49},
            {"Yang Chow Fried Rice","Fried rice with shrimp, pork, and egg",                 10.99},
            {"Dan Dan Noodles",   "Spicy Sichuan noodles with ground pork and peanuts",     11.49},
        }},
        {"Drinks", {
            {"Jasmine Tea",       "Fragrant hot jasmine green tea",                           2.49},
            {"Chinese Iced Tea",  "Chilled chrysanthemum tea",                                2.99},
            {"Lychee Juice",      "Sweet lychee fruit juice",                                 3.49},
            {"Plum Juice",        "Traditional sour plum drink",                              3.49},
        }},
        {"Espresso & Coffee", {
            {"Yuan Yang",         "Hong Kong-style coffee and tea blend",                      3.99},
            {"Espresso",          "Double shot of espresso",                                   2.99},
            {"Mocha",             "Espresso with chocolate and steamed milk",                  4.99},
            {"Iced Americano",    "Espresso with cold water over ice",                         3.49},
        }},
    });

    // ── Sandwich Shop ────────────────────────────────────────────────────
    seedRestaurant("The Crafted Bite", "Sandwiches & More", "Gourmet sandwiches, fresh salads, and espresso",
    {
        {"Sandwiches", {
            {"Classic Club",      "Turkey, bacon, lettuce, tomato on sourdough",             10.99},
            {"Philly Cheesesteak","Shaved beef, peppers, onions, and provolone on hoagie",  12.99},
            {"Caprese Panini",    "Fresh mozzarella, tomato, basil, and balsamic on ciabatta",10.49},
            {"Reuben",            "Corned beef, sauerkraut, Swiss, and Thousand Island on rye",11.99},
            {"BBQ Pulled Pork",   "Slow-smoked pulled pork with coleslaw on brioche",       11.49},
            {"Veggie Wrap",       "Hummus, roasted vegetables, and feta in a spinach tortilla",9.49},
            {"Turkey Avocado",    "Smoked turkey, avocado, sprouts, and aioli on wheat",    10.99},
            {"Grilled Chicken BLT","Grilled chicken breast with bacon, lettuce, tomato",    11.49},
        }},
        {"Salads", {
            {"Caesar Salad",      "Romaine, parmesan, croutons, and Caesar dressing",         8.99},
            {"Greek Salad",       "Mixed greens, feta, olives, cucumber, tomato, red onion",  9.49},
            {"Cobb Salad",        "Chicken, bacon, egg, avocado, blue cheese, and tomato",   11.99},
            {"Asian Sesame Salad","Mixed greens, mandarin, almonds, crispy wontons, sesame", 10.49},
            {"Harvest Bowl",      "Quinoa, roasted sweet potato, kale, cranberries, goat cheese",10.99},
        }},
        {"Sides", {
            {"French Fries",      "Crispy golden fries with sea salt",                        3.99},
            {"Onion Rings",       "Beer-battered onion rings",                                4.49},
            {"Sweet Potato Fries","Crispy sweet potato fries with chipotle aioli",            4.99},
            {"Cup of Soup",       "Daily rotating soup selection",                            4.49},
        }},
        {"Drinks", {
            {"Fresh Lemonade",    "House-made lemonade with real lemons",                     3.49},
            {"Iced Green Tea",    "Brewed green tea over ice",                                2.99},
            {"Sparkling Water",   "San Pellegrino sparkling mineral water",                   2.49},
            {"Fresh OJ",          "Freshly squeezed orange juice",                            4.49},
            {"Craft Soda",        "Rotating selection of artisan sodas",                      3.49},
        }},
        {"Espresso & Coffee", {
            {"Espresso",          "Double shot of locally roasted espresso",                   2.99},
            {"Americano",         "Espresso with hot water",                                   3.49},
            {"Cappuccino",        "Espresso with velvety steamed milk foam",                   4.49},
            {"Latte",             "Espresso with smooth steamed milk",                         4.49},
            {"Mocha",             "Espresso, chocolate, steamed milk, whipped cream",          4.99},
            {"Cold Brew",         "Slow-steeped cold brew coffee, served over ice",            3.99},
            {"Flat White",        "Ristretto shots with micro-foam milk",                      4.49},
        }},
    });

    // Seed a few sample orders
    {
        Wt::Dbo::Transaction t(session_);

        auto restaurants = session_.find<Restaurant>().resultList();
        for (auto& rest : restaurants) {
            // Create two sample orders per restaurant
            auto order1 = session_.add(std::make_unique<Order>());
            order1.modify()->table_number = 1;
            order1.modify()->status = "Pending";
            order1.modify()->customer_name = "Walk-In Guest";
            order1.modify()->notes = "";
            order1.modify()->created_at = getNowTimestamp();
            order1.modify()->updated_at = getNowTimestamp();
            order1.modify()->restaurant = rest;

            auto order2 = session_.add(std::make_unique<Order>());
            order2.modify()->table_number = 3;
            order2.modify()->status = "In Progress";
            order2.modify()->customer_name = "Table 3";
            order2.modify()->notes = "No spicy";
            order2.modify()->created_at = getNowTimestamp();
            order2.modify()->updated_at = getNowTimestamp();
            order2.modify()->restaurant = rest;

            // Add items to first order
            auto cats = rest->categories;
            if (!cats.empty()) {
                auto firstCat = *cats.begin();
                auto menuItems = firstCat->items;
                int count = 0;
                for (auto& mi : menuItems) {
                    if (count >= 2) break;
                    auto oi = session_.add(std::make_unique<OrderItem>());
                    oi.modify()->quantity = count + 1;
                    oi.modify()->unit_price = mi->price;
                    oi.modify()->special_instructions = "";
                    oi.modify()->order = order1;
                    oi.modify()->menu_item = mi;
                    order1.modify()->total += mi->price * (count + 1);
                    count++;
                }
            }
        }
        t.commit();
    }

    std::cout << "[ApiService] Database seeded successfully." << std::endl;
}

// ─── Restaurant endpoints ────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<Restaurant>> ApiService::getRestaurants() {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<Restaurant>().resultList();
    return std::vector<Wt::Dbo::ptr<Restaurant>>(results.begin(), results.end());
}

Wt::Dbo::ptr<Restaurant> ApiService::getRestaurant(long long id) {
    Wt::Dbo::Transaction t(session_);
    return session_.find<Restaurant>().where("id = ?").bind(id).resultValue();
}

// ─── Category endpoints ──────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<Category>> ApiService::getCategories(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<Category>()
        .where("restaurant_id = ?").bind(restaurantId)
        .orderBy("sort_order")
        .resultList();
    return std::vector<Wt::Dbo::ptr<Category>>(results.begin(), results.end());
}

// ─── MenuItem endpoints ──────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<MenuItem>> ApiService::getMenuItemsByCategory(long long categoryId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<MenuItem>()
        .where("category_id = ?").bind(categoryId)
        .resultList();
    return std::vector<Wt::Dbo::ptr<MenuItem>>(results.begin(), results.end());
}

std::vector<Wt::Dbo::ptr<MenuItem>> ApiService::getMenuItemsByRestaurant(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<MenuItem>()
        .where("category_id in (select id from category where restaurant_id = ?)")
        .bind(restaurantId)
        .resultList();
    return std::vector<Wt::Dbo::ptr<MenuItem>>(results.begin(), results.end());
}

Wt::Dbo::ptr<MenuItem> ApiService::getMenuItem(long long id) {
    Wt::Dbo::Transaction t(session_);
    return session_.find<MenuItem>().where("id = ?").bind(id).resultValue();
}

void ApiService::updateMenuItemAvailability(long long id, bool available) {
    Wt::Dbo::Transaction t(session_);
    auto item = session_.find<MenuItem>().where("id = ?").bind(id).resultValue();
    if (item) {
        item.modify()->available = available;
    }
    t.commit();
}

// ─── Order endpoints ─────────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<Order>> ApiService::getOrders(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<Order>()
        .where("restaurant_id = ?").bind(restaurantId)
        .orderBy("id DESC")
        .resultList();
    return std::vector<Wt::Dbo::ptr<Order>>(results.begin(), results.end());
}

std::vector<Wt::Dbo::ptr<Order>> ApiService::getOrdersByStatus(
    long long restaurantId, const std::string& status)
{
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<Order>()
        .where("restaurant_id = ? AND status = ?")
        .bind(restaurantId).bind(status)
        .orderBy("id ASC")
        .resultList();
    return std::vector<Wt::Dbo::ptr<Order>>(results.begin(), results.end());
}

std::vector<Wt::Dbo::ptr<Order>> ApiService::getActiveOrders(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<Order>()
        .where("restaurant_id = ? AND status != 'Served' AND status != 'Cancelled'")
        .bind(restaurantId)
        .orderBy("id ASC")
        .resultList();
    return std::vector<Wt::Dbo::ptr<Order>>(results.begin(), results.end());
}

Wt::Dbo::ptr<Order> ApiService::getOrder(long long id) {
    Wt::Dbo::Transaction t(session_);
    return session_.find<Order>().where("id = ?").bind(id).resultValue();
}

Wt::Dbo::ptr<Order> ApiService::createOrder(
    long long restaurantId, int tableNumber,
    const std::string& customerName, const std::string& notes)
{
    Wt::Dbo::Transaction t(session_);
    auto rest = session_.find<Restaurant>().where("id = ?").bind(restaurantId).resultValue();
    auto order = session_.add(std::make_unique<Order>());
    order.modify()->table_number = tableNumber;
    order.modify()->status = "Pending";
    order.modify()->customer_name = customerName;
    order.modify()->notes = notes;
    order.modify()->created_at = getNowTimestamp();
    order.modify()->updated_at = getNowTimestamp();
    order.modify()->total = 0.0;
    order.modify()->restaurant = rest;
    t.commit();
    return order;
}

void ApiService::addOrderItem(long long orderId, long long menuItemId,
                               int quantity, const std::string& instructions)
{
    Wt::Dbo::Transaction t(session_);
    auto order = session_.find<Order>().where("id = ?").bind(orderId).resultValue();
    auto menuItem = session_.find<MenuItem>().where("id = ?").bind(menuItemId).resultValue();

    if (order && menuItem) {
        auto oi = session_.add(std::make_unique<OrderItem>());
        oi.modify()->quantity = quantity;
        oi.modify()->unit_price = menuItem->price;
        oi.modify()->special_instructions = instructions;
        oi.modify()->order = order;
        oi.modify()->menu_item = menuItem;

        order.modify()->total += menuItem->price * quantity;
        order.modify()->updated_at = getNowTimestamp();
    }
    t.commit();
}

void ApiService::updateOrderStatus(long long orderId, const std::string& status) {
    Wt::Dbo::Transaction t(session_);
    auto order = session_.find<Order>().where("id = ?").bind(orderId).resultValue();
    if (order) {
        order.modify()->status = status;
        order.modify()->updated_at = getNowTimestamp();
    }
    t.commit();
}

void ApiService::cancelOrder(long long orderId) {
    updateOrderStatus(orderId, "Cancelled");
}

// ─── OrderItem endpoints ─────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<OrderItem>> ApiService::getOrderItems(long long orderId) {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<OrderItem>()
        .where("order_id = ?").bind(orderId)
        .resultList();
    return std::vector<Wt::Dbo::ptr<OrderItem>>(results.begin(), results.end());
}

// ─── User endpoints ──────────────────────────────────────────────────────────

std::vector<Wt::Dbo::ptr<User>> ApiService::getUsers() {
    Wt::Dbo::Transaction t(session_);
    auto results = session_.find<User>().resultList();
    return std::vector<Wt::Dbo::ptr<User>>(results.begin(), results.end());
}

Wt::Dbo::ptr<User> ApiService::getUser(long long id) {
    Wt::Dbo::Transaction t(session_);
    return session_.find<User>().where("id = ?").bind(id).resultValue();
}

Wt::Dbo::ptr<User> ApiService::getUserByUsername(const std::string& username) {
    Wt::Dbo::Transaction t(session_);
    return session_.find<User>().where("username = ?").bind(username).resultValue();
}

// ─── Dashboard / reporting ───────────────────────────────────────────────────

int ApiService::getOrderCount(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    return session_.query<int>(
        "select count(1) from orders where restaurant_id = ?").bind(restaurantId);
}

double ApiService::getRevenue(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    auto val = session_.query<double>(
        "select coalesce(sum(total),0) from orders where restaurant_id = ? "
        "AND status = 'Served'").bind(restaurantId);
    return val;
}

int ApiService::getPendingOrderCount(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    return session_.query<int>(
        "select count(1) from orders where restaurant_id = ? AND status = 'Pending'")
        .bind(restaurantId);
}

int ApiService::getInProgressOrderCount(long long restaurantId) {
    Wt::Dbo::Transaction t(session_);
    return session_.query<int>(
        "select count(1) from orders where restaurant_id = ? AND status = 'In Progress'")
        .bind(restaurantId);
}
