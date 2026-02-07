#include <Wt/WApplication.h>
#include <Wt/WServer.h>

#include "ui/RestaurantApp.h"
#include "services/LocalApiService.h"
#include "services/RestApiService.h"
#include "services/SiteConfig.h"

#include <memory>
#include <iostream>

int main(int argc, char** argv) {
    try {
        // Initialize shared site configuration
        auto siteConfig = std::make_shared<SiteConfig>("data/site-config.json");

        // Select API service based on data_source_type config / env variable
        std::shared_ptr<IApiService> apiService;
        std::string mode = siteConfig->dataSourceType();

        if (mode == "ALS") {
            std::cout << "[main] Enterprise mode: connecting to ApiLogicServer at "
                      << siteConfig->apiBaseUrl() << std::endl;
            apiService = std::make_shared<RestApiService>(siteConfig);
        } else {
            std::cout << "[main] Local mode: using SQLite database" << std::endl;
            auto local = std::make_shared<LocalApiService>("restaurant_pos.db");
            local->initializeDatabase();
            local->seedDatabase();
            apiService = local;
        }

        // Store for access in application factory
        RestaurantApp::sharedApiService = apiService;
        RestaurantApp::sharedSiteConfig = siteConfig;

        Wt::WServer server(argc, argv);

        server.addEntryPoint(
            Wt::EntryPointType::Application,
            [apiService, siteConfig](const Wt::WEnvironment& env) {
                return std::make_unique<RestaurantApp>(env, apiService, siteConfig);
            }
        );

        server.run();
    } catch (Wt::WServer::Exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
