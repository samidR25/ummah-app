#ifndef JWT_MANAGER_H
#define JWT_MANAGER_H

#include <string>
#include <map>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

struct JWTPayload {
    std::string user_id;
    std::string username;
    std::string email;
    std::string role;
    std::chrono::system_clock::time_point issued_at;
    std::chrono::system_clock::time_point expires_at;
    
    JWTPayload() : role("user") {}
};

class JWTManager {
private:
    std::string secret_key;
    std::chrono::hours token_validity;
    
public:
    JWTManager(const std::string& secret, std::chrono::hours validity = std::chrono::hours(24));
    
    std::string generateToken(const std::string& user_id, 
                             const std::string& username,
                             const std::string& email,
                             const std::string& role = "user");
    
    std::optional<JWTPayload> validateToken(const std::string& token);
    bool isTokenExpired(const JWTPayload& payload);
    
    // Utility functions
    std::string refreshToken(const std::string& token);
    
private:
    std::string base64UrlEncode(const std::string& input);
    std::string base64UrlDecode(const std::string& input);
    std::string hmacSha256(const std::string& data, const std::string& key);
    std::string createHeader();
    std::string createPayload(const std::string& user_id, const std::string& username, 
                             const std::string& email, const std::string& role);
    std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif // JWT_MANAGER_H