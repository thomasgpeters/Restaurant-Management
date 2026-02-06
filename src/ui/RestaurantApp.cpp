#include "RestaurantApp.h"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <sstream>
#include <iomanip>

#include "../widgets/ManagerView.h"
#include "../widgets/FrontDeskView.h"
#include "../widgets/MobileFrontDeskView.h"
#include "../widgets/KitchenView.h"

std::shared_ptr<ApiService> RestaurantApp::sharedApiService = nullptr;

RestaurantApp::RestaurantApp(const Wt::WEnvironment& env,
                             std::shared_ptr<ApiService> apiService)
    : Wt::WApplication(env), api_(apiService),
      touchDetected_(this, "touchDetected")
{
    setTitle("Restaurant POS System");

    // Detect device type from User-Agent (catches most cases)
    isMobile_ = detectMobileDevice();

    // Use custom stylesheet
    useStyleSheet("resources/style.css");

    // Add mobile class to root for CSS targeting
    if (isMobile_) {
        root()->addStyleClass("is-mobile");
    }

    // Connect JS->C++ signal for client-side touch detection callback
    touchDetected_.connect(this, &RestaurantApp::onTouchDetected);

    setupLayout();
    showLoginScreen();
}

bool RestaurantApp::detectMobileDevice() {
    const std::string& ua = environment().userAgent();

    // iPad: older iPads send "iPad"; modern iPadOS sends "Macintosh"
    if (ua.find("iPad") != std::string::npos) {
        isTablet_ = true;
        return true;
    }

    // "Tablet" token → definitely a tablet
    if (ua.find("Tablet") != std::string::npos) {
        isTablet_ = true;
        return true;
    }

    // Silk browser (Amazon Fire tablets)
    if (ua.find("Silk") != std::string::npos) {
        isTablet_ = true;
        return true;
    }

    // Android: "Mobile" present → phone, absent → tablet
    if (ua.find("Android") != std::string::npos) {
        isTablet_ = (ua.find("Mobile") == std::string::npos);
        return true;
    }

    // Samsung devices: SM-T = tablet, SM-G/SM-A/SM-S = phone (usually)
    if (ua.find("SM-") != std::string::npos) {
        isTablet_ = (ua.find("SM-T") != std::string::npos);
        return true;
    }

    // Generic mobile tokens (phones)
    if (ua.find("Mobile") != std::string::npos ||
        ua.find("webOS") != std::string::npos ||
        ua.find("Opera Mini") != std::string::npos ||
        ua.find("Opera Mobi") != std::string::npos) {
        isTablet_ = false;
        return true;
    }

    return false;
}

void RestaurantApp::onTouchDetected(const std::string& info) {
    // Called from client-side JS when a touch device is detected that
    // the server-side UA parsing missed (e.g. Samsung desktop mode,
    // modern iPadOS). Only act if we haven't already detected mobile.
    // JS detection uses screen >= 600px, so this is always a tablet.
    if (!isMobile_) {
        isMobile_ = true;
        isTablet_ = true;
        root()->addStyleClass("is-mobile");
        if (info.find("ipad") != std::string::npos) {
            root()->addStyleClass("is-ipad");
        }
        // Re-render the login screen so the user gets mobile routing
        if (currentRole_.empty()) {
            showLoginScreen();
        }
    }
}

