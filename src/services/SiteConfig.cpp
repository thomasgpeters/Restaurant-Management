#include "SiteConfig.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

// Minimal JSON helpers (no external library needed)
// We only handle a flat object with string values.

static std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}

static std::string jsonUnescape(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 'r':  out += '\r'; i++; break;
                case 't':  out += '\t'; i++; break;
                default:   out += s[i];
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

// Extract value for a given key from a flat JSON string
static std::string jsonGet(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";

    // Find the colon after the key
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return "";

    // Find the opening quote of the value
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";

    // Find the closing quote (handle escapes)
    size_t start = pos + 1;
    size_t end = start;
    while (end < json.size()) {
        if (json[end] == '\\') { end += 2; continue; }
        if (json[end] == '"') break;
        end++;
    }

    return jsonUnescape(json.substr(start, end - start));
}

// Ensure parent directory exists
static void ensureDir(const std::string& path) {
    auto slash = path.rfind('/');
    if (slash != std::string::npos) {
        std::string dir = path.substr(0, slash);
        mkdir(dir.c_str(), 0755);
    }
}

SiteConfig::SiteConfig(const std::string& configPath)
    : configPath_(configPath)
{
    load();
}

void SiteConfig::load() {
    std::ifstream file(configPath_);
    if (!file.is_open()) {
        std::cout << "[SiteConfig] No config file at " << configPath_
                  << ", using defaults." << std::endl;
    } else {
        std::stringstream ss;
        ss << file.rdbuf();
        std::string json = ss.str();
        file.close();

        storeName_  = jsonGet(json, "store_name");
        storeLogo_  = jsonGet(json, "store_logo");
        std::string url = jsonGet(json, "api_base_url");
        if (!url.empty()) apiBaseUrl_ = url;
        std::string dst = jsonGet(json, "data_source_type");
        if (!dst.empty()) dataSourceType_ = dst;
    }

    // Environment variable overrides config file (always checked)
    const char* envDst = std::getenv("DATA_SOURCE_TYPE");
    if (envDst && (std::string(envDst) == "ALS" || std::string(envDst) == "LOCAL")) {
        dataSourceType_ = envDst;
    }

    std::cout << "[SiteConfig] Loaded: store=" << storeName_
              << " api=" << apiBaseUrl_
              << " data_source=" << dataSourceType_ << std::endl;
}

void SiteConfig::save() const {
    ensureDir(configPath_);

    std::ofstream file(configPath_);
    if (!file.is_open()) {
        std::cerr << "[SiteConfig] Failed to write " << configPath_ << std::endl;
        return;
    }

    file << "{\n"
         << "  \"store_name\": \"" << jsonEscape(storeName_) << "\",\n"
         << "  \"store_logo\": \"" << jsonEscape(storeLogo_) << "\",\n"
         << "  \"api_base_url\": \"" << jsonEscape(apiBaseUrl_) << "\",\n"
         << "  \"data_source_type\": \"" << jsonEscape(dataSourceType_) << "\"\n"
         << "}\n";

    file.close();
    std::cout << "[SiteConfig] Saved config to " << configPath_ << std::endl;
}

void SiteConfig::reload() {
    std::lock_guard<std::mutex> lock(mutex_);
    load();
}

// ── Getters ──

std::string SiteConfig::storeName() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return storeName_;
}

std::string SiteConfig::storeLogo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return storeLogo_;
}

std::string SiteConfig::apiBaseUrl() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return apiBaseUrl_;
}

std::string SiteConfig::dataSourceType() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return dataSourceType_;
}

// ── Setters ──

void SiteConfig::setStoreName(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    storeName_ = name;
    save();
}

void SiteConfig::setStoreLogo(const std::string& logoPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    storeLogo_ = logoPath;
    save();
}

void SiteConfig::setApiBaseUrl(const std::string& url) {
    std::lock_guard<std::mutex> lock(mutex_);
    apiBaseUrl_ = url;
    save();
}

void SiteConfig::setDataSourceType(const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex_);
    dataSourceType_ = type;
    save();
}

void SiteConfig::update(const std::string& storeName,
                        const std::string& storeLogo,
                        const std::string& apiBaseUrl,
                        const std::string& dataSourceType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    storeName_ = storeName;
    storeLogo_ = storeLogo;
    if (!apiBaseUrl.empty()) apiBaseUrl_ = apiBaseUrl;
    if (!dataSourceType.empty()) dataSourceType_ = dataSourceType;
    save();
}
