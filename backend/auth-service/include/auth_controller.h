#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <crow.h>
#include <memory>
#include <regex>
#include <nlohmann/json.hpp>
#include "database_manager.h"
#include "jwt_manager.h"
#include "password_manager.h"

class AuthController {
private:
    std::unique_ptr<DatabaseManager> db_manager;
    std::unique_ptr<JWTManager> jwt_manager;
    
public:
    AuthController(std::unique_ptr<DatabaseManager> db, 
                   std::unique_ptr<JWTManager> jwt);
    
    // Route handlers
    crow::response registerUser(const crow::request& req);
    crow::response loginUser(const crow::request& req);
    crow::response refreshToken(const crow::request& req);
    crow::response getUserProfile(const crow::request& req);
    crow::response updateProfile(const crow::request& req);
    crow::response deactivateAccount(const crow::request& req);
    
    // Middleware helpers
    std::optional<JWTPayload> validateAuthToken(const crow::request& req);
    
private:
    // Validation helpers
    bool isValidEmail(const std::string& email);
    bool isValidUsername(const std::string& username);
    bool isValidGender(const std::string& gender);
    bool isValidRequest(const nlohmann::json& json, const std::vector<std::string>& required_fields);
    
    // Response helpers
    crow::response createErrorResponse(int code, const std::string& message);
    crow::response createSuccessResponse(const nlohmann::json& data);
    crow::response createValidationErrorResponse(const std::string& field, const std::string& message);
    
    // Utility
    std::string extractTokenFromHeader(const std::string& auth_header);
    nlohmann::json userToJson(const User& user, bool include_sensitive = false);
    void logRequest(const std::string& method, const std::string& path, const std::string& result);
};

#endif // AUTH_CONTROLLER_H