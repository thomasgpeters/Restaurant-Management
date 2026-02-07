#pragma once

#include <string>
#include <mutex>

// ─── Site Configuration ──────────────────────────────────────────────────────
// Manages persistent site settings stored in a JSON file.
// Thread-safe: multiple Wt sessions may read/write concurrently.

class SiteConfig {
public:
    explicit SiteConfig(const std::string& configPath = "data/site-config.json");

    // Getters
    std::string storeName() const;
    std::string storeLogo() const;
    std::string apiBaseUrl() const;
    std::string dataSourceType() const;   // "LOCAL" or "ALS"

    // Setters (auto-save to disk)
    void setStoreName(const std::string& name);
    void setStoreLogo(const std::string& logoPath);
    void setApiBaseUrl(const std::string& url);
    void setDataSourceType(const std::string& type);

    // Bulk update + save
    void update(const std::string& storeName,
                const std::string& storeLogo,
                const std::string& apiBaseUrl,
                const std::string& dataSourceType = "");

    // Reload from disk
    void reload();

private:
    void load();
    void save() const;

    std::string configPath_;

    std::string storeName_;
    std::string storeLogo_;
    std::string apiBaseUrl_ = "http://localhost:5656/api";
    std::string dataSourceType_ = "LOCAL";  // "LOCAL" or "ALS"

    mutable std::mutex mutex_;
};
