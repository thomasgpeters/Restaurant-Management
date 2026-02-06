#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <sstream>
#include <ctime>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "../models/Models.h"

// ─── Service layer that abstracts DB operations ──────────────────────────────
// Designed to be swappable with ApiLogicServer REST middleware.
// Each method mirrors a typical REST endpoint:
//   GET    /api/restaurants        -> getRestaurants()
//   GET    /api/menu_items?cat=id  -> getMenuItemsByCategory(id)
//   POST   /api/orders             -> createOrder(...)
//   PATCH  /api/orders/:id         -> updateOrderStatus(id, status)
//   etc.

class ApiService {
public:
    explicit ApiService(const std::string& dbPath);
    ~ApiService() = default;

    void initializeDatabase();
    void seedDatabase();

    // ── Restaurant endpoints ──
    std::vector<Wt::Dbo::ptr<Restaurant>> getRestaurants();
    Wt::Dbo::ptr<Restaurant> getRestaurant(long long id);

    // ── Category endpoints ──
    std::vector<Wt::Dbo::ptr<Category>> getCategories(long long restaurantId);

    // ── MenuItem endpoints ──
    std::vector<Wt::Dbo::ptr<MenuItem>> getMenuItemsByCategory(long long categoryId);
    std::vector<Wt::Dbo::ptr<MenuItem>> getMenuItemsByRestaurant(long long restaurantId);
    Wt::Dbo::ptr<MenuItem> getMenuItem(long long id);
    void updateMenuItemAvailability(long long id, bool available);

    // ── Order endpoints ──
    std::vector<Wt::Dbo::ptr<Order>> getOrders(long long restaurantId);
    std::vector<Wt::Dbo::ptr<Order>> getOrdersByStatus(long long restaurantId,
                                                         const std::string& status);
    std::vector<Wt::Dbo::ptr<Order>> getActiveOrders(long long restaurantId);
    Wt::Dbo::ptr<Order> getOrder(long long id);
    Wt::Dbo::ptr<Order> createOrder(long long restaurantId, int tableNumber,
                                     const std::string& customerName,
                                     const std::string& notes);
    void addOrderItem(long long orderId, long long menuItemId,
                      int quantity, const std::string& instructions);
    void updateOrderStatus(long long orderId, const std::string& status);
    void cancelOrder(long long orderId);

    // ── OrderItem endpoints ──
    std::vector<Wt::Dbo::ptr<OrderItem>> getOrderItems(long long orderId);

    // ── User endpoints ──
    std::vector<Wt::Dbo::ptr<User>> getUsers();
    Wt::Dbo::ptr<User> getUser(long long id);
    Wt::Dbo::ptr<User> getUserByUsername(const std::string& username);

    // ── Dashboard / reporting ──
    int getOrderCount(long long restaurantId);
    double getRevenue(long long restaurantId);
    int getPendingOrderCount(long long restaurantId);
    int getInProgressOrderCount(long long restaurantId);

    Wt::Dbo::Session& session() { return session_; }

private:
    std::string getNowTimestamp();
    void seedRestaurant(const std::string& name, const std::string& cuisine,
                        const std::string& desc,
                        const std::vector<std::pair<std::string,
                            std::vector<std::tuple<std::string,std::string,double>>>>& menu);

    std::unique_ptr<Wt::Dbo::backend::Sqlite3> connection_;
    Wt::Dbo::Session session_;
};
