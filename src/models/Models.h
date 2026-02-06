#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

class Restaurant;
class Category;
class MenuItem;
class Order;
class OrderItem;
class User;

// ─── Restaurant ──────────────────────────────────────────────────────────────

class Restaurant {
public:
    std::string name;
    std::string cuisine_type;
    std::string description;

    Wt::Dbo::collection<Wt::Dbo::ptr<Category>> categories;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, cuisine_type, "cuisine_type");
        Wt::Dbo::field(a, description, "description");
        Wt::Dbo::hasMany(a, categories, Wt::Dbo::ManyToOne, "restaurant");
    }
};

// ─── Category ────────────────────────────────────────────────────────────────

class Category {
public:
    std::string name;
    int sort_order = 0;

    Wt::Dbo::ptr<Restaurant> restaurant;
    Wt::Dbo::collection<Wt::Dbo::ptr<MenuItem>> items;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, sort_order, "sort_order");
        Wt::Dbo::belongsTo(a, restaurant, "restaurant");
        Wt::Dbo::hasMany(a, items, Wt::Dbo::ManyToOne, "category");
    }
};

// ─── MenuItem ────────────────────────────────────────────────────────────────

class MenuItem {
public:
    std::string name;
    std::string description;
    double price = 0.0;
    bool available = true;

    Wt::Dbo::ptr<Category> category;
    Wt::Dbo::collection<Wt::Dbo::ptr<OrderItem>> order_items;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, description, "description");
        Wt::Dbo::field(a, price, "price");
        Wt::Dbo::field(a, available, "available");
        Wt::Dbo::belongsTo(a, category, "category");
        Wt::Dbo::hasMany(a, order_items, Wt::Dbo::ManyToOne, "menu_item");
    }
};

// ─── Order Status Enum ───────────────────────────────────────────────────────

enum class OrderStatus {
    Pending,
    InProgress,
    Ready,
    Served,
    Cancelled
};

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
        case OrderStatus::Pending:    return "Pending";
        case OrderStatus::InProgress: return "In Progress";
        case OrderStatus::Ready:      return "Ready";
        case OrderStatus::Served:     return "Served";
        case OrderStatus::Cancelled:  return "Cancelled";
    }
    return "Unknown";
}

inline OrderStatus stringToOrderStatus(const std::string& s) {
    if (s == "Pending")     return OrderStatus::Pending;
    if (s == "In Progress") return OrderStatus::InProgress;
    if (s == "Ready")       return OrderStatus::Ready;
    if (s == "Served")      return OrderStatus::Served;
    if (s == "Cancelled")   return OrderStatus::Cancelled;
    return OrderStatus::Pending;
}

// ─── Order ───────────────────────────────────────────────────────────────────

class Order {
public:
    int table_number = 0;
    std::string status = "Pending";
    std::string customer_name;
    std::string notes;
    std::string created_at;
    std::string updated_at;
    double total = 0.0;

    Wt::Dbo::ptr<Restaurant> restaurant;
    Wt::Dbo::collection<Wt::Dbo::ptr<OrderItem>> items;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, table_number, "table_number");
        Wt::Dbo::field(a, status, "status");
        Wt::Dbo::field(a, customer_name, "customer_name");
        Wt::Dbo::field(a, notes, "notes");
        Wt::Dbo::field(a, created_at, "created_at");
        Wt::Dbo::field(a, updated_at, "updated_at");
        Wt::Dbo::field(a, total, "total");
        Wt::Dbo::belongsTo(a, restaurant, "restaurant");
        Wt::Dbo::hasMany(a, items, Wt::Dbo::ManyToOne, "order");
    }
};

// ─── OrderItem ───────────────────────────────────────────────────────────────

class OrderItem {
public:
    int quantity = 1;
    double unit_price = 0.0;
    std::string special_instructions;

    Wt::Dbo::ptr<Order> order;
    Wt::Dbo::ptr<MenuItem> menu_item;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, quantity, "quantity");
        Wt::Dbo::field(a, unit_price, "unit_price");
        Wt::Dbo::field(a, special_instructions, "special_instructions");
        Wt::Dbo::belongsTo(a, order, "order");
        Wt::Dbo::belongsTo(a, menu_item, "menu_item");
    }
};

// ─── User Role Enum ──────────────────────────────────────────────────────────

enum class UserRole {
    Manager,
    FrontDesk,
    Kitchen
};

inline std::string userRoleToString(UserRole r) {
    switch (r) {
        case UserRole::Manager:   return "Manager";
        case UserRole::FrontDesk: return "Front Desk";
        case UserRole::Kitchen:   return "Kitchen";
    }
    return "Unknown";
}

inline UserRole stringToUserRole(const std::string& s) {
    if (s == "Manager")    return UserRole::Manager;
    if (s == "Front Desk") return UserRole::FrontDesk;
    if (s == "Kitchen")    return UserRole::Kitchen;
    return UserRole::FrontDesk;
}

// ─── User ────────────────────────────────────────────────────────────────────

class User {
public:
    std::string username;
    std::string display_name;
    std::string role;   // "Manager", "Front Desk", "Kitchen"

    Wt::Dbo::ptr<Restaurant> restaurant;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, username, "username");
        Wt::Dbo::field(a, display_name, "display_name");
        Wt::Dbo::field(a, role, "role");
        Wt::Dbo::belongsTo(a, restaurant, "restaurant");
    }
};
