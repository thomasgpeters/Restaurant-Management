#include "RestApiService.h"

#include <Wt/Json/Parser.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>

#include <curl/curl.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iomanip>

// ─── libcurl helpers ─────────────────────────────────────────────────────────

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

RestApiService::RestApiService(std::shared_ptr<SiteConfig> config)
    : config_(std::move(config))
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

RestApiService::~RestApiService() {
    curl_global_cleanup();
}

std::string RestApiService::baseUrl() const {
    std::string url = config_->apiBaseUrl();
    // Strip trailing slash
    while (!url.empty() && url.back() == '/') url.pop_back();
    return url;
}

std::string RestApiService::urlEncode(const std::string& value) const {
    CURL* curl = curl_easy_init();
    if (!curl) return value;
    char* encoded = curl_easy_escape(curl, value.c_str(), (int)value.size());
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

std::string RestApiService::httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
    headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP GET failed: ") + curl_easy_strerror(res));
    }
    if (httpCode >= 400) {
        std::cerr << "[RestApi] GET " << url << " → " << httpCode << std::endl;
    }
    return response;
}

std::string RestApiService::httpPost(const std::string& url, const std::string& jsonBody) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
    headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP POST failed: ") + curl_easy_strerror(res));
    }
    if (httpCode >= 400) {
        std::cerr << "[RestApi] POST " << url << " → " << httpCode << "\n" << response << std::endl;
    }
    return response;
}

std::string RestApiService::httpPatch(const std::string& url, const std::string& jsonBody) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
    headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP PATCH failed: ") + curl_easy_strerror(res));
    }
    if (httpCode >= 400) {
        std::cerr << "[RestApi] PATCH " << url << " → " << httpCode << "\n" << response << std::endl;
    }
    return response;
}

// ─── JSON:API response parsing helpers ───────────────────────────────────────
// ApiLogicServer returns JSON:API format:
//   { "data": [ { "type": "...", "id": "...", "attributes": { ... } }, ... ] }
// or for single resources:
//   { "data": { "type": "...", "id": "...", "attributes": { ... } } }

static long long jsonId(const Wt::Json::Object& resource) {
    // JSON:API id is a string
    if (resource.contains("id")) {
        auto& val = resource.get("id");
        if (val.type() == Wt::Json::Type::String) {
            return std::stoll(static_cast<const Wt::WString&>(val).toUTF8());
        }
        if (val.type() == Wt::Json::Type::Number) {
            return static_cast<long long>(static_cast<double>(val));
        }
    }
    return 0;
}

static std::string jsonStr(const Wt::Json::Object& obj, const std::string& key) {
    if (!obj.contains(key)) return "";
    auto& val = obj.get(key);
    if (val.type() == Wt::Json::Type::String)
        return static_cast<const Wt::WString&>(val).toUTF8();
    if (val.isNull()) return "";
    return "";
}

static double jsonNum(const Wt::Json::Object& obj, const std::string& key) {
    if (!obj.contains(key)) return 0.0;
    auto& val = obj.get(key);
    if (val.type() == Wt::Json::Type::Number) return static_cast<double>(val);
    if (val.type() == Wt::Json::Type::String) {
        try { return std::stod(static_cast<const Wt::WString&>(val).toUTF8()); }
        catch (...) { return 0.0; }
    }
    return 0.0;
}

static int jsonInt(const Wt::Json::Object& obj, const std::string& key) {
    return static_cast<int>(std::round(jsonNum(obj, key)));
}

static bool jsonBool(const Wt::Json::Object& obj, const std::string& key) {
    if (!obj.contains(key)) return false;
    auto& val = obj.get(key);
    if (val.type() == Wt::Json::Type::Bool) return static_cast<bool>(val);
    return false;
}

static long long jsonFk(const Wt::Json::Object& attrs, const std::string& key) {
    return static_cast<long long>(std::round(jsonNum(attrs, key)));
}

// Parse a JSON:API resource object into DTOs

