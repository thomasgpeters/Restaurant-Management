#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <memory>

#include "../services/ApiService.h"

class ManagerView : public Wt::WContainerWidget {
public:
    ManagerView(std::shared_ptr<ApiService> api, long long restaurantId);

private:
    void buildDashboard(Wt::WContainerWidget* parent);
    void buildOrdersPanel(Wt::WContainerWidget* parent);
    void buildMenuPanel(Wt::WContainerWidget* parent);
    void refreshDashboard();
    void refreshOrders();
    void refreshMenu();

    std::shared_ptr<ApiService> api_;
    long long restaurantId_;

    // Dashboard stat widgets
    Wt::WText* statTotalOrders_ = nullptr;
    Wt::WText* statRevenue_ = nullptr;
    Wt::WText* statPending_ = nullptr;
    Wt::WText* statInProgress_ = nullptr;

    Wt::WContainerWidget* ordersContainer_ = nullptr;
    Wt::WContainerWidget* menuContainer_ = nullptr;
};
