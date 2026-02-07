#include "FrontDeskView.h"

#include <Wt/WBreak.h>
#include <Wt/WMessageBox.h>
#include <sstream>
#include <iomanip>

FrontDeskView::FrontDeskView(std::shared_ptr<IApiService> api, long long restaurantId)
    : api_(api), restaurantId_(restaurantId)
{
    addStyleClass("frontdesk-view");

    auto workspace = addWidget(std::make_unique<Wt::WContainerWidget>());
    workspace->addStyleClass("split-workspace");

    // Left panel: Menu browser
    auto leftPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftPanel->addStyleClass("panel left-panel");
    buildMenuBrowser(leftPanel);

    // Right panel: Order builder + active orders
    auto rightPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightPanel->addStyleClass("panel right-panel");
    buildOrderPanel(rightPanel);
}

void FrontDeskView::buildMenuBrowser(Wt::WContainerWidget* parent) {
    parent->addWidget(std::make_unique<Wt::WText>("<h3>Menu</h3>"))
        ->addStyleClass("panel-title");

    // Category tabs
    categoryContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    categoryContainer_->addStyleClass("category-tabs");

    // Menu items display
    menuItemsContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    menuItemsContainer_->addStyleClass("menu-items-grid");

    // Load categories
    auto categories = api_->getCategories(restaurantId_);
    bool first = true;
    for (auto& cat : categories) {
        long long catId = cat.id;
        auto btn = categoryContainer_->addWidget(
            std::make_unique<Wt::WPushButton>(cat.name));
        btn->addStyleClass("btn btn-category");
        if (first) btn->addStyleClass("active");
        btn->clicked().connect([this, catId] {
            showCategoryItems(catId);
        });
        if (first) {
            showCategoryItems(catId);
            first = false;
        }
    }
}

void FrontDeskView::showCategoryItems(long long categoryId) {
    menuItemsContainer_->clear();

    auto items = api_->getMenuItemsByCategory(categoryId);

    for (auto& item : items) {
        if (!item.available) continue;

        long long itemId = item.id;
        std::string itemName = item.name;
        double itemPrice = item.price;

        auto card = menuItemsContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->addStyleClass("menu-card");

        card->addWidget(std::make_unique<Wt::WText>(item.name))
            ->addStyleClass("menu-card-name");
        card->addWidget(std::make_unique<Wt::WText>(item.description))
            ->addStyleClass("menu-card-desc");

        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << item.price;
        card->addWidget(std::make_unique<Wt::WText>(ss.str()))
            ->addStyleClass("menu-card-price");

        auto addBtn = card->addWidget(std::make_unique<Wt::WPushButton>("+ Add"));
        addBtn->addStyleClass("btn btn-primary btn-sm");
        addBtn->clicked().connect([this, itemId, itemName, itemPrice] {
            addToCart(itemId, itemName, itemPrice);
        });
    }
}

void FrontDeskView::addToCart(long long menuItemId, const std::string& name, double price) {
    // Check if already in cart, increment quantity
    for (auto& ci : cart_) {
        if (ci.menuItemId == menuItemId) {
            ci.quantity++;
            refreshCart();
            return;
        }
    }
    cart_.push_back({menuItemId, name, price, 1});
    refreshCart();
}

void FrontDeskView::removeFromCart(int index) {
    if (index >= 0 && index < (int)cart_.size()) {
        cart_.erase(cart_.begin() + index);
    }
    refreshCart();
}

