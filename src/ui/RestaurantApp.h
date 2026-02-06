#pragma once

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <memory>

#include "../services/ApiService.h"

class RestaurantApp : public Wt::WApplication {
public:
    RestaurantApp(const Wt::WEnvironment& env,
                  std::shared_ptr<ApiService> apiService);

    static std::shared_ptr<ApiService> sharedApiService;

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

    // JS -> C++ signal for client-side touch detection
    Wt::JSignal<std::string> touchDetected_;

    // Layout elements
    Wt::WContainerWidget* header_ = nullptr;
    Wt::WContainerWidget* workspace_ = nullptr;
    Wt::WContainerWidget* footer_ = nullptr;
    Wt::WText* headerTitle_ = nullptr;
    Wt::WText* headerUserInfo_ = nullptr;

    long long currentUserId_ = -1;
    long long currentRestaurantId_ = -1;
    std::string currentRole_;
};
