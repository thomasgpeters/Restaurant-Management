#include "ManagerView.h"
#include "../ui/RestaurantApp.h"

#include <Wt/WBreak.h>
#include <Wt/WTemplate.h>
#include <sstream>
#include <iomanip>

ManagerView::ManagerView(std::shared_ptr<IApiService> api, long long restaurantId,
                         RestaurantApp* app)
    : api_(api), restaurantId_(restaurantId), app_(app)
{
    addStyleClass("manager-view");

    // Dashboard stats bar
    auto dashRow = addWidget(std::make_unique<Wt::WContainerWidget>());
    dashRow->addStyleClass("dashboard-stats");
    buildDashboard(dashRow);

    // Tab widget for main content areas
    auto tabs = addWidget(std::make_unique<Wt::WTabWidget>());
    tabs->addStyleClass("manager-tabs");

    // Orders tab
    auto ordersTab = std::make_unique<Wt::WContainerWidget>();
    ordersTab->addStyleClass("panel manager-panel");
    buildOrdersPanel(ordersTab.get());
    tabs->addTab(std::move(ordersTab), "Orders");

    // Menu Management tab
    auto menuTab = std::make_unique<Wt::WContainerWidget>();
    menuTab->addStyleClass("panel manager-panel");
    buildMenuPanel(menuTab.get());
    tabs->addTab(std::move(menuTab), "Menu");

    // Site Configuration tab
    auto configTab = std::make_unique<Wt::WContainerWidget>();
    configTab->addStyleClass("panel manager-panel");
    buildConfigPanel(configTab.get());
    tabs->addTab(std::move(configTab), "Settings");
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

    refreshDashboard();

    // Wire header refresh button (circular icon in the app header)
    if (app_) {
        app_->setRefreshClickTarget([this] {
            refreshDashboard();
            refreshOrders();
            refreshMenu();
        });
        app_->setHeaderRefreshVisible(true);
    }
}

