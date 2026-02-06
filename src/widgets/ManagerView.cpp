#include "ManagerView.h"

#include <Wt/WBreak.h>
#include <Wt/WTemplate.h>
#include <sstream>
#include <iomanip>

ManagerView::ManagerView(std::shared_ptr<ApiService> api, long long restaurantId)
    : api_(api), restaurantId_(restaurantId)
{
    addStyleClass("manager-view");

    // Dashboard stats bar
    auto dashRow = addWidget(std::make_unique<Wt::WContainerWidget>());
    dashRow->addStyleClass("dashboard-stats");
    buildDashboard(dashRow);

    // Two-column workspace
    auto workspace = addWidget(std::make_unique<Wt::WContainerWidget>());
    workspace->addStyleClass("split-workspace");

    auto leftPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftPanel->addStyleClass("panel left-panel");
    buildOrdersPanel(leftPanel);

    auto rightPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightPanel->addStyleClass("panel right-panel");
    buildMenuPanel(rightPanel);
}

void ManagerView::buildDashboard(Wt::WContainerWidget* parent) {
    auto makeCard = [&](const std::string& label, const std::string& cssClass) -> Wt::WText* {
        auto card = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->addStyleClass("stat-card " + cssClass);
        card->addWidget(std::make_unique<Wt::WText>(label))->addStyleClass("stat-label");
        auto val = card->addWidget(std::make_unique<Wt::WText>("--"));
        val->addStyleClass("stat-value");
        return val;
    };

    statTotalOrders_ = makeCard("Total Orders", "stat-total");
    statRevenue_     = makeCard("Revenue", "stat-revenue");
    statPending_     = makeCard("Pending", "stat-pending");
    statInProgress_  = makeCard("In Progress", "stat-progress");

    auto refreshBtn = parent->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
    refreshBtn->addStyleClass("btn btn-refresh");
    refreshBtn->clicked().connect([this] { refreshDashboard(); refreshOrders(); refreshMenu(); });

    refreshDashboard();
}

void ManagerView::refreshDashboard() {
    Wt::Dbo::Transaction t(api_->session());

    statTotalOrders_->setText(std::to_string(api_->getOrderCount(restaurantId_)));

    std::stringstream ss;
    ss << "$" << std::fixed << std::setprecision(2) << api_->getRevenue(restaurantId_);
    statRevenue_->setText(ss.str());

    statPending_->setText(std::to_string(api_->getPendingOrderCount(restaurantId_)));
    statInProgress_->setText(std::to_string(api_->getInProgressOrderCount(restaurantId_)));
}

void ManagerView::buildOrdersPanel(Wt::WContainerWidget* parent) {
    auto header = parent->addWidget(std::make_unique<Wt::WText>("<h3>Orders</h3>"));
    header->addStyleClass("panel-title");

    ordersContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    ordersContainer_->addStyleClass("orders-list");
    refreshOrders();
}

void ManagerView::refreshOrders() {
    ordersContainer_->clear();

    Wt::Dbo::Transaction t(api_->session());
    auto orders = api_->getOrders(restaurantId_);

    if (orders.empty()) {
        ordersContainer_->addWidget(std::make_unique<Wt::WText>(
            "<p class='empty-msg'>No orders yet</p>"));
        return;
    }

    auto table = ordersContainer_->addWidget(std::make_unique<Wt::WTable>());
    table->addStyleClass("data-table");
    table->setHeaderCount(1);
    table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("#"));
    table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Table"));
    table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Customer"));
    table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("Total"));
    table->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("Status"));
    table->elementAt(0, 5)->addWidget(std::make_unique<Wt::WText>("Actions"));

    int row = 1;
    for (auto& order : orders) {
        long long oid = order.id();
        table->elementAt(row, 0)->addWidget(
            std::make_unique<Wt::WText>(std::to_string(oid)));
        table->elementAt(row, 1)->addWidget(
            std::make_unique<Wt::WText>(std::to_string(order->table_number)));
        table->elementAt(row, 2)->addWidget(
            std::make_unique<Wt::WText>(order->customer_name));

        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << order->total;
        table->elementAt(row, 3)->addWidget(std::make_unique<Wt::WText>(ss.str()));

        auto statusText = table->elementAt(row, 4)->addWidget(
            std::make_unique<Wt::WText>(order->status));
        statusText->addStyleClass("status-badge status-" +
            std::string(order->status == "Pending" ? "pending" :
                       order->status == "In Progress" ? "progress" :
                       order->status == "Ready" ? "ready" :
                       order->status == "Served" ? "served" : "cancelled"));

        if (order->status != "Served" && order->status != "Cancelled") {
            auto cancelBtn = table->elementAt(row, 5)->addWidget(
                std::make_unique<Wt::WPushButton>("Cancel"));
            cancelBtn->addStyleClass("btn btn-danger btn-sm");
            cancelBtn->clicked().connect([this, oid] {
                api_->cancelOrder(oid);
                refreshOrders();
                refreshDashboard();
            });

            if (order->status == "Ready") {
                auto serveBtn = table->elementAt(row, 5)->addWidget(
                    std::make_unique<Wt::WPushButton>("Mark Served"));
                serveBtn->addStyleClass("btn btn-success btn-sm");
                serveBtn->clicked().connect([this, oid] {
                    api_->updateOrderStatus(oid, "Served");
                    refreshOrders();
                    refreshDashboard();
                });
            }
        }
        row++;
    }
}

void ManagerView::buildMenuPanel(Wt::WContainerWidget* parent) {
    auto header = parent->addWidget(std::make_unique<Wt::WText>("<h3>Menu Management</h3>"));
    header->addStyleClass("panel-title");

    menuContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    menuContainer_->addStyleClass("menu-management");
    refreshMenu();
}

void ManagerView::refreshMenu() {
    menuContainer_->clear();

    Wt::Dbo::Transaction t(api_->session());
    auto categories = api_->getCategories(restaurantId_);

    for (auto& cat : categories) {
        auto catBlock = menuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        catBlock->addStyleClass("menu-category-block");
        catBlock->addWidget(std::make_unique<Wt::WText>(
            "<h4>" + cat->name + "</h4>"))->addStyleClass("category-title");

        auto items = api_->getMenuItemsByCategory(cat.id());
        for (auto& item : items) {
            long long itemId = item.id();
            auto itemRow = catBlock->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemRow->addStyleClass("menu-item-row");

            std::stringstream priceStr;
            priceStr << "$" << std::fixed << std::setprecision(2) << item->price;

            itemRow->addWidget(std::make_unique<Wt::WText>(item->name))
                ->addStyleClass("menu-item-name");
            itemRow->addWidget(std::make_unique<Wt::WText>(priceStr.str()))
                ->addStyleClass("menu-item-price");

            auto toggle = itemRow->addWidget(std::make_unique<Wt::WCheckBox>("Available"));
            toggle->setChecked(item->available);
            toggle->addStyleClass("menu-item-toggle");
            toggle->changed().connect([this, itemId, toggle] {
                api_->updateMenuItemAvailability(itemId, toggle->isChecked());
            });
        }
    }
}
