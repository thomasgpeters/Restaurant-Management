#pragma once

#include "IApiService.h"
#include "ApiService.h"
#include <memory>

// ─── Local (SQLite / Wt::Dbo) implementation of IApiService ─────────────────
// Delegates to the existing ApiService and converts Wt::Dbo::ptr<T> → DTO.

class LocalApiService : public IApiService {
public:
    explicit LocalApiService(const std::string& dbPath);

    void initializeDatabase();
    void seedDatabase();

    // ── IApiService ──
    std::vector<RestaurantDto> getRestaurants() override;
    RestaurantDto getRestaurant(long long id) override;

    std::vector<CategoryDto> getCategories(long long restaurantId) override;

    std::vector<MenuItemDto> getMenuItemsByCategory(long long categoryId) override;
    std::vector<MenuItemDto> getMenuItemsByRestaurant(long long restaurantId) override;
    MenuItemDto getMenuItem(long long id) override;
    void updateMenuItemAvailability(long long id, bool available) override;

    std::vector<OrderDto> getOrders(long long restaurantId) override;
    std::vector<OrderDto> getOrdersByStatus(long long restaurantId,
                                             const std::string& status) override;
    std::vector<OrderDto> getActiveOrders(long long restaurantId) override;
    OrderDto getOrder(long long id) override;
    OrderDto createOrder(long long restaurantId, int tableNumber,
                          const std::string& customerName,
                          const std::string& notes) override;
    void addOrderItem(long long orderId, long long menuItemId,
                      int quantity, const std::string& instructions) override;
    void updateOrderStatus(long long orderId, const std::string& status) override;
    void cancelOrder(long long orderId) override;

    std::vector<OrderItemDto> getOrderItems(long long orderId) override;

    int getOrderCount(long long restaurantId) override;
    double getRevenue(long long restaurantId) override;
    int getPendingOrderCount(long long restaurantId) override;
    int getInProgressOrderCount(long long restaurantId) override;

private:
    std::unique_ptr<ApiService> dbo_;
};
