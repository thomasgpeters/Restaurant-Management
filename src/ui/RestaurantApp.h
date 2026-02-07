#pragma once

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WJavaScript.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <functional>
#include <memory>

#include "../services/IApiService.h"
#include "../services/SiteConfig.h"

class RestaurantApp : public Wt::WApplication {
public:
    RestaurantApp(const Wt::WEnvironment& env,
                  std::shared_ptr<IApiService> apiService,
                  std::shared_ptr<SiteConfig> siteConfig);

    static std::shared_ptr<IApiService> sharedApiService;
    static std::shared_ptr<SiteConfig> sharedSiteConfig;

    // Called by MobileFrontDeskView to update the header cart bubble
    void updateHeaderCart(int itemCount, double total);
    void setHeaderCartVisible(bool visible);

    // Called by MobileFrontDeskView to register itself for header cart clicks
    void setCartClickTarget(std::function<void()> callback);

    // Header refresh button (shown in Manager view)
    void setHeaderRefreshVisible(bool visible);
    void setRefreshClickTarget(std::function<void()> callback);

    // Refresh header branding from SiteConfig (called after config save)
    void refreshHeaderBranding();

    std::shared_ptr<SiteConfig> siteConfig() const { return siteConfig_; }

private:
    void setupLayout();
    void showLoginScreen();
    void showRoleView(long long userId);
    void showManagerView(long long restaurantId);
    void showFrontDeskView(long long restaurantId);
    void showKitchenView(long long restaurantId);
    void logout();

    bool detectMobileDevice();
    void onTouchDetected(const std::string& info);

    std::shared_ptr<IApiService> api_;
    std::shared_ptr<SiteConfig> siteConfig_;
    bool isMobile_ = false;
    bool isTablet_ = false;  // tablet vs phone (for split-panel vs sequential menu)

    // JS -> C++ signal for client-side touch detection
    Wt::JSignal<std::string> touchDetected_;

    // Layout elements
    Wt::WContainerWidget* header_ = nullptr;
    Wt::WContainerWidget* workspace_ = nullptr;
    Wt::WContainerWidget* footer_ = nullptr;
    Wt::WImage* headerLogo_ = nullptr;
    Wt::WText* headerTitle_ = nullptr;
    Wt::WText* headerUserInfo_ = nullptr;

    // Header controls (right side)
    Wt::WContainerWidget* headerControls_ = nullptr;
    Wt::WContainerWidget* headerCartBubble_ = nullptr;
    Wt::WText* headerCartCount_ = nullptr;
    Wt::WText* headerCartTotal_ = nullptr;
    Wt::WContainerWidget* headerThemeBtn_ = nullptr;
    Wt::WText* headerThemeIcon_ = nullptr;
    Wt::WContainerWidget* headerRefreshBtn_ = nullptr;
    Wt::WPushButton* headerLogoutBtn_ = nullptr;

    long long currentUserId_ = -1;
    long long currentRestaurantId_ = -1;
    std::string currentRole_;

    // Theme management
    Wt::JSignal<std::string> themeChanged_;
    void onThemeChanged(const std::string& theme);

    // Cart click callback (set by MobileFrontDeskView)
    std::function<void()> cartClickCallback_;
    std::function<void()> refreshClickCallback_;
};
