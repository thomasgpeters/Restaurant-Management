#include "MobileFrontDeskView.h"
#include "../ui/RestaurantApp.h"

#include <Wt/WBreak.h>
#include <Wt/WTable.h>
#include <sstream>
#include <iomanip>

// ─── Constructor ─────────────────────────────────────────────────────────────

MobileFrontDeskView::MobileFrontDeskView(
    std::shared_ptr<ApiService> api, long long restaurantId,
    RestaurantApp* app)
    : api_(api), restaurantId_(restaurantId), app_(app)
{
    addStyleClass("m-frontdesk");

    // Screen area (fills between header and tab bar)
    screenContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    screenContainer_->addStyleClass("m-screen-container");

    // Register header cart bubble click to navigate to cart
    if (app_) {
        app_->setCartClickTarget([this] {
            navigateTo(MobileScreen::Cart);
        });
    }

    // Bottom tab bar
    buildTabBar();

    // Start on Categories screen
    navigateTo(MobileScreen::Categories);
}

// ─── Tab Bar ─────────────────────────────────────────────────────────────────

void MobileFrontDeskView::buildTabBar() {
    tabBar_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    tabBar_->addStyleClass("m-tab-bar");

    // Menu tab
    tabMenu_ = tabBar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabMenu_->addStyleClass("m-tab active");
    auto menuInner = tabMenu_->addWidget(std::make_unique<Wt::WContainerWidget>());
    menuInner->addStyleClass("m-tab-inner");
    menuInner->addWidget(std::make_unique<Wt::WText>(
        "<svg width='22' height='22' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'>"
        "<line x1='3' y1='6' x2='21' y2='6'/><line x1='3' y1='12' x2='21' y2='12'/>"
        "<line x1='3' y1='18' x2='21' y2='18'/></svg>"));
    menuInner->addWidget(std::make_unique<Wt::WText>("Menu"));
    tabMenu_->clicked().connect([this] {
        navigateTo(MobileScreen::Categories);
    });

    // Cart tab (with badge)
    tabCart_ = tabBar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabCart_->addStyleClass("m-tab");
    auto cartInner = tabCart_->addWidget(std::make_unique<Wt::WContainerWidget>());
    cartInner->addStyleClass("m-tab-inner");
    auto cartIconWrap = cartInner->addWidget(std::make_unique<Wt::WContainerWidget>());
    cartIconWrap->addStyleClass("m-tab-icon-wrap");
    cartIconWrap->addWidget(std::make_unique<Wt::WText>(
        "<svg width='22' height='22' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'>"
        "<circle cx='9' cy='21' r='1'/><circle cx='20' cy='21' r='1'/>"
        "<path d='M1 1h4l2.68 13.39a2 2 0 0 0 2 1.61h9.72a2 2 0 0 0 2-1.61L23 6H6'/></svg>"));
    cartBadge_ = cartIconWrap->addWidget(std::make_unique<Wt::WText>("0"));
    cartBadge_->addStyleClass("m-cart-badge");
    cartBadge_->setHidden(true);
    cartInner->addWidget(std::make_unique<Wt::WText>("Cart"));
    tabCart_->clicked().connect([this] {
        navigateTo(MobileScreen::Cart);
    });

    // Orders tab
    tabOrders_ = tabBar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabOrders_->addStyleClass("m-tab");
    auto ordersInner = tabOrders_->addWidget(std::make_unique<Wt::WContainerWidget>());
    ordersInner->addStyleClass("m-tab-inner");
    ordersInner->addWidget(std::make_unique<Wt::WText>(
        "<svg width='22' height='22' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'>"
        "<path d='M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z'/>"
        "<polyline points='14 2 14 8 20 8'/>"
        "<line x1='16' y1='13' x2='8' y2='13'/><line x1='16' y1='17' x2='8' y2='17'/>"
        "<polyline points='10 9 9 9 8 9'/></svg>"));
    ordersInner->addWidget(std::make_unique<Wt::WText>("Orders"));
    tabOrders_->clicked().connect([this] {
        navigateTo(MobileScreen::ActiveOrders);
    });
}

// ─── Navigation ──────────────────────────────────────────────────────────────

