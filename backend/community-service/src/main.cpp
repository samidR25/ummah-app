#include <crow.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include "community_controller.h"
#include "community_manager.h"
#include "config.h"

int main() {
    std::cout << "🏘️ Starting Ummah Community Service..." << std::endl;
    
    try {
        Config config;
        config.printConfig();
        
        auto community_manager = std::make_unique<CommunityManager>(config.getDatabaseUrl());
        if (!community_manager->connect()) {
            std::cerr << "❌ Failed to connect to database!" << std::endl;
            return 1;
        }
        
        CommunityController controller(std::move(community_manager));
        crow::SimpleApp app;
        app.loglevel(crow::LogLevel::Info);
        
        // Health check
        CROW_ROUTE(app, "/health").methods("GET"_method)([]() {
            nlohmann::json response;
            response["status"] = "healthy";
            response["service"] = "ummah-community-service";
            auto crow_response = crow::response(200, response.dump());
            crow_response.add_header("Content-Type", "application/json");
            crow_response.add_header("Access-Control-Allow-Origin", "*");
            return crow_response;
        });
        
        // Community endpoints
        CROW_ROUTE(app, "/api/communities").methods("POST"_method)
        ([&controller](const crow::request& req) {
            auto response = controller.createCommunity(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/communities").methods("GET"_method)
        ([&controller](const crow::request& req) {
            auto response = controller.getPublicCommunities(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/communities/my").methods("GET"_method)
        ([&controller](const crow::request& req) {
            auto response = controller.getUserCommunities(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/communities/<string>/join").methods("POST"_method)
        ([&controller](const crow::request& req, const std::string& community_id) {
            auto response = controller.joinCommunity(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/communities/<string>/leave").methods("POST"_method)
        ([&controller](const crow::request& req, const std::string& community_id) {
            auto response = controller.leaveCommunity(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        int port = std::stoi(config.get("api_port")) + 1; // 8081
        std::cout << "🚀 Community service starting on port " << port << std::endl;
        app.port(port).multithreaded().run();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}