#include "RestaurantApp.h"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "../widgets/ManagerView.h"
#include "../widgets/FrontDeskView.h"
#include "../widgets/KitchenView.h"

std::shared_ptr<ApiService> RestaurantApp::sharedApiService = nullptr;

RestaurantApp::RestaurantApp(const Wt::WEnvironment& env,
                             std::shared_ptr<ApiService> apiService)
    : Wt::WApplication(env), api_(apiService)
{
    setTitle("Restaurant POS System");

    // Use custom stylesheet
    useStyleSheet("resources/style.css");

    setupLayout();
    showLoginScreen();
}

void RestaurantApp::setupLayout() {
    auto body = root();
    body->addStyleClass("app-body");

    // Header
    header_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
    header_->addStyleClass("app-header");

    auto headerInner = header_->addWidget(std::make_unique<Wt::WContainerWidget>());
    headerInner->addStyleClass("header-inner");

    headerTitle_ = headerInner->addWidget(std::make_unique<Wt::WText>("Restaurant POS"));
    headerTitle_->addStyleClass("header-title");

    headerUserInfo_ = headerInner->addWidget(std::make_unique<Wt::WText>(""));
    headerUserInfo_->addStyleClass("header-user-info");

    // Main workspace
    workspace_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
    workspace_->addStyleClass("app-workspace");

    // Footer
    footer_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer_->addStyleClass("app-footer");
    footer_->addWidget(std::make_unique<Wt::WText>(
        "Restaurant POS System &copy; 2026 | Powered by Wt C++ Framework | ApiLogicServer Integration"));
}

void RestaurantApp::showLoginScreen() {
    workspace_->clear();
    headerUserInfo_->setText("");

    auto loginPanel = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
    loginPanel->addStyleClass("login-panel");

    loginPanel->addWidget(std::make_unique<Wt::WText>("<h2>Welcome to Restaurant POS</h2>"))
        ->addStyleClass("login-title");
    loginPanel->addWidget(std::make_unique<Wt::WText>(
        "<p>Select a restaurant and role to begin</p>"))
        ->addStyleClass("login-subtitle");

    // Restaurant selection
    auto restGroup = loginPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    restGroup->addStyleClass("form-group");
    restGroup->addWidget(std::make_unique<Wt::WText>("Restaurant"));

    auto restCombo = restGroup->addWidget(std::make_unique<Wt::WComboBox>());
    restCombo->addStyleClass("form-control");

    Wt::Dbo::Transaction t(api_->session());
    auto restaurants = api_->getRestaurants();
    std::vector<long long> restIds;
    for (auto& r : restaurants) {
        restCombo->addItem(r->name + " (" + r->cuisine_type + ")");
        restIds.push_back(r.id());
    }

    // Role selection
    auto roleGroup = loginPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    roleGroup->addStyleClass("form-group");
    roleGroup->addWidget(std::make_unique<Wt::WText>("Role"));

    auto roleCombo = roleGroup->addWidget(std::make_unique<Wt::WComboBox>());
    roleCombo->addStyleClass("form-control");
    roleCombo->addItem("Restaurant Manager");
    roleCombo->addItem("Front Desk");
    roleCombo->addItem("Kitchen Operations");

    // Login button
    auto loginBtn = loginPanel->addWidget(std::make_unique<Wt::WPushButton>("Enter POS"));
    loginBtn->addStyleClass("btn btn-primary btn-lg login-btn");
    loginBtn->clicked().connect([this, restCombo, roleCombo, restIds] {
        int restIdx = restCombo->currentIndex();
        int roleIdx = roleCombo->currentIndex();

        if (restIdx < 0 || restIdx >= (int)restIds.size()) return;

        currentRestaurantId_ = restIds[restIdx];

        Wt::Dbo::Transaction t(api_->session());
        auto rest = api_->getRestaurant(currentRestaurantId_);
        std::string restName = rest->name;

        switch (roleIdx) {
            case 0:
                currentRole_ = "Manager";
                headerUserInfo_->setText(restName + " | Manager");
                showManagerView(currentRestaurantId_);
                break;
            case 1:
                currentRole_ = "Front Desk";
                headerUserInfo_->setText(restName + " | Front Desk");
                showFrontDeskView(currentRestaurantId_);
                break;
            case 2:
                currentRole_ = "Kitchen";
                headerUserInfo_->setText(restName + " | Kitchen Operations");
                showKitchenView(currentRestaurantId_);
                break;
        }
    });
}

void RestaurantApp::showManagerView(long long restaurantId) {
    workspace_->clear();

    auto topNav = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
    topNav->addStyleClass("view-nav");
    auto logoutBtn = topNav->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
    logoutBtn->addStyleClass("btn btn-secondary");
    logoutBtn->clicked().connect(this, &RestaurantApp::logout);

    workspace_->addWidget(std::make_unique<ManagerView>(api_, restaurantId));
}

void RestaurantApp::showFrontDeskView(long long restaurantId) {
    workspace_->clear();

    auto topNav = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
    topNav->addStyleClass("view-nav");
    auto logoutBtn = topNav->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
    logoutBtn->addStyleClass("btn btn-secondary");
    logoutBtn->clicked().connect(this, &RestaurantApp::logout);

    workspace_->addWidget(std::make_unique<FrontDeskView>(api_, restaurantId));
}

void RestaurantApp::showKitchenView(long long restaurantId) {
    workspace_->clear();

    auto topNav = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
    topNav->addStyleClass("view-nav");
    auto logoutBtn = topNav->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
    logoutBtn->addStyleClass("btn btn-secondary");
    logoutBtn->clicked().connect(this, &RestaurantApp::logout);

    workspace_->addWidget(std::make_unique<KitchenView>(api_, restaurantId));
}

void RestaurantApp::logout() {
    currentUserId_ = -1;
    currentRestaurantId_ = -1;
    currentRole_.clear();
    showLoginScreen();
}