void MobileFrontDeskView::navigateTo(MobileScreen screen) {
    // Push current screen for back nav (unless it's the same)
    if (currentScreen_ != screen) {
        screenHistory_.push_back(currentScreen_);
    }
    currentScreen_ = screen;

    // Update tab bar highlights
    tabMenu_->removeStyleClass("active");
    tabCart_->removeStyleClass("active");
    tabOrders_->removeStyleClass("active");

    switch (screen) {
        case MobileScreen::Categories:
        case MobileScreen::MenuItems:
            tabMenu_->addStyleClass("active");
            break;
        case MobileScreen::Cart:
        case MobileScreen::OrderForm:
        case MobileScreen::Confirmation:
            tabCart_->addStyleClass("active");
            break;
        case MobileScreen::ActiveOrders:
            tabOrders_->addStyleClass("active");
            break;
    }

    // Show/hide floating cart bubble based on screen
    updateCartBubble();

    // Clear and rebuild screen
    screenContainer_->clear();

    switch (screen) {
        case MobileScreen::Categories:
            screenHistory_.clear(); // root screen
            buildCategoriesScreen();
            break;
        case MobileScreen::MenuItems:
            buildMenuItemsScreen();
            break;
        case MobileScreen::Cart:
            buildCartScreen();
            break;
        case MobileScreen::OrderForm:
            buildOrderFormScreen();
            break;
        case MobileScreen::Confirmation:
            buildConfirmationScreen(lastOrderId_);
            break;
        case MobileScreen::ActiveOrders:
            buildActiveOrdersScreen();
            break;
    }
}

void MobileFrontDeskView::navigateBack() {
    if (screenHistory_.empty()) {
        navigateTo(MobileScreen::Categories);
        return;
    }
    MobileScreen prev = screenHistory_.back();
    screenHistory_.pop_back();
    // Don't push to history again
    currentScreen_ = prev;
    screenHistory_.pop_back(); // navigateTo will push, so pre-pop
    navigateTo(prev);
}

// ─── Categories Screen ───────────────────────────────────────────────────────

void MobileFrontDeskView::buildCategoriesScreen() {
    // Screen header
    auto header = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("m-screen-header");
    header->addWidget(std::make_unique<Wt::WText>("Menu Categories"))
        ->addStyleClass("m-screen-title");

    auto list = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    list->addStyleClass("m-list");

    Wt::Dbo::Transaction t(api_->session());
    auto categories = api_->getCategories(restaurantId_);

    for (auto& cat : categories) {
        long long catId = cat.id();
        std::string catName = cat->name;

        // Count available items
        auto items = api_->getMenuItemsByCategory(catId);
        int availCount = 0;
        for (auto& item : items) {
            if (item->available) availCount++;
        }

        auto row = list->addWidget(std::make_unique<Wt::WContainerWidget>());
        row->addStyleClass("m-list-item");

        auto info = row->addWidget(std::make_unique<Wt::WContainerWidget>());
        info->addStyleClass("m-list-item-info");
        info->addWidget(std::make_unique<Wt::WText>(catName))
            ->addStyleClass("m-list-item-title");
        info->addWidget(std::make_unique<Wt::WText>(
            std::to_string(availCount) + " items"))
            ->addStyleClass("m-list-item-subtitle");

        row->addWidget(std::make_unique<Wt::WText>(
            "<svg width='20' height='20' viewBox='0 0 24 24' fill='none' stroke='currentColor' "
            "stroke-width='2'><polyline points='9 18 15 12 9 6'/></svg>"))
            ->addStyleClass("m-list-chevron");

        row->clicked().connect([this, catId, catName] {
            showCategoryItems(catId, catName);
        });
    }
}

void MobileFrontDeskView::showCategoryItems(
    long long categoryId, const std::string& categoryName)
{
    currentCategoryId_ = categoryId;
    currentCategoryName_ = categoryName;
    navigateTo(MobileScreen::MenuItems);
}

// ─── Menu Items Screen ───────────────────────────────────────────────────────