void FrontDeskView::refreshCart() {
    cartContainer_->clear();

    if (cart_.empty()) {
        cartContainer_->addWidget(std::make_unique<Wt::WText>(
            "<p class='empty-msg'>Cart is empty. Select items from the menu.</p>"));
        cartTotal_->setText("$0.00");
        return;
    }

    auto table = cartContainer_->addWidget(std::make_unique<Wt::WTable>());
    table->addStyleClass("data-table cart-table");
    table->setHeaderCount(1);
    table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Item"));
    table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Qty"));
    table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Price"));
    table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>(""));

    double total = 0.0;
    for (int i = 0; i < (int)cart_.size(); i++) {
        int row = i + 1;
        auto& ci = cart_[i];
        table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(ci.name));

        // Quantity controls
        auto qtyContainer = table->elementAt(row, 1)->addWidget(
            std::make_unique<Wt::WContainerWidget>());
        qtyContainer->addStyleClass("qty-controls");

        auto minusBtn = qtyContainer->addWidget(std::make_unique<Wt::WPushButton>("-"));
        minusBtn->addStyleClass("btn btn-sm btn-qty");
        minusBtn->clicked().connect([this, i] {
            if (cart_[i].quantity > 1) {
                cart_[i].quantity--;
                refreshCart();
            }
        });

        qtyContainer->addWidget(std::make_unique<Wt::WText>(
            " " + std::to_string(ci.quantity) + " "));

        auto plusBtn = qtyContainer->addWidget(std::make_unique<Wt::WPushButton>("+"));
        plusBtn->addStyleClass("btn btn-sm btn-qty");
        plusBtn->clicked().connect([this, i] {
            cart_[i].quantity++;
            refreshCart();
        });

        double lineTotal = ci.price * ci.quantity;
        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << lineTotal;
        table->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(ss.str()));

        auto rmBtn = table->elementAt(row, 3)->addWidget(
            std::make_unique<Wt::WPushButton>("X"));
        rmBtn->addStyleClass("btn btn-danger btn-sm");
        rmBtn->clicked().connect([this, i] { removeFromCart(i); });

        total += lineTotal;
    }

    std::stringstream ts;
    ts << "$" << std::fixed << std::setprecision(2) << total;
    cartTotal_->setText(ts.str());
}

void FrontDeskView::buildOrderPanel(Wt::WContainerWidget* parent) {
    parent->addWidget(std::make_unique<Wt::WText>("<h3>New Order</h3>"))
        ->addStyleClass("panel-title");

    // Customer info
    auto formRow = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    formRow->addStyleClass("form-row");

    auto nameGroup = formRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    nameGroup->addStyleClass("form-group");
    nameGroup->addWidget(std::make_unique<Wt::WText>("Customer Name"));
    customerNameEdit_ = nameGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    customerNameEdit_->setPlaceholderText("Guest name...");
    customerNameEdit_->addStyleClass("form-control");

    auto tableGroup = formRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    tableGroup->addStyleClass("form-group");
    tableGroup->addWidget(std::make_unique<Wt::WText>("Table #"));
    tableNumberEdit_ = tableGroup->addWidget(std::make_unique<Wt::WSpinBox>());
    tableNumberEdit_->setRange(1, 50);
    tableNumberEdit_->setValue(1);
    tableNumberEdit_->addStyleClass("form-control");

    auto notesGroup = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    notesGroup->addStyleClass("form-group");
    notesGroup->addWidget(std::make_unique<Wt::WText>("Notes"));
    notesEdit_ = notesGroup->addWidget(std::make_unique<Wt::WTextArea>());
    notesEdit_->setPlaceholderText("Special instructions...");
    notesEdit_->addStyleClass("form-control");
    notesEdit_->setRows(2);

    // Cart
    auto cartHeader = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    cartHeader->addStyleClass("cart-header");
    cartHeader->addWidget(std::make_unique<Wt::WText>("Cart Items"))
        ->addStyleClass("cart-title");
    cartTotal_ = cartHeader->addWidget(std::make_unique<Wt::WText>("$0.00"));
    cartTotal_->addStyleClass("cart-total-value");

    cartContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    cartContainer_->addStyleClass("cart-items");
    cartContainer_->addWidget(std::make_unique<Wt::WText>(
        "<p class='empty-msg'>Cart is empty. Select items from the menu.</p>"));

    // Submit button
    auto btnRow = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    btnRow->addStyleClass("btn-row");
    auto submitBtn = btnRow->addWidget(std::make_unique<Wt::WPushButton>("Place Order"));
    submitBtn->addStyleClass("btn btn-primary btn-lg");
    submitBtn->clicked().connect(this, &FrontDeskView::submitOrder);

    auto clearBtn = btnRow->addWidget(std::make_unique<Wt::WPushButton>("Clear Cart"));
    clearBtn->addStyleClass("btn btn-secondary btn-lg");
    clearBtn->clicked().connect([this] {
        cart_.clear();
        refreshCart();
    });

    // Active orders section
    parent->addWidget(std::make_unique<Wt::WText>("<h3>Active Orders</h3>"))
        ->addStyleClass("panel-title");
    activeOrdersContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    activeOrdersContainer_->addStyleClass("active-orders");
    refreshActiveOrders();
}

