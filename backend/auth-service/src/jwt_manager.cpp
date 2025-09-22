#include "jwt_manager.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>

JWTManager::JWTManager(const std::string& secret, std::chrono::hours validity) 
    : secret_key(secret), token_validity(validity) {
    if (secret_key.empty()) {
        std::cerr << "⚠️  Warning: Empty JWT secret key!" << std::endl;
    }
    std::cout << "🔐 JWT manager initialized with " << validity.count() << "h token validity" << std::endl;
}

std::string JWTManager::generateToken(const std::string& user_id, 
                                      const std::string& username,
                                      const std::string& email,
                                      const std::string& role) {
    try {
        std::string header = createHeader();
        std::string payload = createPayload(user_id, username, email, role);
        
        std::string message = base64UrlEncode(header) + "." + base64UrlEncode(payload);
        std::string signature = base64UrlEncode(hmacSha256(message, secret_key));
        
        return message + "." + signature;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error generating JWT token: " << e.what() << std::endl;
        return "";
    }
}

std::optional<JWTPayload> JWTManager::validateToken(const std::string& token) {
    try {
        std::vector<std::string> parts = split(token, '.');
        if (parts.size() != 3) {
            return std::nullopt;
        }
        
        // Verify signature
        std::string message = parts[0] + "." + parts[1];
        std::string expected_signature = base64UrlEncode(hmacSha256(message, secret_key));
        
        if (parts[2] != expected_signature) {
            std::cout << "🔒 JWT signature validation failed" << std::endl;
            return std::nullopt;
        }
        
        // Parse payload
        std::string payload_json = base64UrlDecode(parts[1]);
        nlohmann::json payload = nlohmann::json::parse(payload_json);
        
        JWTPayload jwt_payload;
        jwt_payload.user_id = payload["sub"];
        jwt_payload.username = payload["username"];
        jwt_payload.email = payload["email"];
        jwt_payload.role = payload.value("role", "user");
        
        // Parse timestamps
        jwt_payload.issued_at = std::chrono::system_clock::from_time_t(payload["iat"]);
        jwt_payload.expires_at = std::chrono::system_clock::from_time_t(payload["exp"]);
        
        // Check if token is expired
        if (isTokenExpired(jwt_payload)) {
            std::cout << "⏰ JWT token has expired" << std::endl;
            return std::nullopt;
        }
        
        return jwt_payload;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error validating JWT token: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool JWTManager::isTokenExpired(const JWTPayload& payload) {
    return std::chrono::system_clock::now() > payload.expires_at;
}

std::string JWTManager::refreshToken(const std::string& token) {
    auto payload = validateToken(token);
    if (!payload) {
        return "";
    }
    
    // Generate new token with same user data
    return generateToken(payload->user_id, payload->username, payload->email, payload->role);
}

std::string JWTManager::createHeader() {
    nlohmann::json header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    return header.dump();
}

std::string JWTManager::createPayload(const std::string& user_id, const std::string& username, 
                                      const std::string& email, const std::string& role) {
    auto now = std::chrono::system_clock::now();
    auto iat = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    auto exp = std::chrono::duration_cast<std::chrono::seconds>((now + token_validity).time_since_epoch()).count();
    
    nlohmann::json payload;
    payload["sub"] = user_id;  // Subject (user ID)
    payload["username"] = username;
    payload["email"] = email;
    payload["role"] = role;
    payload["iat"] = iat;  // Issued at
    payload["exp"] = exp;  // Expires at
    payload["iss"] = "ummah-auth-service";  // Issuer
    
    return payload.dump();
}

std::string JWTManager::base64UrlEncode(const std::string& input) {
    // Simple base64url encoding (production should use proper library)
    std::string encoded;
    
    // Base64 character set
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    // Add padding
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    
    // Convert to base64url (replace + with -, / with _, remove padding)
    std::replace(encoded.begin(), encoded.end(), '+', '-');
    std::replace(encoded.begin(), encoded.end(), '/', '_');
    encoded.erase(std::find(encoded.begin(), encoded.end(), '='), encoded.end());
    
    return encoded;
}

std::string JWTManager::base64UrlDecode(const std::string& input) {
    // Simple base64url decoding
    std::string padded = input;
    
    // Convert base64url back to base64
    std::replace(padded.begin(), padded.end(), '-', '+');
    std::replace(padded.begin(), padded.end(), '_', '/');
    
    // Add padding
    while (padded.size() % 4) {
        padded.push_back('=');
    }
    
    // Decode base64
    std::string decoded;
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    int val = 0, valb = -8;
    for (char c : padded) {
        if (c == '=') break;
        
        size_t pos = chars.find(c);
        if (pos == std::string::npos) continue;
        
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return decoded;
}

std::string JWTManager::hmacSha256(const std::string& data, const std::string& key) {
    unsigned char* result = HMAC(EVP_sha256(), key.c_str(), key.length(),
                                 reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
                                 nullptr, nullptr);
    
    return std::string(reinterpret_cast<char*>(result), SHA256_DIGEST_LENGTH);
}

std::vector<std::string> JWTManager::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}