void MobileFrontDeskView::buildMenuItemsScreen() {
    // Screen header with back button
    auto header = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("m-screen-header");

    auto backBtn = header->addWidget(std::make_unique<Wt::WPushButton>(
        "<svg width='20' height='20' viewBox='0 0 24 24' fill='none' stroke='currentColor' "
        "stroke-width='2'><polyline points='15 18 9 12 15 6'/></svg> Back"));
    backBtn->addStyleClass("m-back-btn");
    backBtn->clicked().connect(this, &MobileFrontDeskView::navigateBack);

    header->addWidget(std::make_unique<Wt::WText>(currentCategoryName_))
        ->addStyleClass("m-screen-title");

    auto list = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    list->addStyleClass("m-items-list");

    Wt::Dbo::Transaction t(api_->session());
    auto items = api_->getMenuItemsByCategory(currentCategoryId_);

    for (auto& item : items) {
        if (!item->available) continue;

        long long itemId = item.id();
        std::string itemName = item->name;
        std::string itemDesc = item->description;
        double itemPrice = item->price;

        auto card = list->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->addStyleClass("m-item-card");

        auto info = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        info->addStyleClass("m-item-info");

        info->addWidget(std::make_unique<Wt::WText>(itemName))
            ->addStyleClass("m-item-name");
        info->addWidget(std::make_unique<Wt::WText>(itemDesc))
            ->addStyleClass("m-item-desc");

        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << itemPrice;
        info->addWidget(std::make_unique<Wt::WText>(ss.str()))
            ->addStyleClass("m-item-price");

        auto addBtn = card->addWidget(
            std::make_unique<Wt::WPushButton>("+ Add"));
        addBtn->addStyleClass("m-add-btn");
        addBtn->clicked().connect([this, itemId, itemName, itemPrice] {
            addToCart(itemId, itemName, itemPrice);
        });
    }
}

// ─── Cart Screen ─────────────────────────────────────────────────────────────

void MobileFrontDeskView::buildCartScreen() {
    auto header = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("m-screen-header");
    header->addWidget(std::make_unique<Wt::WText>("Your Cart"))
        ->addStyleClass("m-screen-title");

    if (cart_.empty()) {
        auto empty = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        empty->addStyleClass("m-empty-state");
        empty->addWidget(std::make_unique<Wt::WText>(
            "<svg width='48' height='48' viewBox='0 0 24 24' fill='none' stroke='currentColor' "
            "stroke-width='1.5' opacity='0.3'><circle cx='9' cy='21' r='1'/>"
            "<circle cx='20' cy='21' r='1'/>"
            "<path d='M1 1h4l2.68 13.39a2 2 0 0 0 2 1.61h9.72a2 2 0 0 0 2-1.61L23 6H6'/></svg>"))
            ->addStyleClass("m-empty-icon");
        empty->addWidget(std::make_unique<Wt::WText>("Your cart is empty"))
            ->addStyleClass("m-empty-title");
        empty->addWidget(std::make_unique<Wt::WText>("Browse the menu to add items"))
            ->addStyleClass("m-empty-subtitle");

        auto browseBtn = empty->addWidget(
            std::make_unique<Wt::WPushButton>("Browse Menu"));
        browseBtn->addStyleClass("m-action-btn m-btn-primary");
        browseBtn->clicked().connect([this] {
            navigateTo(MobileScreen::Categories);
        });
        return;
    }

    // Cart items list
    auto list = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    list->addStyleClass("m-cart-list");

    for (int i = 0; i < (int)cart_.size(); i++) {
        auto& ci = cart_[i];
        auto row = list->addWidget(std::make_unique<Wt::WContainerWidget>());
        row->addStyleClass("m-cart-row");

        auto info = row->addWidget(std::make_unique<Wt::WContainerWidget>());
        info->addStyleClass("m-cart-item-info");
        info->addWidget(std::make_unique<Wt::WText>(ci.name))
            ->addStyleClass("m-cart-item-name");

        std::stringstream linePrice;
        linePrice << "$" << std::fixed << std::setprecision(2)
                  << (ci.price * ci.quantity);
        info->addWidget(std::make_unique<Wt::WText>(linePrice.str()))
            ->addStyleClass("m-cart-item-price");

        // Quantity stepper
        auto stepper = row->addWidget(std::make_unique<Wt::WContainerWidget>());
        stepper->addStyleClass("m-stepper");

        auto minusBtn = stepper->addWidget(std::make_unique<Wt::WPushButton>("-"));
        minusBtn->addStyleClass("m-stepper-btn");
        minusBtn->clicked().connect([this, i] {
            if (cart_[i].quantity > 1) {
                cart_[i].quantity--;
            } else {
                removeFromCart(i);
            }
            updateCartBadge();
            screenContainer_->clear();
            buildCartScreen();
        });

        stepper->addWidget(std::make_unique<Wt::WText>(std::to_string(ci.quantity)))
            ->addStyleClass("m-stepper-value");

        auto plusBtn = stepper->addWidget(std::make_unique<Wt::WPushButton>("+"));
        plusBtn->addStyleClass("m-stepper-btn");
        plusBtn->clicked().connect([this, i] {
            cart_[i].quantity++;
            updateCartBadge();
            screenContainer_->clear();
            buildCartScreen();
        });
    }

    // Total + action buttons (sticky at bottom of scroll area)
    auto footer = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer->addStyleClass("m-cart-footer");

    auto totalRow = footer->addWidget(std::make_unique<Wt::WContainerWidget>());
    totalRow->addStyleClass("m-cart-total-row");
    totalRow->addWidget(std::make_unique<Wt::WText>("Total"))
        ->addStyleClass("m-cart-total-label");

    std::stringstream ts;
    ts << "$" << std::fixed << std::setprecision(2) << cartTotal();
    totalRow->addWidget(std::make_unique<Wt::WText>(ts.str()))
        ->addStyleClass("m-cart-total-amount");

    auto checkoutBtn = footer->addWidget(
        std::make_unique<Wt::WPushButton>("Continue to Order"));
    checkoutBtn->addStyleClass("m-action-btn m-btn-primary");
    checkoutBtn->clicked().connect([this] {
        navigateTo(MobileScreen::OrderForm);
    });

    auto clearBtn = footer->addWidget(
        std::make_unique<Wt::WPushButton>("Clear Cart"));
    clearBtn->addStyleClass("m-action-btn m-btn-secondary");
    clearBtn->clicked().connect([this] {
        cart_.clear();
        updateCartBadge();
        navigateTo(MobileScreen::Cart);
    });
}

