#include "auth_controller.h"
#include <iostream>
#include <sstream>

AuthController::AuthController(std::unique_ptr<DatabaseManager> db, 
                               std::unique_ptr<JWTManager> jwt)
    : db_manager(std::move(db)), jwt_manager(std::move(jwt)) {
    std::cout << "🎮 Auth controller initialized" << std::endl;
}

crow::response AuthController::registerUser(const crow::request& req) {
    logRequest("POST", "/api/auth/register", "attempt");
    
    try {
        // Parse JSON request
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        
        // Validate required fields
        std::vector<std::string> required_fields = {"username", "email", "password", "gender"};
        if (!isValidRequest(json_req, required_fields)) {
            return createErrorResponse(400, "Missing required fields: username, email, password, gender");
        }
        
        std::string username = json_req["username"];
        std::string email = json_req["email"];
        std::string password = json_req["password"];
        std::string gender = json_req["gender"];
        
        // Validation
        if (!isValidEmail(email)) {
            return createValidationErrorResponse("email", "Invalid email format");
        }
        
        if (!isValidUsername(username)) {
            return createValidationErrorResponse("username", "Username must be 3-50 characters, alphanumeric and underscore only");
        }
        
        if (!isValidGender(gender)) {
            return createValidationErrorResponse("gender", "Gender must be 'male' or 'female'");
        }
        
        if (!PasswordManager::isPasswordStrong(password)) {
            return createValidationErrorResponse("password", 
                "Password must be at least 8 characters with uppercase, lowercase, digit and special character");
        }
        
        if (!PasswordManager::isIslamicallyAppropriate(password)) {
            return createValidationErrorResponse("password", 
                "Password contains inappropriate content. Please choose a different password.");
        }
        
        // Check if user already exists
        if (db_manager->getUserByEmail(email)) {
            return createErrorResponse(409, "User with this email already exists");
        }
        
        if (db_manager->getUserByUsername(username)) {
            return createErrorResponse(409, "Username already taken");
        }
        
        // Hash password
        std::string password_hash = PasswordManager::hashPassword(password);
        
        // Create user
        auto user = db_manager->createUser(username, email, password_hash, gender);
        if (!user) {
            return createErrorResponse(500, "Failed to create user");
        }
        
        // Generate JWT token
        std::string token = jwt_manager->generateToken(user->id, user->username, user->email, "user");
        
        // Prepare response
        nlohmann::json response_data;
        response_data["message"] = "User registered successfully. Welcome to the Ummah!";
        response_data["user"] = userToJson(*user, false);
        response_data["token"] = token;
        
        logRequest("POST", "/api/auth/register", "success");
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Registration error: " << e.what() << std::endl;
        logRequest("POST", "/api/auth/register", "error");
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response AuthController::loginUser(const crow::request& req) {
    logRequest("POST", "/api/auth/login", "attempt");
    
    try {
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        
        std::vector<std::string> required_fields = {"email", "password"};
        if (!isValidRequest(json_req, required_fields)) {
            return createErrorResponse(400, "Missing required fields: email, password");
        }
        
        std::string email = json_req["email"];
        std::string password = json_req["password"];
        
        // Get user from database
        auto user = db_manager->getUserByEmail(email);
        if (!user) {
            logRequest("POST", "/api/auth/login", "user_not_found");
            return createErrorResponse(401, "Invalid email or password");
        }
        
        // Verify password
        if (!PasswordManager::verifyPassword(password, user->password_hash)) {
            logRequest("POST", "/api/auth/login", "invalid_password");
            return createErrorResponse(401, "Invalid email or password");
        }
        
        // Generate JWT token
        std::string token = jwt_manager->generateToken(user->id, user->username, user->email, "user");
        
        // Prepare response
        nlohmann::json response_data;
        response_data["message"] = "Login successful. As-salamu alaikum!";
        response_data["user"] = userToJson(*user, false);
        response_data["token"] = token;
        
        logRequest("POST", "/api/auth/login", "success");
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Login error: " << e.what() << std::endl;
        logRequest("POST", "/api/auth/login", "error");
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response AuthController::refreshToken(const crow::request& req) {
    logRequest("POST", "/api/auth/refresh", "attempt");
    
    auto token_payload = validateAuthToken(req);
    if (!token_payload) {
        return createErrorResponse(401, "Invalid or expired token");
    }
    
    // Generate new token
    std::string new_token = jwt_manager->generateToken(
        token_payload->user_id, 
        token_payload->username, 
        token_payload->email, 
        token_payload->role
    );
    
    nlohmann::json response_data;
    response_data["message"] = "Token refreshed successfully";
    response_data["token"] = new_token;
    
    logRequest("POST", "/api/auth/refresh", "success");
    return createSuccessResponse(response_data);
}

crow::response AuthController::getUserProfile(const crow::request& req) {
    logRequest("GET", "/api/auth/profile", "attempt");
    
    auto token_payload = validateAuthToken(req);
    if (!token_payload) {
        return createErrorResponse(401, "Authentication required");
    }
    
    // Get fresh user data from database
    auto user = db_manager->getUserById(token_payload->user_id);
    if (!user) {
        return createErrorResponse(404, "User not found");
    }
    
    nlohmann::json response_data;
    response_data["user"] = userToJson(*user, false);
    
    logRequest("GET", "/api/auth/profile", "success");
    return createSuccessResponse(response_data);
}

crow::response AuthController::updateProfile(const crow::request& req) {
    logRequest("PUT", "/api/auth/profile", "attempt");
    
    auto token_payload = validateAuthToken(req);
    if (!token_payload) {
        return createErrorResponse(401, "Authentication required");
    }
    
    try {
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        
        // Get current user
        auto user = db_manager->getUserById(token_payload->user_id);
        if (!user) {
            return createErrorResponse(404, "User not found");
        }
        
        // Update allowed fields
        if (json_req.contains("location")) {
            user->location = json_req["location"];
        }
        
        if (json_req.contains("prayer_reminders")) {
            user->prayer_reminders = json_req["prayer_reminders"];
        }
        
        if (json_req.contains("gender_interaction_preference")) {
            std::string pref = json_req["gender_interaction_preference"];
            if (pref == "same_gender_only" || pref == "all" || pref == "family_only") {
                user->gender_interaction_preference = pref;
            } else {
                return createValidationErrorResponse("gender_interaction_preference", 
                    "Must be 'same_gender_only', 'all', or 'family_only'");
            }
        }
        
        // Update in database
        if (!db_manager->updateUser(token_payload->user_id, *user)) {
            return createErrorResponse(500, "Failed to update profile");
        }
        
        nlohmann::json response_data;
        response_data["message"] = "Profile updated successfully";
        response_data["user"] = userToJson(*user, false);
        
        logRequest("PUT", "/api/auth/profile", "success");
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Profile update error: " << e.what() << std::endl;
        logRequest("PUT", "/api/auth/profile", "error");
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response AuthController::deactivateAccount(const crow::request& req) {
    logRequest("DELETE", "/api/auth/deactivate", "attempt");
    
    auto token_payload = validateAuthToken(req);
    if (!token_payload) {
        return createErrorResponse(401, "Authentication required");
    }
    
    if (!db_manager->deactivateUser(token_payload->user_id)) {
        return createErrorResponse(500, "Failed to deactivate account");
    }
    
    nlohmann::json response_data;
    response_data["message"] = "Account deactivated successfully. May Allah bless you in your journey.";
    
    logRequest("DELETE", "/api/auth/deactivate", "success");
    return createSuccessResponse(response_data);
}

std::optional<JWTPayload> AuthController::validateAuthToken(const crow::request& req) {
    std::string auth_header = req.get_header_value("Authorization");
    if (auth_header.empty()) {
        return std::nullopt;
    }
    
    std::string token = extractTokenFromHeader(auth_header);
    if (token.empty()) {
        return std::nullopt;
    }
    
    return jwt_manager->validateToken(token);
}

bool AuthController::isValidEmail(const std::string& email) {
    const std::regex email_regex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );
    return std::regex_match(email, email_regex);
}

bool AuthController::isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 50) {
        return false;
    }
    
    const std::regex username_regex(R"(^[a-zA-Z0-9_]+$)");
    return std::regex_match(username, username_regex);
}

bool AuthController::isValidGender(const std::string& gender) {
    return gender == "male" || gender == "female";
}

bool AuthController::isValidRequest(const nlohmann::json& json, const std::vector<std::string>& required_fields) {
    for (const std::string& field : required_fields) {
        if (!json.contains(field) || json[field].is_null()) {
            return false;
        }
    }
    return true;
}

crow::response AuthController::createErrorResponse(int code, const std::string& message) {
    nlohmann::json error_response;
    error_response["success"] = false;
    error_response["error"] = {
        {"code", code},
        {"message", message}
    };
    error_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return crow::response(code, error_response.dump());
}

crow::response AuthController::createSuccessResponse(const nlohmann::json& data) {
    nlohmann::json success_response;
    success_response["success"] = true;
    success_response["data"] = data;
    success_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return crow::response(200, success_response.dump());
}

crow::response AuthController::createValidationErrorResponse(const std::string& field, const std::string& message) {
    nlohmann::json error_response;
    error_response["success"] = false;
    error_response["error"] = {
        {"code", 400},
        {"message", "Validation error"},
        {"field", field},
        {"details", message}
    };
    error_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return crow::response(400, error_response.dump());
}

std::string AuthController::extractTokenFromHeader(const std::string& auth_header) {
    const std::string bearer_prefix = "Bearer ";
    if (auth_header.length() > bearer_prefix.length() && 
        auth_header.substr(0, bearer_prefix.length()) == bearer_prefix) {
        return auth_header.substr(bearer_prefix.length());
    }
    return "";
}

nlohmann::json AuthController::userToJson(const User& user, bool include_sensitive) {
    nlohmann::json user_json;
    user_json["id"] = user.id;
    user_json["username"] = user.username;
    user_json["email"] = user.email;
    user_json["gender"] = user.gender;
    user_json["location"] = user.location;
    user_json["islamic_verification_level"] = user.islamic_verification_level;
    user_json["prayer_reminders"] = user.prayer_reminders;
    user_json["gender_interaction_preference"] = user.gender_interaction_preference;
    user_json["is_active"] = user.is_active;
    user_json["created_at"] = user.created_at;
    
    // Never include password hash in JSON response
    // include_sensitive flag reserved for future admin features
    
    return user_json;
}

void AuthController::logRequest(const std::string& method, const std::string& path, const std::string& result) {
    std::cout << "📡 " << method << " " << path << " - " << result << std::endl;
}