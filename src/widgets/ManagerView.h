#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTabWidget.h>
#include <memory>

#include "../services/IApiService.h"

class RestaurantApp;

class ManagerView : public Wt::WContainerWidget {
public:
    ManagerView(std::shared_ptr<IApiService> api, long long restaurantId,
                RestaurantApp* app = nullptr);

private:
    void buildDashboard(Wt::WContainerWidget* parent);
    void buildOrdersPanel(Wt::WContainerWidget* parent);
    void buildMenuPanel(Wt::WContainerWidget* parent);
    void buildConfigPanel(Wt::WContainerWidget* parent);
    void refreshDashboard();
    void refreshOrders();
    void refreshMenu();

    std::shared_ptr<IApiService> api_;
    long long restaurantId_;
    RestaurantApp* app_ = nullptr;

    // Dashboard stat widgets
    Wt::WText* statTotalOrders_ = nullptr;
    Wt::WText* statRevenue_ = nullptr;
    Wt::WText* statPending_ = nullptr;
    Wt::WText* statInProgress_ = nullptr;

    Wt::WContainerWidget* ordersContainer_ = nullptr;
    Wt::WContainerWidget* menuContainer_ = nullptr;

    // Config form fields
    Wt::WLineEdit* configStoreName_ = nullptr;
    Wt::WLineEdit* configStoreLogo_ = nullptr;
    Wt::WLineEdit* configApiUrl_ = nullptr;
    Wt::WComboBox* configDataSource_ = nullptr;
    Wt::WText* configStatus_ = nullptr;
};