// ─── Order Form Screen ───────────────────────────────────────────────────────

void MobileFrontDeskView::buildOrderFormScreen() {
    auto header = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("m-screen-header");

    auto backBtn = header->addWidget(std::make_unique<Wt::WPushButton>(
        "<svg width='20' height='20' viewBox='0 0 24 24' fill='none' stroke='currentColor' "
        "stroke-width='2'><polyline points='15 18 9 12 15 6'/></svg> Cart"));
    backBtn->addStyleClass("m-back-btn");
    backBtn->clicked().connect([this] { navigateTo(MobileScreen::Cart); });

    header->addWidget(std::make_unique<Wt::WText>("Order Details"))
        ->addStyleClass("m-screen-title");

    auto form = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    form->addStyleClass("m-form");

    // Customer name
    auto nameGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    nameGroup->addStyleClass("m-form-group");
    nameGroup->addWidget(std::make_unique<Wt::WText>("Customer Name"))
        ->addStyleClass("m-form-label");
    auto nameEdit = nameGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    nameEdit->setPlaceholderText("Guest name...");
    nameEdit->addStyleClass("m-form-input");
    nameEdit->setText(customerName_);

    // Table number
    auto tableGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    tableGroup->addStyleClass("m-form-group");
    tableGroup->addWidget(std::make_unique<Wt::WText>("Table Number"))
        ->addStyleClass("m-form-label");
    auto tableEdit = tableGroup->addWidget(std::make_unique<Wt::WSpinBox>());
    tableEdit->setRange(1, 50);
    tableEdit->setValue(tableNumber_);
    tableEdit->addStyleClass("m-form-input");

    // Notes
    auto notesGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    notesGroup->addStyleClass("m-form-group");
    notesGroup->addWidget(std::make_unique<Wt::WText>("Special Instructions"))
        ->addStyleClass("m-form-label");
    auto notesEdit = notesGroup->addWidget(std::make_unique<Wt::WTextArea>());
    notesEdit->setPlaceholderText("Allergies, preferences...");
    notesEdit->addStyleClass("m-form-input");
    notesEdit->setRows(3);
    notesEdit->setText(orderNotes_);

    // Order summary
    auto summary = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    summary->addStyleClass("m-order-summary");
    summary->addWidget(std::make_unique<Wt::WText>("Order Summary"))
        ->addStyleClass("m-summary-title");

    for (auto& ci : cart_) {
        auto line = summary->addWidget(std::make_unique<Wt::WContainerWidget>());
        line->addStyleClass("m-summary-line");
        line->addWidget(std::make_unique<Wt::WText>(
            std::to_string(ci.quantity) + "x " + ci.name))
            ->addStyleClass("m-summary-item");

        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << (ci.price * ci.quantity);
        line->addWidget(std::make_unique<Wt::WText>(ss.str()))
            ->addStyleClass("m-summary-price");
    }

    auto totalLine = summary->addWidget(std::make_unique<Wt::WContainerWidget>());
    totalLine->addStyleClass("m-summary-line m-summary-total");
    totalLine->addWidget(std::make_unique<Wt::WText>("Total"))
        ->addStyleClass("m-summary-item");
    std::stringstream ts;
    ts << "$" << std::fixed << std::setprecision(2) << cartTotal();
    totalLine->addWidget(std::make_unique<Wt::WText>(ts.str()))
        ->addStyleClass("m-summary-price");

    // Place order button
    auto placeBtn = screenContainer_->addWidget(
        std::make_unique<Wt::WPushButton>("Place Order"));
    placeBtn->addStyleClass("m-action-btn m-btn-success m-place-order-btn");
    placeBtn->clicked().connect([this, nameEdit, tableEdit, notesEdit] {
        customerName_ = nameEdit->text().toUTF8();
        tableNumber_ = tableEdit->value();
        orderNotes_ = notesEdit->text().toUTF8();
        submitOrder();
    });
}

