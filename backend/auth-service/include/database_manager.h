#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <libpq-fe.h>

struct User {
    std::string id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string gender;
    std::string location;
    int islamic_verification_level;
    bool prayer_reminders;
    std::string gender_interaction_preference;
    bool is_active;
    std::string created_at;
    
    // Constructor
    User() : islamic_verification_level(0), prayer_reminders(true), is_active(true) {}
};

class DatabaseManager {
private:
    PGconn* connection;
    std::string connection_string;
    
public:
    DatabaseManager(const std::string& conn_str);
    ~DatabaseManager();
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;
    bool reconnectIfNeeded();
    
    // User operations
    std::optional<User> createUser(const std::string& username, 
                                   const std::string& email,
                                   const std::string& password_hash,
                                   const std::string& gender);
    std::optional<User> getUserByEmail(const std::string& email);
    std::optional<User> getUserByUsername(const std::string& username);
    std::optional<User> getUserById(const std::string& id);
    bool updateUser(const std::string& id, const User& user);
    bool deactivateUser(const std::string& id);
    
    // Utility
    std::string escapeString(const std::string& input);
    bool testConnection();
    
private:
    User parseUserFromResult(PGresult* result, int row);
    bool executeQuery(const std::string& query);
    void logError(const std::string& operation, const std::string& error);
};

#endif // DATABASE_MANAGER_H