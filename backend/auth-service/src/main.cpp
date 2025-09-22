#include <crow.h>
#include <iostream>
#include <memory>
#include <csignal>
#include <nlohmann/json.hpp>  // ADDED: Missing include
#include "auth_controller.h"
#include "database_manager.h"
#include "jwt_manager.h"
#include "config.h"

// Global flag for graceful shutdown
volatile sig_atomic_t shutdown_requested = 0;

void signalHandler(int signal) {
    std::cout << "\n🛑 Received signal " << signal << ". Shutting down gracefully..." << std::endl;
    shutdown_requested = 1;
}

int main() {
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    std::cout << "🕌 Starting Ummah App Authentication Service..." << std::endl;
    
    try {
        // Load configuration
        Config config;
        config.printConfig();  // This method now exists
        
        // Initialize database manager
        auto db_manager = std::make_unique<DatabaseManager>(config.getDatabaseUrl());
        if (!db_manager->connect()) {
            std::cerr << "❌ Failed to connect to database!" << std::endl;
            return 1;
        }
        
        if (!db_manager->testConnection()) {
            std::cerr << "❌ Database connection test failed!" << std::endl;
            return 1;
        }
        std::cout << "✅ Database connected and tested successfully" << std::endl;
        
        // Initialize JWT manager
        auto jwt_manager = std::make_unique<JWTManager>(config.get("jwt_secret"));
        std::cout << "✅ JWT manager initialized" << std::endl;
        
        // Initialize auth controller
        AuthController auth_controller(std::move(db_manager), std::move(jwt_manager));
        
        // Initialize Crow app
        crow::SimpleApp app;
        
        // Basic configuration
        app.loglevel(crow::LogLevel::Info);
        
        // REMOVED: CORS middleware (not available in this Crow version)
        // We'll handle CORS manually in responses if needed
        
        // Health check endpoint
        CROW_ROUTE(app, "/health").methods("GET"_method)([]() {
            nlohmann::json response;
            response["status"] = "healthy";
            response["service"] = "ummah-auth-service";
            response["version"] = "1.0.0";
            response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            auto crow_response = crow::response(200, response.dump());
            crow_response.add_header("Content-Type", "application/json");
            // Manual CORS headers for development
            crow_response.add_header("Access-Control-Allow-Origin", "*");
            crow_response.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            crow_response.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return crow_response;
        });
        
        // API Info endpoint
        CROW_ROUTE(app, "/api/info").methods("GET"_method)([]() {
            nlohmann::json response;
            response["service"] = "Ummah App Authentication Service";
            response["version"] = "1.0.0";
            response["endpoints"] = {
                {"POST /api/auth/register", "Register new user"},
                {"POST /api/auth/login", "User login"},
                {"POST /api/auth/refresh", "Refresh JWT token"},
                {"GET /api/auth/profile", "Get user profile (requires auth)"},
                {"PUT /api/auth/profile", "Update user profile (requires auth)"},
                {"DELETE /api/auth/deactivate", "Deactivate account (requires auth)"}
            };
            response["islamic_compliance"] = {
                {"content_moderation", "Islamic guidelines enforced"},
                {"privacy_controls", "Gender interaction preferences supported"},
                {"data_protection", "Minimal data collection principles"}
            };
            
            auto crow_response = crow::response(200, response.dump());
            crow_response.add_header("Content-Type", "application/json");
            crow_response.add_header("Access-Control-Allow-Origin", "*");
            return crow_response;
        });
        
        // Authentication endpoints
        CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.registerUser(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.loginUser(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/auth/refresh").methods("POST"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.refreshToken(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/auth/profile").methods("GET"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.getUserProfile(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/auth/profile").methods("PUT"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.updateProfile(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        CROW_ROUTE(app, "/api/auth/deactivate").methods("DELETE"_method)
        ([&auth_controller](const crow::request& req) {
            auto response = auth_controller.deactivateAccount(req);
            response.add_header("Access-Control-Allow-Origin", "*");
            return response;
        });
        
        // Start server
        int port = std::stoi(config.get("api_port"));
        std::string host = config.get("api_host");
        
        std::cout << "🚀 Auth service starting on " << host << ":" << port << std::endl;
        std::cout << "📚 Available endpoints:" << std::endl;
        std::cout << "  GET  /health                 - Health check" << std::endl;
        std::cout << "  GET  /api/info               - Service information" << std::endl;
        std::cout << "  POST /api/auth/register      - Register new user" << std::endl;
        std::cout << "  POST /api/auth/login         - User login" << std::endl;
        std::cout << "  POST /api/auth/refresh       - Refresh JWT token" << std::endl;
        std::cout << "  GET  /api/auth/profile       - Get user profile (auth required)" << std::endl;
        std::cout << "  PUT  /api/auth/profile       - Update profile (auth required)" << std::endl;
        std::cout << "  DEL  /api/auth/deactivate    - Deactivate account (auth required)" << std::endl;
        std::cout << "🕌 Ready to serve the Ummah with secure authentication!" << std::endl;
        
        app.bindaddr(host).port(port).multithreaded().run();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "👋 Authentication service stopped gracefully. As-salamu alaikum!" << std::endl;
    return 0;
}