void ManagerView::refreshDashboard() {
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
        long long oid = order.id;
        table->elementAt(row, 0)->addWidget(
            std::make_unique<Wt::WText>(std::to_string(oid)));
        table->elementAt(row, 1)->addWidget(
            std::make_unique<Wt::WText>(std::to_string(order.table_number)));
        table->elementAt(row, 2)->addWidget(
            std::make_unique<Wt::WText>(order.customer_name));

        std::stringstream ss;
        ss << "$" << std::fixed << std::setprecision(2) << order.total;
        table->elementAt(row, 3)->addWidget(std::make_unique<Wt::WText>(ss.str()));

        auto statusText = table->elementAt(row, 4)->addWidget(
            std::make_unique<Wt::WText>(order.status));
        statusText->addStyleClass("status-badge status-" +
            std::string(order.status == "Pending" ? "pending" :
                       order.status == "In Progress" ? "progress" :
                       order.status == "Ready" ? "ready" :
                       order.status == "Served" ? "served" : "cancelled"));

        if (order.status != "Served" && order.status != "Cancelled") {
            auto cancelBtn = table->elementAt(row, 5)->addWidget(
                std::make_unique<Wt::WPushButton>("Cancel"));
            cancelBtn->addStyleClass("btn btn-danger btn-sm");
            cancelBtn->clicked().connect([this, oid] {
                api_->cancelOrder(oid);
                refreshOrders();
                refreshDashboard();
            });

            if (order.status == "Ready") {
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

    auto categories = api_->getCategories(restaurantId_);

    for (auto& cat : categories) {
        auto catBlock = menuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        catBlock->addStyleClass("menu-category-block");
        catBlock->addWidget(std::make_unique<Wt::WText>(
            "<h4>" + cat.name + "</h4>"))->addStyleClass("category-title");

        auto items = api_->getMenuItemsByCategory(cat.id);
        for (auto& item : items) {
            long long itemId = item.id;
            auto itemRow = catBlock->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemRow->addStyleClass("menu-item-row");

            std::stringstream priceStr;
            priceStr << "$" << std::fixed << std::setprecision(2) << item.price;

            itemRow->addWidget(std::make_unique<Wt::WText>(item.name))
                ->addStyleClass("menu-item-name");
            itemRow->addWidget(std::make_unique<Wt::WText>(priceStr.str()))
                ->addStyleClass("menu-item-price");

            auto toggle = itemRow->addWidget(std::make_unique<Wt::WCheckBox>("Available"));
            toggle->setChecked(item.available);
            toggle->addStyleClass("menu-item-toggle");
            toggle->changed().connect([this, itemId, toggle] {
                api_->updateMenuItemAvailability(itemId, toggle->isChecked());
            });
        }
    }
}

// ─── Site Configuration Panel ─────────────────────────────────────────────────

void ManagerView::buildConfigPanel(Wt::WContainerWidget* parent) {
    parent->addWidget(std::make_unique<Wt::WText>("<h3>Site Configuration</h3>"))
        ->addStyleClass("panel-title");

    auto desc = parent->addWidget(std::make_unique<Wt::WText>(
        "<p>Configure your store branding and ApiLogicServer connection.</p>"));
    desc->addStyleClass("config-description");

    auto form = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    form->addStyleClass("config-form");

    // Store Name
    auto nameGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    nameGroup->addStyleClass("form-group");
    nameGroup->addWidget(std::make_unique<Wt::WText>("Store Name"))
        ->addStyleClass("form-label");
    configStoreName_ = nameGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    configStoreName_->setPlaceholderText("e.g. Siam Garden, Golden Dragon...");
    configStoreName_->addStyleClass("form-control");

    // Store Logo URL
    auto logoGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoGroup->addStyleClass("form-group");
    logoGroup->addWidget(std::make_unique<Wt::WText>("Store Logo URL"))
        ->addStyleClass("form-label");
    configStoreLogo_ = logoGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    configStoreLogo_->setPlaceholderText("e.g. resources/logo.png or https://...");
    configStoreLogo_->addStyleClass("form-control");

    // API Base URL
    auto apiGroup = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    apiGroup->addStyleClass("form-group");
    apiGroup->addWidget(std::make_unique<Wt::WText>("ApiLogicServer Base URL"))
        ->addStyleClass("form-label");
    configApiUrl_ = apiGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    configApiUrl_->setPlaceholderText("http://localhost:5656/api");
    configApiUrl_->addStyleClass("form-control");

    auto helpText = apiGroup->addWidget(std::make_unique<Wt::WText>(
        "<small>The REST API endpoint generated by ApiLogicServer. "
        "Example: <code>http://localhost:5656/api</code></small>"));
    helpText->addStyleClass("form-help-text");

    // Load current values from config
    if (app_ && app_->siteConfig()) {
        auto cfg = app_->siteConfig();
        configStoreName_->setText(cfg->storeName());
        configStoreLogo_->setText(cfg->storeLogo());
        configApiUrl_->setText(cfg->apiBaseUrl());
    }

    // Status message area
    configStatus_ = form->addWidget(std::make_unique<Wt::WText>(""));
    configStatus_->addStyleClass("config-status");

    // Action buttons
    auto btnRow = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    btnRow->addStyleClass("config-btn-row");

    auto saveBtn = btnRow->addWidget(std::make_unique<Wt::WPushButton>("Save Configuration"));
    saveBtn->addStyleClass("btn btn-primary");
    saveBtn->clicked().connect([this] {
        if (!app_ || !app_->siteConfig()) {
            configStatus_->setText("Error: Configuration service not available");
            configStatus_->setStyleClass("config-status config-error");
            return;
        }

        auto cfg = app_->siteConfig();
        cfg->update(
            configStoreName_->text().toUTF8(),
            configStoreLogo_->text().toUTF8(),
            configApiUrl_->text().toUTF8()
        );

        // Refresh the header branding immediately
        app_->refreshHeaderBranding();

        configStatus_->setText("Configuration saved successfully.");
        configStatus_->setStyleClass("config-status config-success");
    });

    auto resetBtn = btnRow->addWidget(std::make_unique<Wt::WPushButton>("Reset"));
    resetBtn->addStyleClass("btn btn-secondary");
    resetBtn->clicked().connect([this] {
        if (app_ && app_->siteConfig()) {
            auto cfg = app_->siteConfig();
            cfg->reload();
            configStoreName_->setText(cfg->storeName());
            configStoreLogo_->setText(cfg->storeLogo());
            configApiUrl_->setText(cfg->apiBaseUrl());
            configStatus_->setText("Values reset from saved config.");
            configStatus_->setStyleClass("config-status config-info");
        }
    });
}
