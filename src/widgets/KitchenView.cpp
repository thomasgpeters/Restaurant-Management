#include "KitchenView.h"

#include <Wt/WBreak.h>
#include <sstream>
#include <iomanip>

KitchenView::KitchenView(std::shared_ptr<IApiService> api, long long restaurantId)
    : api_(api), restaurantId_(restaurantId)
{
    addStyleClass("kitchen-view");

    // Auto-refresh header
    auto topBar = addWidget(std::make_unique<Wt::WContainerWidget>());
    topBar->addStyleClass("kitchen-topbar");
    topBar->addWidget(std::make_unique<Wt::WText>("Kitchen Display"))
        ->addStyleClass("kitchen-title");

    auto refreshBtn = topBar->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
    refreshBtn->addStyleClass("btn btn-refresh");
    refreshBtn->clicked().connect(this, &KitchenView::refreshOrders);

    // Two-column: Pending | In Progress
    auto workspace = addWidget(std::make_unique<Wt::WContainerWidget>());
    workspace->addStyleClass("split-workspace");

    auto leftPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftPanel->addStyleClass("panel left-panel kitchen-pending");
    buildPendingPanel(leftPanel);

    auto rightPanel = workspace->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightPanel->addStyleClass("panel right-panel kitchen-inprogress");
    buildInProgressPanel(rightPanel);

    // Initial data load (both containers must exist before calling refresh)
    refreshOrders();

    // Auto-refresh timer (every 10 seconds)
    refreshTimer_ = addChild(std::make_unique<Wt::WTimer>());
    refreshTimer_->setInterval(std::chrono::seconds(10));
    refreshTimer_->timeout().connect(this, &KitchenView::refreshOrders);
    refreshTimer_->start();
}

void KitchenView::buildPendingPanel(Wt::WContainerWidget* parent) {
    parent->addWidget(std::make_unique<Wt::WText>("<h3>Pending Orders</h3>"))
        ->addStyleClass("panel-title");
    pendingContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    pendingContainer_->addStyleClass("kitchen-orders");
}

void KitchenView::buildInProgressPanel(Wt::WContainerWidget* parent) {
    parent->addWidget(std::make_unique<Wt::WText>("<h3>In Progress</h3>"))
        ->addStyleClass("panel-title");
    inProgressContainer_ = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    inProgressContainer_->addStyleClass("kitchen-orders");
}

void KitchenView::refreshOrders() {
    // Refresh pending
    pendingContainer_->clear();
    {
        auto pending = api_->getOrdersByStatus(restaurantId_, "Pending");

        if (pending.empty()) {
            pendingContainer_->addWidget(std::make_unique<Wt::WText>(
                "<p class='empty-msg'>No pending orders</p>"));
        }

        for (auto& order : pending) {
            long long oid = order.id;
            auto card = pendingContainer_->addWidget(
                std::make_unique<Wt::WContainerWidget>());
            card->addStyleClass("kitchen-card pending-card");

            auto hdr = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            hdr->addStyleClass("kitchen-card-header");
            hdr->addWidget(std::make_unique<Wt::WText>(
                "Order #" + std::to_string(oid)))->addStyleClass("order-id");
            hdr->addWidget(std::make_unique<Wt::WText>(
                "Table " + std::to_string(order.table_number)))->addStyleClass("order-table");

            if (!order.notes.empty()) {
                auto notesEl = card->addWidget(std::make_unique<Wt::WText>(
                    "Notes: " + order.notes));
                notesEl->addStyleClass("order-notes");
            }

            auto items = api_->getOrderItems(oid);
            auto itemsList = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemsList->addStyleClass("kitchen-items");
            for (auto& oi : items) {
                auto line = itemsList->addWidget(std::make_unique<Wt::WContainerWidget>());
                line->addStyleClass("kitchen-item-line");
                line->addWidget(std::make_unique<Wt::WText>(
                    std::to_string(oi.quantity) + "x "));
                line->addWidget(std::make_unique<Wt::WText>(oi.menu_item_name))
                    ->addStyleClass("item-name-bold");
                if (!oi.special_instructions.empty()) {
                    line->addWidget(std::make_unique<Wt::WText>(
                        " (" + oi.special_instructions + ")"))
                        ->addStyleClass("item-instructions");
                }
            }

            auto acceptBtn = card->addWidget(
                std::make_unique<Wt::WPushButton>("Accept Order"));
            acceptBtn->addStyleClass("btn btn-primary btn-block");
            acceptBtn->clicked().connect([this, oid] { acceptOrder(oid); });
        }
    }

    // Refresh in-progress
    inProgressContainer_->clear();
    {
        auto inProgress = api_->getOrdersByStatus(restaurantId_, "In Progress");

        if (inProgress.empty()) {
            inProgressContainer_->addWidget(std::make_unique<Wt::WText>(
                "<p class='empty-msg'>No orders in progress</p>"));
        }

        for (auto& order : inProgress) {
            long long oid = order.id;
            auto card = inProgressContainer_->addWidget(
                std::make_unique<Wt::WContainerWidget>());
            card->addStyleClass("kitchen-card progress-card");

            auto hdr = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            hdr->addStyleClass("kitchen-card-header");
            hdr->addWidget(std::make_unique<Wt::WText>(
                "Order #" + std::to_string(oid)))->addStyleClass("order-id");
            hdr->addWidget(std::make_unique<Wt::WText>(
                "Table " + std::to_string(order.table_number)))->addStyleClass("order-table");

            auto items = api_->getOrderItems(oid);
            auto itemsList = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemsList->addStyleClass("kitchen-items");
            for (auto& oi : items) {
                auto line = itemsList->addWidget(std::make_unique<Wt::WContainerWidget>());
                line->addStyleClass("kitchen-item-line");
                line->addWidget(std::make_unique<Wt::WText>(
                    std::to_string(oi.quantity) + "x "));
                line->addWidget(std::make_unique<Wt::WText>(oi.menu_item_name))
                    ->addStyleClass("item-name-bold");
            }

            auto readyBtn = card->addWidget(
                std::make_unique<Wt::WPushButton>("Mark Ready"));
            readyBtn->addStyleClass("btn btn-success btn-block");
            readyBtn->clicked().connect([this, oid] { markReady(oid); });
        }
    }
}

void KitchenView::acceptOrder(long long orderId) {
    api_->updateOrderStatus(orderId, "In Progress");
    refreshOrders();
}

void KitchenView::markReady(long long orderId) {
    api_->updateOrderStatus(orderId, "Ready");
    refreshOrders();
}
