#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>
#include <memory>

#include "../services/ApiService.h"

class KitchenView : public Wt::WContainerWidget {
public:
    KitchenView(std::shared_ptr<ApiService> api, long long restaurantId);

private:
    void buildPendingPanel(Wt::WContainerWidget* parent);
    void buildInProgressPanel(Wt::WContainerWidget* parent);
    void refreshOrders();
    void acceptOrder(long long orderId);
    void markReady(long long orderId);

    std::shared_ptr<ApiService> api_;
    long long restaurantId_;

    Wt::WContainerWidget* pendingContainer_ = nullptr;
    Wt::WContainerWidget* inProgressContainer_ = nullptr;
    Wt::WTimer* refreshTimer_ = nullptr;
};