static RestaurantDto parseRestaurant(const Wt::Json::Object& res) {
    RestaurantDto d;
    d.id = jsonId(res);
    const Wt::Json::Object& a = res.get("attributes");
    d.name         = jsonStr(a, "name");
    d.cuisine_type = jsonStr(a, "cuisine_type");
    d.description  = jsonStr(a, "description");
    return d;
}

static CategoryDto parseCategory(const Wt::Json::Object& res) {
    CategoryDto d;
    d.id = jsonId(res);
    const Wt::Json::Object& a = res.get("attributes");
    d.name          = jsonStr(a, "name");
    d.sort_order    = jsonInt(a, "sort_order");
    d.restaurant_id = jsonFk(a, "restaurant_id");
    return d;
}

static MenuItemDto parseMenuItem(const Wt::Json::Object& res) {
    MenuItemDto d;
    d.id = jsonId(res);
    const Wt::Json::Object& a = res.get("attributes");
    d.name        = jsonStr(a, "name");
    d.description = jsonStr(a, "description");
    d.price       = jsonNum(a, "price");
    d.available   = jsonBool(a, "available");
    d.category_id = jsonFk(a, "category_id");
    return d;
}

static OrderDto parseOrder(const Wt::Json::Object& res) {
    OrderDto d;
    d.id = jsonId(res);
    const Wt::Json::Object& a = res.get("attributes");
    d.table_number  = jsonInt(a, "table_number");
    d.status        = jsonStr(a, "status");
    d.customer_name = jsonStr(a, "customer_name");
    d.notes         = jsonStr(a, "notes");
    d.total         = jsonNum(a, "total");
    d.restaurant_id = jsonFk(a, "restaurant_id");
    d.created_at    = jsonStr(a, "created_at");
    return d;
}

static OrderItemDto parseOrderItem(const Wt::Json::Object& res) {
    OrderItemDto d;
    d.id = jsonId(res);
    const Wt::Json::Object& a = res.get("attributes");
    d.quantity             = jsonInt(a, "quantity");
    d.unit_price           = jsonNum(a, "unit_price");
    d.special_instructions = jsonStr(a, "special_instructions");
    d.order_id             = jsonFk(a, "order_id");
    d.menu_item_id         = jsonFk(a, "menu_item_id");
    // menu_item_name may come from include; caller resolves it
    return d;
}

// Parse a JSON:API collection response → vector of Json::Object resources
static Wt::Json::Array parseDataArray(const std::string& json) {
    Wt::Json::Object root;
    Wt::Json::parse(json, root);
    if (root.contains("data")) {
        auto& data = root.get("data");
        if (data.type() == Wt::Json::Type::Array)
            return static_cast<const Wt::Json::Array&>(data);
    }
    return Wt::Json::Array();
}

// Parse a single JSON:API resource → Json::Object
static Wt::Json::Object parseDataObject(const std::string& json) {
    Wt::Json::Object root;
    Wt::Json::parse(json, root);
    if (root.contains("data")) {
        auto& data = root.get("data");
        if (data.type() == Wt::Json::Type::Object)
            return static_cast<const Wt::Json::Object&>(data);
    }
    return Wt::Json::Object();
}

// Build an "included" lookup: type+id → Object
static std::map<std::string, Wt::Json::Object>
buildIncludedMap(const std::string& json) {
    std::map<std::string, Wt::Json::Object> m;
    Wt::Json::Object root;
    Wt::Json::parse(json, root);
    if (root.contains("included")) {
        auto& inc = root.get("included");
        if (inc.type() == Wt::Json::Type::Array) {
            const Wt::Json::Array& arr = inc;
            for (int i = 0; i < (int)arr.size(); i++) {
                const Wt::Json::Object& obj = arr[i];
                std::string key = jsonStr(obj, "type") + ":" +
                    std::to_string(jsonId(obj));
                m[key] = obj;
            }
        }
    }
    return m;
}

// ─── IApiService implementation ──────────────────────────────────────────────

