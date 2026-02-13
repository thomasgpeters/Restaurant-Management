#pragma once

#include <string>
#include <vector>

// ─── Plain Data Transfer Objects ─────────────────────────────────────────────
// Decoupled from Wt::Dbo ORM.  Both LocalApiService (SQLite) and
// RestApiService (ApiLogicServer JSON:API) return these same types.

struct RestaurantDto {
    long long id = 0;
    std::string name;
    std::string cuisine_type;
    std::string description;
};

struct CategoryDto {
    long long id = 0;
    std::string name;
    int sort_order = 0;
    long long restaurant_id = 0;
};

struct MenuItemDto {
    long long id = 0;
    std::string name;
    std::string description;
    double price = 0.0;
    bool available = true;
    long long category_id = 0;
};

struct OrderDto {
    long long id = 0;
    int table_number = 0;
    std::string status;
    std::string customer_name;
    std::string notes;
    double total = 0.0;
    long long restaurant_id = 0;
    std::string created_at;
};

struct OrderItemDto {
    long long id = 0;
    int quantity = 0;
    double unit_price = 0.0;
    std::string special_instructions;
    long long order_id = 0;
    long long menu_item_id = 0;
    std::string menu_item_name;  // denormalized for display
};

struct UserDto {
    long long id = 0;
    std::string username;
    std::string display_name;
    std::string role;
    long long restaurant_id = 0;
};
