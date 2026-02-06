#pragma once

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <functional>
#include <memory>

#include "../services/ApiService.h"

class RestaurantApp : public Wt::WApplication {
public:
    RestaurantApp(const Wt::WEnvironment& env,
                  std::shared_ptr<ApiService> apiService);

    static std::shared_ptr<ApiService> sharedApiService;

    // Called by MobileFrontDeskView to update the header cart bubble
    void updateHeaderCart(int itemCount, double total);
    void setHeaderCartVisible(bool visible);

    // Called by MobileFrontDeskView to register itself for header cart clicks
    void setCartClickTarget(std::function<void()> callback);

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

    std::shared_ptr<ApiService> api_;
    bool isMobile_ = false;
    bool isTablet_ = false;  // tablet vs phone (for split-panel vs sequential menu)

    // JS -> C++ signal for client-side touch detection
    Wt::JSignal<std::string> touchDetected_;

    // Layout elements
    Wt::WContainerWidget* header_ = nullptr;
    Wt::WContainerWidget* workspace_ = nullptr;
    Wt::WContainerWidget* footer_ = nullptr;
    Wt::WText* headerTitle_ = nullptr;
    Wt::WText* headerUserInfo_ = nullptr;

    // Header controls (right side)
    Wt::WContainerWidget* headerControls_ = nullptr;
    Wt::WContainerWidget* headerCartBubble_ = nullptr;
    Wt::WText* headerCartCount_ = nullptr;
    Wt::WText* headerCartTotal_ = nullptr;
    Wt::WPushButton* headerLogoutBtn_ = nullptr;

    long long currentUserId_ = -1;
    long long currentRestaurantId_ = -1;
    std::string currentRole_;

    // Cart click callback (set by MobileFrontDeskView)
    std::function<void()> cartClickCallback_;
};