std::vector<RestaurantDto> RestApiService::getRestaurants() {
    auto json = httpGet(baseUrl() + "/restaurant/");
    auto arr = parseDataArray(json);
    std::vector<RestaurantDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        out.push_back(parseRestaurant(obj));
    }
    return out;
}

RestaurantDto RestApiService::getRestaurant(long long id) {
    auto json = httpGet(baseUrl() + "/restaurant/" + std::to_string(id) + "/");
    return parseRestaurant(parseDataObject(json));
}

std::vector<CategoryDto> RestApiService::getCategories(long long restaurantId) {
    auto json = httpGet(baseUrl() + "/category/?filter%5Brestaurant_id%5D=" +
                        std::to_string(restaurantId) + "&sort=sort_order");
    auto arr = parseDataArray(json);
    std::vector<CategoryDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        out.push_back(parseCategory(obj));
    }
    return out;
}

std::vector<MenuItemDto> RestApiService::getMenuItemsByCategory(long long categoryId) {
    auto json = httpGet(baseUrl() + "/menu_item/?filter%5Bcategory_id%5D=" +
                        std::to_string(categoryId));
    auto arr = parseDataArray(json);
    std::vector<MenuItemDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        out.push_back(parseMenuItem(obj));
    }
    return out;
}

std::vector<MenuItemDto> RestApiService::getMenuItemsByRestaurant(long long restaurantId) {
    // ALS: filter menu_items by category's restaurant
    // This requires a join filter; use include or separate calls
    auto cats = getCategories(restaurantId);
    std::vector<MenuItemDto> all;
    for (auto& cat : cats) {
        auto items = getMenuItemsByCategory(cat.id);
        all.insert(all.end(), items.begin(), items.end());
    }
    return all;
}

MenuItemDto RestApiService::getMenuItem(long long id) {
    auto json = httpGet(baseUrl() + "/menu_item/" + std::to_string(id) + "/");
    return parseMenuItem(parseDataObject(json));
}

void RestApiService::updateMenuItemAvailability(long long id, bool available) {
    std::string body = "{\"data\":{\"type\":\"menu_item\",\"id\":\"" +
        std::to_string(id) + "\",\"attributes\":{\"available\":" +
        (available ? "true" : "false") + "}}}";
    httpPatch(baseUrl() + "/menu_item/" + std::to_string(id) + "/", body);
}

std::vector<OrderDto> RestApiService::getOrders(long long restaurantId) {
    auto json = httpGet(baseUrl() + "/orders/?filter%5Brestaurant_id%5D=" +
                        std::to_string(restaurantId) + "&sort=-id");
    auto arr = parseDataArray(json);
    std::vector<OrderDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        out.push_back(parseOrder(obj));
    }
    return out;
}

std::vector<OrderDto> RestApiService::getOrdersByStatus(
    long long restaurantId, const std::string& status)
{
    auto json = httpGet(baseUrl() + "/orders/?filter%5Brestaurant_id%5D=" +
                        std::to_string(restaurantId) +
                        "&filter%5Bstatus%5D=" + urlEncode(status) +
                        "&sort=id");
    auto arr = parseDataArray(json);
    std::vector<OrderDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        out.push_back(parseOrder(obj));
    }
    return out;
}

std::vector<OrderDto> RestApiService::getActiveOrders(long long restaurantId) {
    // JSON:API doesn't have != filter natively; fetch all and client-filter
    auto all = getOrders(restaurantId);
    std::vector<OrderDto> out;
    for (auto& o : all) {
        if (o.status != "Served" && o.status != "Cancelled") {
            out.push_back(o);
        }
    }
    // Sort by id ascending (getOrders returns DESC)
    std::sort(out.begin(), out.end(),
              [](const OrderDto& a, const OrderDto& b) { return a.id < b.id; });
    return out;
}

OrderDto RestApiService::getOrder(long long id) {
    auto json = httpGet(baseUrl() + "/orders/" + std::to_string(id) + "/");
    return parseOrder(parseDataObject(json));
}

