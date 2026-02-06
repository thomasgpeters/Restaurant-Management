#include "RestaurantApp.h"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "../widgets/ManagerView.h"
#include "../widgets/FrontDeskView.h"
#include "../widgets/MobileFrontDeskView.h"
#include "../widgets/KitchenView.h"

std::shared_ptr<ApiService> RestaurantApp::sharedApiService = nullptr;

RestaurantApp::RestaurantApp(const Wt::WEnvironment& env,
                             std::shared_ptr<ApiService> apiService)
    : Wt::WApplication(env), api_(apiService)
{
    setTitle("Restaurant POS System");

    // Detect device type from User-Agent
    isMobile_ = detectMobileDevice();

    // Use custom stylesheet
    useStyleSheet("resources/style.css");

    // Add mobile class to root for CSS targeting
    if (isMobile_) {
        root()->addStyleClass("is-mobile");
    }

    setupLayout();
    showLoginScreen();
}

bool RestaurantApp::detectMobileDevice() {
    const std::string& ua = environment().userAgent();

    // iPad detection: Modern iPads send "Macintosh" UA but are touch devices;
    // older iPads send "iPad" in the UA string.
    if (ua.find("iPad") != std::string::npos)
        return true;

    // Modern iPadOS identifies as Macintosh + touch. We detect via
    // the "Macintosh" token combined with being a touch device.
    // Wt provides screen dimensions we can check as a fallback.
    if (ua.find("Macintosh") != std::string::npos) {
        // If the screen width hints at a tablet, treat it as mobile.
        // This catches modern iPadOS which masquerades as macOS Safari.
        // We'll rely on CSS media queries + JS detection as the final guard.
    }

    // Standard mobile/tablet tokens
    if (ua.find("Mobile") != std::string::npos ||
        ua.find("Android") != std::string::npos ||
        ua.find("webOS") != std::string::npos)
        return true;

    return false;
}

void RestaurantApp::setupLayout() {
    auto body = root();
    body->addStyleClass("app-body");

    // Client-side detection for modern iPadOS (reports as Macintosh + touch)
    // This adds the 'is-mobile' class via JS if touch is detected on a
    // Macintosh UA, catching iPadOS 13+ which masquerades as desktop Safari.
    doJavaScript(
        "if (navigator.userAgent.indexOf('Macintosh') !== -1 && "
        "'ontouchend' in document) {"
        "  document.body.classList.add('is-mobile');"
        "  document.body.classList.add('is-ipad');"
        "}");

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

    if (isMobile_) {
        // Mobile: compact header with logout integrated in the flow
        auto topNav = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
        topNav->addStyleClass("view-nav m-view-nav");
        auto logoutBtn = topNav->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
        logoutBtn->addStyleClass("m-logout-btn");
        logoutBtn->clicked().connect(this, &RestaurantApp::logout);

        workspace_->addWidget(
            std::make_unique<MobileFrontDeskView>(api_, restaurantId));
    } else {
        auto topNav = workspace_->addWidget(std::make_unique<Wt::WContainerWidget>());
        topNav->addStyleClass("view-nav");
        auto logoutBtn = topNav->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
        logoutBtn->addStyleClass("btn btn-secondary");
        logoutBtn->clicked().connect(this, &RestaurantApp::logout);

        workspace_->addWidget(std::make_unique<FrontDeskView>(api_, restaurantId));
    }
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
