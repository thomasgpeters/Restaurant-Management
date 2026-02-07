#pragma once

#include "../models/Dto.h"
#include <vector>
#include <string>
#include <memory>

// ─── Abstract API Service Interface ──────────────────────────────────────────
// All widgets program against this interface.  Concrete implementations:
//   LocalApiService  – Wt::Dbo / SQLite  (offline / local dev)
//   RestApiService   – HTTP JSON:API      (ApiLogicServer)

class IApiService {
public:
    virtual ~IApiService() = default;

    // ── Restaurant ──
    virtual std::vector<RestaurantDto> getRestaurants() = 0;
    virtual RestaurantDto getRestaurant(long long id) = 0;

    // ── Category ──
    virtual std::vector<CategoryDto> getCategories(long long restaurantId) = 0;

    // ── MenuItem ──
    virtual std::vector<MenuItemDto> getMenuItemsByCategory(long long categoryId) = 0;
    virtual std::vector<MenuItemDto> getMenuItemsByRestaurant(long long restaurantId) = 0;
    virtual MenuItemDto getMenuItem(long long id) = 0;
    virtual void updateMenuItemAvailability(long long id, bool available) = 0;

    // ── Order ──
    virtual std::vector<OrderDto> getOrders(long long restaurantId) = 0;
    virtual std::vector<OrderDto> getOrdersByStatus(long long restaurantId,
                                                     const std::string& status) = 0;
    virtual std::vector<OrderDto> getActiveOrders(long long restaurantId) = 0;
    virtual OrderDto getOrder(long long id) = 0;
    virtual OrderDto createOrder(long long restaurantId, int tableNumber,
                                  const std::string& customerName,
                                  const std::string& notes) = 0;
    virtual void addOrderItem(long long orderId, long long menuItemId,
                              int quantity, const std::string& instructions) = 0;
    virtual void updateOrderStatus(long long orderId, const std::string& status) = 0;
    virtual void cancelOrder(long long orderId) = 0;

    // ── OrderItem ──
    virtual std::vector<OrderItemDto> getOrderItems(long long orderId) = 0;

    // ── Dashboard / reporting ──
    virtual int getOrderCount(long long restaurantId) = 0;
    virtual double getRevenue(long long restaurantId) = 0;
    virtual int getPendingOrderCount(long long restaurantId) = 0;
    virtual int getInProgressOrderCount(long long restaurantId) = 0;
};
