#include "database_manager.h"
#include <iostream>
#include <cstring>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& conn_str) 
    : connection(nullptr), connection_string(conn_str) {
    std::cout << "🗄️  Database manager initialized" << std::endl;
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect() {
    if (connection && PQstatus(connection) == CONNECTION_OK) {
        return true; // Already connected
    }
    
    connection = PQconnectdb(connection_string.c_str());
    
    if (PQstatus(connection) != CONNECTION_OK) {
        logError("Connection", PQerrorMessage(connection));
        PQfinish(connection);
        connection = nullptr;
        return false;
    }
    
    std::cout << "✅ Database connection established" << std::endl;
    return true;
}

void DatabaseManager::disconnect() {
    if (connection) {
        PQfinish(connection);
        connection = nullptr;
        std::cout << "🔌 Database disconnected" << std::endl;
    }
}

bool DatabaseManager::isConnected() const {
    return connection != nullptr && PQstatus(connection) == CONNECTION_OK;
}

bool DatabaseManager::reconnectIfNeeded() {
    if (!isConnected()) {
        std::cout << "🔄 Attempting database reconnection..." << std::endl;
        return connect();
    }
    return true;
}

bool DatabaseManager::testConnection() {
    if (!isConnected() && !connect()) {
        return false;
    }
    
    PGresult* result = PQexec(connection, "SELECT 1 as test;");
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        logError("Connection test", PQerrorMessage(connection));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

std::optional<User> DatabaseManager::createUser(const std::string& username,
                                                const std::string& email,
                                                const std::string& password_hash,
                                                const std::string& gender) {
    if (!reconnectIfNeeded()) {
        return std::nullopt;
    }
    
    std::string query = 
        "INSERT INTO users (username, email, password_hash, gender) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING id, username, email, password_hash, gender, location, "
        "islamic_verification_level, prayer_reminders, gender_interaction_preference, "
        "is_active, created_at;";
    
    const char* params[4] = {
        username.c_str(),
        email.c_str(),
        password_hash.c_str(),
        gender.c_str()
    };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 4, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        logError("Create user", PQerrorMessage(connection));
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        std::cout << "⚠️  User creation succeeded but no data returned" << std::endl;
        PQclear(result);
        return std::nullopt;
    }
    
    User user = parseUserFromResult(result, 0);
    PQclear(result);
    
    std::cout << "✅ User created successfully: " << username << " (" << email << ")" << std::endl;
    return user;
}

std::optional<User> DatabaseManager::getUserByEmail(const std::string& email) {
    if (!reconnectIfNeeded()) {
        return std::nullopt;
    }
    
    std::string query = 
        "SELECT id, username, email, password_hash, gender, location, "
        "islamic_verification_level, prayer_reminders, gender_interaction_preference, "
        "is_active, created_at FROM users WHERE email = $1 AND is_active = true;";
    
    const char* params[1] = { email.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        logError("Get user by email", PQerrorMessage(connection));
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt; // User not found (this is normal)
    }
    
    User user = parseUserFromResult(result, 0);
    PQclear(result);
    return user;
}

std::optional<User> DatabaseManager::getUserByUsername(const std::string& username) {
    if (!reconnectIfNeeded()) {
        return std::nullopt;
    }
    
    std::string query = 
        "SELECT id, username, email, password_hash, gender, location, "
        "islamic_verification_level, prayer_reminders, gender_interaction_preference, "
        "is_active, created_at FROM users WHERE username = $1 AND is_active = true;";
    
    const char* params[1] = { username.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        logError("Get user by username", PQerrorMessage(connection));
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = parseUserFromResult(result, 0);
    PQclear(result);
    return user;
}

std::optional<User> DatabaseManager::getUserById(const std::string& id) {
    if (!reconnectIfNeeded()) {
        return std::nullopt;
    }
    
    std::string query = 
        "SELECT id, username, email, password_hash, gender, location, "
        "islamic_verification_level, prayer_reminders, gender_interaction_preference, "
        "is_active, created_at FROM users WHERE id = $1 AND is_active = true;";
    
    const char* params[1] = { id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        logError("Get user by ID", PQerrorMessage(connection));
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    User user = parseUserFromResult(result, 0);
    PQclear(result);
    return user;
}

bool DatabaseManager::updateUser(const std::string& id, const User& user) {
    if (!reconnectIfNeeded()) {
        return false;
    }
    
    std::string query = 
        "UPDATE users SET location = $1, prayer_reminders = $2, "
        "gender_interaction_preference = $3, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = $4 AND is_active = true;";
    
    std::string prayer_reminders_str = user.prayer_reminders ? "true" : "false";
    
    const char* params[4] = {
        user.location.c_str(),
        prayer_reminders_str.c_str(),
        user.gender_interaction_preference.c_str(),
        id.c_str()
    };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 4, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        logError("Update user", PQerrorMessage(connection));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    std::cout << "✅ User updated successfully: " << id << std::endl;
    return true;
}

bool DatabaseManager::deactivateUser(const std::string& id) {
    if (!reconnectIfNeeded()) {
        return false;
    }
    
    std::string query = 
        "UPDATE users SET is_active = false, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = $1;";
    
    const char* params[1] = { id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        logError("Deactivate user", PQerrorMessage(connection));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    std::cout << "✅ User deactivated successfully: " << id << std::endl;
    return true;
}

User DatabaseManager::parseUserFromResult(PGresult* result, int row) {
    User user;
    user.id = PQgetvalue(result, row, 0) ? PQgetvalue(result, row, 0) : "";
    user.username = PQgetvalue(result, row, 1) ? PQgetvalue(result, row, 1) : "";
    user.email = PQgetvalue(result, row, 2) ? PQgetvalue(result, row, 2) : "";
    user.password_hash = PQgetvalue(result, row, 3) ? PQgetvalue(result, row, 3) : "";
    user.gender = PQgetvalue(result, row, 4) ? PQgetvalue(result, row, 4) : "";
    user.location = PQgetvalue(result, row, 5) ? PQgetvalue(result, row, 5) : "";
    
    // Handle integer field
    const char* verification_level = PQgetvalue(result, row, 6);
    user.islamic_verification_level = verification_level ? std::stoi(verification_level) : 0;
    
    // Handle boolean fields
    const char* prayer_reminders = PQgetvalue(result, row, 7);
    user.prayer_reminders = prayer_reminders && strcmp(prayer_reminders, "t") == 0;
    
    user.gender_interaction_preference = PQgetvalue(result, row, 8) ? PQgetvalue(result, row, 8) : "";
    
    const char* is_active = PQgetvalue(result, row, 9);
    user.is_active = is_active && strcmp(is_active, "t") == 0;
    
    user.created_at = PQgetvalue(result, row, 10) ? PQgetvalue(result, row, 10) : "";
    
    return user;
}

std::string DatabaseManager::escapeString(const std::string& input) {
    if (!isConnected()) return input;
    
    char* escaped = PQescapeLiteral(connection, input.c_str(), input.length());
    if (!escaped) return input;
    
    std::string result(escaped);
    PQfreemem(escaped);
    return result;
}

bool DatabaseManager::executeQuery(const std::string& query) {
    if (!reconnectIfNeeded()) {
        return false;
    }
    
    PGresult* result = PQexec(connection, query.c_str());
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        logError("Execute query", PQerrorMessage(connection));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

void DatabaseManager::logError(const std::string& operation, const std::string& error) {
    std::cerr << "❌ Database error (" << operation << "): " << error << std::endl;
}