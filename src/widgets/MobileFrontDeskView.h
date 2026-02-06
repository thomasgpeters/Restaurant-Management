#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>
#include <Wt/WTextArea.h>
#include <memory>
#include <vector>

#include "../services/ApiService.h"

class RestaurantApp;  // forward declaration

struct MobileCartItem {
    long long menuItemId;
    std::string name;
    double price;
    int quantity;
};

// Screen flow:
//   Categories -> Menu Items -> Cart Review -> Order Confirmation
//   Bottom tab bar: Menu | Cart (badge) | Orders
enum class MobileScreen {
    Categories,
    MenuItems,
    Cart,
    OrderForm,
    Confirmation,
    ActiveOrders
};

class MobileFrontDeskView : public Wt::WContainerWidget {
public:
    MobileFrontDeskView(std::shared_ptr<ApiService> api, long long restaurantId,
                        RestaurantApp* app = nullptr, bool isTablet = false);

private:
    // Screen builders
    void buildTabBar();
    void buildMenuBrowserScreen();   // tablet: split categories/items
    void refreshItemsPanel();
    void buildCategoriesScreen();    // phone: full-screen category list
    void buildMenuItemsScreen();     // phone: full-screen items with back
    void buildCartScreen();
    void buildOrderFormScreen();
    void buildConfirmationScreen(long long orderId);
    void buildActiveOrdersScreen();

    // Navigation
    void navigateTo(MobileScreen screen);
    void navigateBack();

    // Category -> Items
    void showCategoryItems(long long categoryId, const std::string& categoryName);

    // Cart operations
    void addToCart(long long menuItemId, const std::string& name, double price);
    void removeFromCart(int index);
    void updateCartBadge();
    void updateCartBubble();
    double cartTotal() const;

    // Order submission
    void submitOrder();

    std::shared_ptr<ApiService> api_;
    long long restaurantId_;
    RestaurantApp* app_ = nullptr;  // for header cart updates
    bool isTablet_ = false;         // tablet: split panel, phone: sequential

    // Navigation state
    MobileScreen currentScreen_ = MobileScreen::Categories;
    std::vector<MobileScreen> screenHistory_;
    long long currentCategoryId_ = -1;
    std::string currentCategoryName_;

    // Layout
    Wt::WContainerWidget* screenContainer_ = nullptr;  // holds the current screen
    Wt::WContainerWidget* tabBar_ = nullptr;
    Wt::WText* cartBadge_ = nullptr;
    Wt::WContainerWidget* tabMenu_ = nullptr;
    Wt::WContainerWidget* tabCart_ = nullptr;
    Wt::WContainerWidget* tabOrders_ = nullptr;

    // Menu browser split-panel
    Wt::WContainerWidget* itemsPanel_ = nullptr;

    // Order form fields (persisted across nav)
    std::string customerName_;
    int tableNumber_ = 1;
    std::string orderNotes_;

    // Cart
    std::vector<MobileCartItem> cart_;

    // Last confirmed order
    long long lastOrderId_ = -1;
};