void FrontDeskView::submitOrder() {
    if (cart_.empty()) return;

    std::string custName = customerNameEdit_->text().toUTF8();
    if (custName.empty()) custName = "Walk-In Guest";
    int tableNum = tableNumberEdit_->value();
    std::string notes = notesEdit_->text().toUTF8();

    auto order = api_->createOrder(restaurantId_, tableNum, custName, notes);
    long long orderId = order.id;

    for (auto& ci : cart_) {
        api_->addOrderItem(orderId, ci.menuItemId, ci.quantity, "");
    }

    cart_.clear();
    customerNameEdit_->setText("");
    tableNumberEdit_->setValue(1);
    notesEdit_->setText("");
    refreshCart();
    refreshActiveOrders();
}

void FrontDeskView::refreshActiveOrders() {
    activeOrdersContainer_->clear();

    auto orders = api_->getActiveOrders(restaurantId_);

    if (orders.empty()) {
        activeOrdersContainer_->addWidget(std::make_unique<Wt::WText>(
            "<p class='empty-msg'>No active orders</p>"));
        return;
    }

    for (auto& order : orders) {
        long long oid = order.id;
        auto card = activeOrdersContainer_->addWidget(
            std::make_unique<Wt::WContainerWidget>());
        card->addStyleClass("order-card");

        auto headerRow = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        headerRow->addStyleClass("order-card-header");

        headerRow->addWidget(std::make_unique<Wt::WText>(
            "Order #" + std::to_string(oid)))
            ->addStyleClass("order-id");
        headerRow->addWidget(std::make_unique<Wt::WText>(
            "Table " + std::to_string(order.table_number)))
            ->addStyleClass("order-table");

        auto statusBadge = headerRow->addWidget(std::make_unique<Wt::WText>(order.status));
        statusBadge->addStyleClass("status-badge status-" +
            std::string(order.status == "Pending" ? "pending" :
                       order.status == "In Progress" ? "progress" :
                       order.status == "Ready" ? "ready" : "other"));

        // Show items
        auto items = api_->getOrderItems(oid);
        for (auto& oi : items) {
            auto itemLine = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemLine->addStyleClass("order-item-line");
            itemLine->addWidget(std::make_unique<Wt::WText>(
                std::to_string(oi.quantity) + "x " + oi.menu_item_name));

            std::stringstream ss;
            ss << "$" << std::fixed << std::setprecision(2)
               << (oi.unit_price * oi.quantity);
            itemLine->addWidget(std::make_unique<Wt::WText>(ss.str()))
                ->addStyleClass("item-price");
        }

        auto footerRow = card->addWidget(std::make_unique<Wt::WContainerWidget>());
        footerRow->addStyleClass("order-card-footer");

        std::stringstream ss;
        ss << "Total: $" << std::fixed << std::setprecision(2) << order.total;
        footerRow->addWidget(std::make_unique<Wt::WText>(ss.str()))
            ->addStyleClass("order-total");

        if (order.status == "Ready") {
            auto serveBtn = footerRow->addWidget(
                std::make_unique<Wt::WPushButton>("Mark Served"));
            serveBtn->addStyleClass("btn btn-success btn-sm");
            serveBtn->clicked().connect([this, oid] {
                api_->updateOrderStatus(oid, "Served");
                refreshActiveOrders();
            });
        }
    }
}