// ─── Confirmation Screen ─────────────────────────────────────────────────────

void MobileFrontDeskView::buildConfirmationScreen(long long orderId) {
    auto content = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    content->addStyleClass("m-confirmation");

    content->addWidget(std::make_unique<Wt::WText>(
        "<svg width='64' height='64' viewBox='0 0 24 24' fill='none' stroke='#16a34a' "
        "stroke-width='2'><path d='M22 11.08V12a10 10 0 1 1-5.93-9.14'/>"
        "<polyline points='22 4 12 14.01 9 11.01'/></svg>"))
        ->addStyleClass("m-confirm-icon");

    content->addWidget(std::make_unique<Wt::WText>("Order Placed!"))
        ->addStyleClass("m-confirm-title");

    content->addWidget(std::make_unique<Wt::WText>(
        "Order #" + std::to_string(orderId) + " has been sent to the kitchen"))
        ->addStyleClass("m-confirm-subtitle");

    auto newBtn = content->addWidget(
        std::make_unique<Wt::WPushButton>("New Order"));
    newBtn->addStyleClass("m-action-btn m-btn-primary");
    newBtn->clicked().connect([this] {
        navigateTo(MobileScreen::Categories);
    });

    auto viewBtn = content->addWidget(
        std::make_unique<Wt::WPushButton>("View Active Orders"));
    viewBtn->addStyleClass("m-action-btn m-btn-secondary");
    viewBtn->clicked().connect([this] {
        navigateTo(MobileScreen::ActiveOrders);
    });
}

// ─── Active Orders Screen ────────────────────────────────────────────────────