OrderDto RestApiService::createOrder(long long restaurantId, int tableNumber,
                                      const std::string& customerName,
                                      const std::string& notes)
{
    std::stringstream body;
    body << "{\"data\":{\"type\":\"orders\",\"attributes\":{"
         << "\"restaurant_id\":" << restaurantId << ","
         << "\"table_number\":" << tableNumber << ","
         << "\"status\":\"Pending\","
         << "\"customer_name\":\"" << customerName << "\","
         << "\"notes\":\"" << notes << "\","
         << "\"total\":0.0"
         << "}}}";
    auto json = httpPost(baseUrl() + "/orders/", body.str());
    return parseOrder(parseDataObject(json));
}

void RestApiService::addOrderItem(long long orderId, long long menuItemId,
                                   int quantity, const std::string& instructions)
{
    // First get the menu item to know its price
    auto mi = getMenuItem(menuItemId);

    std::stringstream body;
    body << "{\"data\":{\"type\":\"order_item\",\"attributes\":{"
         << "\"order_id\":" << orderId << ","
         << "\"menu_item_id\":" << menuItemId << ","
         << "\"quantity\":" << quantity << ","
         << "\"unit_price\":" << std::fixed << std::setprecision(2) << mi.price << ","
         << "\"special_instructions\":\"" << instructions << "\""
         << "}}}";
    httpPost(baseUrl() + "/order_item/", body.str());

    // Update order total
    auto order = getOrder(orderId);
    double newTotal = order.total + mi.price * quantity;
    std::stringstream patchBody;
    patchBody << "{\"data\":{\"type\":\"orders\",\"id\":\"" << orderId
              << "\",\"attributes\":{\"total\":"
              << std::fixed << std::setprecision(2) << newTotal << "}}}";
    httpPatch(baseUrl() + "/orders/" + std::to_string(orderId) + "/", patchBody.str());
}

void RestApiService::updateOrderStatus(long long orderId, const std::string& status) {
    std::string body = "{\"data\":{\"type\":\"orders\",\"id\":\"" +
        std::to_string(orderId) + "\",\"attributes\":{\"status\":\"" +
        status + "\"}}}";
    httpPatch(baseUrl() + "/orders/" + std::to_string(orderId) + "/", body);
}

void RestApiService::cancelOrder(long long orderId) {
    updateOrderStatus(orderId, "Cancelled");
}

std::vector<OrderItemDto> RestApiService::getOrderItems(long long orderId) {
    // Use include to get menu_item names in one request
    auto json = httpGet(baseUrl() + "/order_item/?filter%5Border_id%5D=" +
                        std::to_string(orderId) +
                        "&include=menu_item");
    auto arr = parseDataArray(json);
    auto included = buildIncludedMap(json);

    std::vector<OrderItemDto> out;
    for (int i = 0; i < (int)arr.size(); i++) {
        const Wt::Json::Object& obj = arr[i];
        auto dto = parseOrderItem(obj);

        // Resolve menu_item name from included
        std::string miKey = "menu_item:" + std::to_string(dto.menu_item_id);
        auto it = included.find(miKey);
        if (it != included.end()) {
            const Wt::Json::Object& miAttrs = it->second.get("attributes");
            dto.menu_item_name = jsonStr(miAttrs, "name");
        }
        out.push_back(dto);
    }
    return out;
}

// ─── Dashboard / reporting ───────────────────────────────────────────────────
// These aggregate on the client from the full order list.

int RestApiService::getOrderCount(long long restaurantId) {
    return (int)getOrders(restaurantId).size();
}

double RestApiService::getRevenue(long long restaurantId) {
    auto all = getOrders(restaurantId);
    double total = 0.0;
    for (auto& o : all) {
        if (o.status == "Served") total += o.total;
    }
    return total;
}

int RestApiService::getPendingOrderCount(long long restaurantId) {
    return (int)getOrdersByStatus(restaurantId, "Pending").size();
}

int RestApiService::getInProgressOrderCount(long long restaurantId) {
    return (int)getOrdersByStatus(restaurantId, "In Progress").size();
}
