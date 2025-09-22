#include "password_manager.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>

std::string PasswordManager::hashPassword(const std::string& password) {
    // Generate a random salt
    std::string salt = generateSalt();
    
    // Create hash with PBKDF2 simulation (simplified for demo)
    std::string combined = salt + password;
    
    // Use SHA-256 for hashing (in production, use proper PBKDF2)
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    // Convert to hex string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    // Return salt:hash format
    return salt + ":" + ss.str();
}

bool PasswordManager::verifyPassword(const std::string& password, const std::string& hash) {
    // Split salt and hash
    size_t colon_pos = hash.find(':');
    if (colon_pos == std::string::npos) {
        std::cerr << "❌ Invalid hash format" << std::endl;
        return false;
    }
    
    std::string salt = hash.substr(0, colon_pos);
    std::string stored_hash = hash.substr(colon_pos + 1);
    
    // Hash the provided password with the stored salt
    std::string combined = salt + password;
    
    unsigned char computed_hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), computed_hash);
    
    // Convert computed hash to hex string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)computed_hash[i];
    }
    
    return ss.str() == stored_hash;
}

bool PasswordManager::isPasswordStrong(const std::string& password) {
    // Minimum 8 characters
    if (password.length() < 8) {
        return false;
    }
    
    // Check for at least one uppercase, one lowercase, one digit, one special character
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    
    for (char c : password) {
        if (c >= 'A' && c <= 'Z') {
            has_upper = true;
        } else if (c >= 'a' && c <= 'z') {
            has_lower = true;
        } else if (c >= '0' && c <= '9') {
            has_digit = true;
        } else if (c == '!' || c == '@' || c == '#' || c == '$' || 
                   c == '%' || c == '^' || c == '&' || c == '*') {
            has_special = true;
        }
    }
    
    return has_upper && has_lower && has_digit && has_special;
}


bool PasswordManager::isIslamicallyAppropriate(const std::string& password) {
    // Check for inappropriate words/content
    std::vector<std::string> inappropriate_words = {
        "allah", "muhammad", "prophet", "islam", "muslim", "quran", "haram", 
        "halal", "mosque", "mecca", "medina" // Add more as needed
    };
    
    std::string lower_password = password;
    std::transform(lower_password.begin(), lower_password.end(), lower_password.begin(), ::tolower);
    
    for (const std::string& word : inappropriate_words) {
        if (lower_password.find(word) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::string PasswordManager::generateSalt() {
    return generateRandomString(SALT_LENGTH);
}

std::string PasswordManager::generateRandomString(int length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);
    
    unsigned char random_bytes[length];
    if (RAND_bytes(random_bytes, length) != 1) {
        // Fallback to simpler random generation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, charset.size() - 1);
        
        for (int i = 0; i < length; ++i) {
            result += charset[dis(gen)];
        }
    } else {
        for (int i = 0; i < length; ++i) {
            result += charset[random_bytes[i] % charset.size()];
        }
    }
    
    return result;
}