void MobileFrontDeskView::buildActiveOrdersScreen() {
    auto header = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("m-screen-header");
    header->addWidget(std::make_unique<Wt::WText>("Active Orders"))
        ->addStyleClass("m-screen-title");

    auto refreshBtn = header->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
    refreshBtn->addStyleClass("m-header-action-btn");
    refreshBtn->clicked().connect([this] {
        navigateTo(MobileScreen::ActiveOrders);
    });

    Wt::Dbo::Transaction t(api_->session());
    auto orders = api_->getActiveOrders(restaurantId_);

    if (orders.empty()) {
        auto empty = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        empty->addStyleClass("m-empty-state");
        empty->addWidget(std::make_unique<Wt::WText>("No active orders"))
            ->addStyleClass("m-empty-title");
        empty->addWidget(std::make_unique<Wt::WText>("Orders will appear here once placed"))
            ->addStyleClass("m-empty-subtitle");
        return;
    }

    auto list = screenContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    list->addStyleClass("m-orders-list");

    for (auto& order : orders) {
        long long oid = order.id();
        auto card = list->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->addStyleClass("m-order-card");

        // Card header
        auto cardHeader = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        cardHeader->addStyleClass("m-order-card-header");

        auto leftInfo = cardHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
        leftInfo->addWidget(std::make_unique<Wt::WText>(
            "Order #" + std::to_string(oid)))->addStyleClass("m-order-id");
        leftInfo->addWidget(std::make_unique<Wt::WText>(
            "Table " + std::to_string(order->table_number)))->addStyleClass("m-order-table-num");

        auto statusBadge = cardHeader->addWidget(std::make_unique<Wt::WText>(order->status));
        std::string statusClass = "m-status-badge ";
        if (order->status == "Pending") statusClass += "status-pending";
        else if (order->status == "In Progress") statusClass += "status-progress";
        else if (order->status == "Ready") statusClass += "status-ready";
        statusBadge->addStyleClass(statusClass);

        // Items
        auto items = api_->getOrderItems(oid);
        auto itemsList = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        itemsList->addStyleClass("m-order-items");
        for (auto& oi : items) {
            auto line = itemsList->addWidget(std::make_unique<Wt::WContainerWidget>());
            line->addStyleClass("m-order-item-line");
            line->addWidget(std::make_unique<Wt::WText>(
                std::to_string(oi->quantity) + "x " + oi->menu_item->name));

            std::stringstream ss;
            ss << "$" << std::fixed << std::setprecision(2)
               << (oi->unit_price * oi->quantity);
            line->addWidget(std::make_unique<Wt::WText>(ss.str()))
                ->addStyleClass("m-order-item-price");
        }

        // Card footer with total
        auto cardFooter = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        cardFooter->addStyleClass("m-order-card-footer");

        std::stringstream totalSs;
        totalSs << "Total: $" << std::fixed << std::setprecision(2) << order->total;
        cardFooter->addWidget(std::make_unique<Wt::WText>(totalSs.str()))
            ->addStyleClass("m-order-total");

        if (order->status == "Ready") {
            auto serveBtn = cardFooter->addWidget(
                std::make_unique<Wt::WPushButton>("Mark Served"));
            serveBtn->addStyleClass("m-action-btn-sm m-btn-success");
            serveBtn->clicked().connect([this, oid] {
                api_->updateOrderStatus(oid, "Served");
                navigateTo(MobileScreen::ActiveOrders);
            });
        }
    }
}

// ─── Cart Operations ─────────────────────────────────────────────────────────

void MobileFrontDeskView::addToCart(
    long long menuItemId, const std::string& name, double price)
{
    for (auto& ci : cart_) {
        if (ci.menuItemId == menuItemId) {
            ci.quantity++;
            updateCartBadge();
            updateCartBubble();
            return;
        }
    }
    cart_.push_back({menuItemId, name, price, 1});
    updateCartBadge();
    updateCartBubble();
}

void MobileFrontDeskView::removeFromCart(int index) {
    if (index >= 0 && index < (int)cart_.size()) {
        cart_.erase(cart_.begin() + index);
    }
    updateCartBadge();
    updateCartBubble();
}

void MobileFrontDeskView::updateCartBadge() {
    int total = 0;
    for (auto& ci : cart_) total += ci.quantity;

    if (total > 0) {
        cartBadge_->setText(std::to_string(total));
        cartBadge_->setHidden(false);
    } else {
        cartBadge_->setHidden(true);
    }
}

void MobileFrontDeskView::updateCartBubble() {
    if (!app_) return;

    int totalItems = 0;
    for (auto& ci : cart_) totalItems += ci.quantity;

    // Update the header cart bubble with current totals
    app_->updateHeaderCart(totalItems, cartTotal());
}

double MobileFrontDeskView::cartTotal() const {
    double t = 0;
    for (auto& ci : cart_) t += ci.price * ci.quantity;
    return t;
}

// ─── Order Submission ────────────────────────────────────────────────────────

void MobileFrontDeskView::submitOrder() {
    if (cart_.empty()) return;

    std::string name = customerName_.empty() ? "Walk-In Guest" : customerName_;
    auto order = api_->createOrder(restaurantId_, tableNumber_, name, orderNotes_);
    long long orderId = order.id();

    for (auto& ci : cart_) {
        api_->addOrderItem(orderId, ci.menuItemId, ci.quantity, "");
    }

    lastOrderId_ = orderId;
    cart_.clear();
    customerName_.clear();
    tableNumber_ = 1;
    orderNotes_.clear();
    updateCartBadge();

    navigateTo(MobileScreen::Confirmation);
}
