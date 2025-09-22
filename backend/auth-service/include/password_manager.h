#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include <string>
#include <random>

class PasswordManager {
public:
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hash);
    static bool isPasswordStrong(const std::string& password);
    static std::string generateSalt();
    
    // Islamic compliance checks
    static bool isIslamicallyAppropriate(const std::string& password);
    
private:
    static constexpr int SALT_LENGTH = 16;
    static std::string pbkdf2(const std::string& password, const std::string& salt, int iterations);
    static std::string generateRandomString(int length);
};

#endif // PASSWORD_MANAGER_H