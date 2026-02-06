#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WTextArea.h>
#include <memory>
#include <vector>

#include "../services/ApiService.h"

struct CartItem {
    long long menuItemId;
    std::string name;
    double price;
    int quantity;
};

class FrontDeskView : public Wt::WContainerWidget {
public:
    FrontDeskView(std::shared_ptr<ApiService> api, long long restaurantId);

private:
    void buildMenuBrowser(Wt::WContainerWidget* parent);
    void buildOrderPanel(Wt::WContainerWidget* parent);
    void showCategoryItems(long long categoryId);
    void addToCart(long long menuItemId, const std::string& name, double price);
    void removeFromCart(int index);
    void refreshCart();
    void submitOrder();
    void refreshActiveOrders();

    std::shared_ptr<ApiService> api_;
    long long restaurantId_;

    // Menu browsing
    Wt::WContainerWidget* categoryContainer_ = nullptr;
    Wt::WContainerWidget* menuItemsContainer_ = nullptr;

    // Order panel
    Wt::WContainerWidget* cartContainer_ = nullptr;
    Wt::WText* cartTotal_ = nullptr;
    Wt::WLineEdit* customerNameEdit_ = nullptr;
    Wt::WSpinBox* tableNumberEdit_ = nullptr;
    Wt::WTextArea* notesEdit_ = nullptr;
    Wt::WContainerWidget* activeOrdersContainer_ = nullptr;

    std::vector<CartItem> cart_;
};