void RestaurantApp::setupLayout() {
    auto body = root();
    body->addStyleClass("app-body");

    // Client-side touch detection: catches tablets in desktop mode
    // (Samsung desktop mode, modern iPadOS reporting as Macintosh, etc.)
    // Uses touch capability + screen size to determine if this is a tablet.
    // Fires a JSignal back to the server so we can switch to mobile views.
    //
    // The detection retries until Wt's JS framework is loaded (Wt.emit),
    // and also hooks a touchstart listener as a fallback trigger.
    std::string signalCall = touchDetected_.createCall({"info"});
    doJavaScript(
        "(function(){"
        "  var fired = false;"
        "  function detect(){"
        "    var isTouch = ('ontouchstart' in window) || "
        "      (navigator.maxTouchPoints > 0) || "
        "      (navigator.msMaxTouchPoints > 0);"
        "    if (!isTouch) return false;"
        "    var sw = Math.min(screen.width, screen.height);"
        "    var ww = Math.min(window.innerWidth, window.innerHeight);"
        "    var w = Math.max(sw, ww);"
        "    return (w >= 600);"  // tablet-sized screen
        "  }"
        "  function fire(){"
        "    if (fired) return;"
        "    if (!detect()) return;"
        "    if (typeof Wt === 'undefined' || typeof Wt.emit !== 'function'){"
        "      setTimeout(fire, 150);"
        "      return;"
        "    }"
        "    fired = true;"
        "    document.body.classList.add('is-mobile');"
        "    var info = 'touch';"
        "    if (navigator.userAgent.indexOf('Macintosh') !== -1){"
        "      document.body.classList.add('is-ipad');"
        "      info = 'ipad';"
        "    }"
        "    " + signalCall + ";"
        "  }"
        // Try immediately, then retry with delays
        "  fire();"
        "  setTimeout(fire, 300);"
        "  setTimeout(fire, 800);"
        "  setTimeout(fire, 2000);"
        // Also trigger on first touch as ultimate fallback
        "  document.addEventListener('touchstart', function ts(){"
        "    document.removeEventListener('touchstart', ts);"
        "    fire();"
        "  }, {passive:true});"
        "})();");

    // Header
    header_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
    header_->addStyleClass("app-header");

    auto headerInner = header_->addWidget(std::make_unique<Wt::WContainerWidget>());
    headerInner->addStyleClass("header-inner");

    headerTitle_ = headerInner->addWidget(std::make_unique<Wt::WText>("Restaurant POS"));
    headerTitle_->addStyleClass("header-title");

    // Right-side controls group: [cart bubble] [logout] [user info]
    headerControls_ = headerInner->addWidget(std::make_unique<Wt::WContainerWidget>());
    headerControls_->addStyleClass("header-controls");

    // Cart bubble in header (hidden by default, shown during Front Desk)
    headerCartBubble_ = headerControls_->addWidget(std::make_unique<Wt::WContainerWidget>());
    headerCartBubble_->addStyleClass("header-cart-bubble");
    headerCartBubble_->setHidden(true);

    headerCartBubble_->addWidget(std::make_unique<Wt::WText>(
        "<svg width='18' height='18' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'>"
        "<circle cx='9' cy='21' r='1'/><circle cx='20' cy='21' r='1'/>"
        "<path d='M1 1h4l2.68 13.39a2 2 0 0 0 2 1.61h9.72a2 2 0 0 0 2-1.61L23 6H6'/></svg>"))
        ->addStyleClass("header-cart-icon");

    headerCartCount_ = headerCartBubble_->addWidget(std::make_unique<Wt::WText>("0"));
    headerCartCount_->addStyleClass("header-cart-count");

    headerCartTotal_ = headerCartBubble_->addWidget(std::make_unique<Wt::WText>("$0.00"));
    headerCartTotal_->addStyleClass("header-cart-total");

    headerCartBubble_->clicked().connect([this] {
        if (cartClickCallback_) cartClickCallback_();
    });

    // Logout button in header (hidden until logged in)
    headerLogoutBtn_ = headerControls_->addWidget(std::make_unique<Wt::WPushButton>("Logout"));
    headerLogoutBtn_->addStyleClass("header-logout-btn");
    headerLogoutBtn_->setHidden(true);
    headerLogoutBtn_->clicked().connect(this, &RestaurantApp::logout);

    // User info pill
    headerUserInfo_ = headerControls_->addWidget(std::make_unique<Wt::WText>(""));
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
    workspace_->addStyleClass("login-mode");
    headerUserInfo_->setText("");
    headerLogoutBtn_->setHidden(true);
    headerCartBubble_->setHidden(true);

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
    workspace_->removeStyleClass("login-mode");
    headerLogoutBtn_->setHidden(false);
    headerCartBubble_->setHidden(true);

    workspace_->addWidget(std::make_unique<ManagerView>(api_, restaurantId));
}

void RestaurantApp::showFrontDeskView(long long restaurantId) {
    workspace_->clear();
    workspace_->removeStyleClass("login-mode");
    headerLogoutBtn_->setHidden(false);
    headerCartBubble_->setHidden(true);

    if (isMobile_) {
        workspace_->addWidget(
            std::make_unique<MobileFrontDeskView>(api_, restaurantId, this, isTablet_));
    } else {
        workspace_->addWidget(std::make_unique<FrontDeskView>(api_, restaurantId));
    }
}

void RestaurantApp::showKitchenView(long long restaurantId) {
    workspace_->clear();
    workspace_->removeStyleClass("login-mode");
    headerLogoutBtn_->setHidden(false);
    headerCartBubble_->setHidden(true);

    workspace_->addWidget(std::make_unique<KitchenView>(api_, restaurantId));
}

void RestaurantApp::logout() {
    currentUserId_ = -1;
    currentRestaurantId_ = -1;
    currentRole_.clear();
    showLoginScreen();
}

void RestaurantApp::updateHeaderCart(int itemCount, double total) {
    if (itemCount > 0) {
        headerCartCount_->setText(std::to_string(itemCount));
        std::stringstream ts;
        ts << "$" << std::fixed << std::setprecision(2) << total;
        headerCartTotal_->setText(ts.str());
        headerCartBubble_->setHidden(false);
        // Trigger bounce animation
        doJavaScript(
            "var b = document.querySelector('.header-cart-bubble');"
            "if(b){b.classList.remove('bounce');"
            "void b.offsetWidth; b.classList.add('bounce');}");
    } else {
        headerCartBubble_->setHidden(true);
    }
}

void RestaurantApp::setHeaderCartVisible(bool visible) {
    headerCartBubble_->setHidden(!visible);
}

void RestaurantApp::setCartClickTarget(std::function<void()> callback) {
    cartClickCallback_ = std::move(callback);
}
