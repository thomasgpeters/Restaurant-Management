#include <Wt/WApplication.h>
#include <Wt/WServer.h>

#include "ui/RestaurantApp.h"
#include "services/ApiService.h"

#include <memory>
#include <iostream>

int main(int argc, char** argv) {
    try {
        // Initialize shared API service (single DB connection for the process)
        auto apiService = std::make_shared<ApiService>("restaurant_pos.db");
        apiService->initializeDatabase();
        apiService->seedDatabase();

        // Store for access in application factory
        RestaurantApp::sharedApiService = apiService;

        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        server.addEntryPoint(
            Wt::EntryPointType::Application,
            [apiService](const Wt::WEnvironment& env) {
                return std::make_unique<RestaurantApp>(env, apiService);
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
