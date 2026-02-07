#include "LocalApiService.h"

LocalApiService::LocalApiService(const std::string& dbPath)
    : dbo_(std::make_unique<ApiService>(dbPath))
{}

void LocalApiService::initializeDatabase() { dbo_->initializeDatabase(); }
void LocalApiService::seedDatabase()       { dbo_->seedDatabase(); }

// ─── Helpers: Wt::Dbo::ptr<T> → DTO ─────────────────────────────────────────

static RestaurantDto toDto(const Wt::Dbo::ptr<Restaurant>& p) {
    RestaurantDto d;
    d.id           = p.id();
    d.name         = p->name;
    d.cuisine_type = p->cuisine_type;
    d.description  = p->description;
    return d;
}

static CategoryDto toDto(const Wt::Dbo::ptr<Category>& p) {
    CategoryDto d;
    d.id            = p.id();
    d.name          = p->name;
    d.sort_order    = p->sort_order;
    d.restaurant_id = p->restaurant.id();
    return d;
}

static MenuItemDto toDto(const Wt::Dbo::ptr<MenuItem>& p) {
    MenuItemDto d;
    d.id          = p.id();
    d.name        = p->name;
    d.description = p->description;
    d.price       = p->price;
    d.available   = p->available;
    d.category_id = p->category.id();
    return d;
}

static OrderDto toDto(const Wt::Dbo::ptr<Order>& p) {
    OrderDto d;
    d.id            = p.id();
    d.table_number  = p->table_number;
    d.status        = p->status;
    d.customer_name = p->customer_name;
    d.notes         = p->notes;
    d.total         = p->total;
    d.restaurant_id = p->restaurant.id();
    d.created_at    = p->created_at;
    return d;
}

static OrderItemDto toDto(const Wt::Dbo::ptr<OrderItem>& p) {
    OrderItemDto d;
    d.id                   = p.id();
    d.quantity             = p->quantity;
    d.unit_price           = p->unit_price;
    d.special_instructions = p->special_instructions;
    d.order_id             = p->order.id();
    d.menu_item_id         = p->menu_item.id();
    d.menu_item_name       = p->menu_item->name;
    return d;
}

// ─── IApiService implementation ──────────────────────────────────────────────

std::vector<RestaurantDto> LocalApiService::getRestaurants() {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getRestaurants();
    std::vector<RestaurantDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

RestaurantDto LocalApiService::getRestaurant(long long id) {
    Wt::Dbo::Transaction t(dbo_->session());
    return toDto(dbo_->getRestaurant(id));
}

std::vector<CategoryDto> LocalApiService::getCategories(long long restaurantId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getCategories(restaurantId);
    std::vector<CategoryDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

std::vector<MenuItemDto> LocalApiService::getMenuItemsByCategory(long long categoryId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getMenuItemsByCategory(categoryId);
    std::vector<MenuItemDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

std::vector<MenuItemDto> LocalApiService::getMenuItemsByRestaurant(long long restaurantId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getMenuItemsByRestaurant(restaurantId);
    std::vector<MenuItemDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

MenuItemDto LocalApiService::getMenuItem(long long id) {
    Wt::Dbo::Transaction t(dbo_->session());
    return toDto(dbo_->getMenuItem(id));
}

void LocalApiService::updateMenuItemAvailability(long long id, bool available) {
    dbo_->updateMenuItemAvailability(id, available);
}

std::vector<OrderDto> LocalApiService::getOrders(long long restaurantId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getOrders(restaurantId);
    std::vector<OrderDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

std::vector<OrderDto> LocalApiService::getOrdersByStatus(
    long long restaurantId, const std::string& status)
{
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getOrdersByStatus(restaurantId, status);
    std::vector<OrderDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

std::vector<OrderDto> LocalApiService::getActiveOrders(long long restaurantId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getActiveOrders(restaurantId);
    std::vector<OrderDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

OrderDto LocalApiService::getOrder(long long id) {
    Wt::Dbo::Transaction t(dbo_->session());
    return toDto(dbo_->getOrder(id));
}

OrderDto LocalApiService::createOrder(long long restaurantId, int tableNumber,
                                       const std::string& customerName,
                                       const std::string& notes)
{
    auto p = dbo_->createOrder(restaurantId, tableNumber, customerName, notes);
    Wt::Dbo::Transaction t(dbo_->session());
    return toDto(p);
}

void LocalApiService::addOrderItem(long long orderId, long long menuItemId,
                                    int quantity, const std::string& instructions)
{
    dbo_->addOrderItem(orderId, menuItemId, quantity, instructions);
}

void LocalApiService::updateOrderStatus(long long orderId, const std::string& status) {
    dbo_->updateOrderStatus(orderId, status);
}

void LocalApiService::cancelOrder(long long orderId) {
    dbo_->cancelOrder(orderId);
}

std::vector<OrderItemDto> LocalApiService::getOrderItems(long long orderId) {
    Wt::Dbo::Transaction t(dbo_->session());
    auto src = dbo_->getOrderItems(orderId);
    std::vector<OrderItemDto> out;
    out.reserve(src.size());
    for (auto& p : src) out.push_back(toDto(p));
    return out;
}

int    LocalApiService::getOrderCount(long long rid)        { return dbo_->getOrderCount(rid); }
double LocalApiService::getRevenue(long long rid)           { return dbo_->getRevenue(rid); }
int    LocalApiService::getPendingOrderCount(long long rid) { return dbo_->getPendingOrderCount(rid); }
int    LocalApiService::getInProgressOrderCount(long long rid) { return dbo_->getInProgressOrderCount(rid); }
