#ifndef UMMAH_CONFIG_H
#define UMMAH_CONFIG_H

#include <string>
#include <map>
#include <cstdlib>
#include <iostream>

class Config {
private:
    std::map<std::string, std::string> config_map;
    
public:
    Config() {
        loadFromEnvironment();
    }
    
    void loadFromEnvironment() {
        // Database configuration
        config_map["db_host"] = getEnvVar("POSTGRES_HOST", "localhost");
        config_map["db_port"] = getEnvVar("POSTGRES_PORT", "5432");
        config_map["db_name"] = getEnvVar("POSTGRES_DB", "ummah_app_dev");
        config_map["db_user"] = getEnvVar("POSTGRES_USER", "ummah_dev_user");
        config_map["db_password"] = getEnvVar("POSTGRES_PASSWORD", "CHANGE_ME_IN_PRODUCTION");
        
        // Redis configuration
        config_map["redis_host"] = getEnvVar("REDIS_HOST", "localhost");
        config_map["redis_port"] = getEnvVar("REDIS_PORT", "6379");
        config_map["redis_password"] = getEnvVar("REDIS_PASSWORD", "CHANGE_ME_IN_PRODUCTION");
        
        // Security configuration
        config_map["jwt_secret"] = getEnvVar("JWT_SECRET", "CHANGE_JWT_SECRET_IN_PRODUCTION");
        config_map["encryption_key"] = getEnvVar("ENCRYPTION_KEY", "dev_encryption_key_32_chars_long");
        
        // API configuration
        config_map["api_port"] = getEnvVar("API_PORT", "8080");
        config_map["api_host"] = getEnvVar("API_HOST", "0.0.0.0");
        
        // Service identification
        config_map["service_name"] = getEnvVar("SERVICE_NAME", "ummah-auth-service");
        config_map["service_version"] = getEnvVar("SERVICE_VERSION", "1.0.0");
    }
    
    std::string get(const std::string& key) const {
        auto it = config_map.find(key);
        if (it == config_map.end()) {
            std::cerr << "⚠️  Configuration key not found: " << key << std::endl;
            return "";
        }
        return it->second;
    }
    
    std::string getDatabaseUrl() const {
        return "postgresql://" + get("db_user") + ":" + get("db_password") + 
               "@" + get("db_host") + ":" + get("db_port") + "/" + get("db_name");
    }
    
    // ADDED: Missing printConfig method
    void printConfig() const {
        std::cout << "🔧 Configuration loaded:" << std::endl;
        std::cout << "  Service: " << get("service_name") << " v" << get("service_version") << std::endl;
        std::cout << "  Database: " << get("db_host") << ":" << get("db_port") << "/" << get("db_name") << std::endl;
        std::cout << "  Redis: " << get("redis_host") << ":" << get("redis_port") << std::endl;
        std::cout << "  API: " << get("api_host") << ":" << get("api_port") << std::endl;
    }
    
private:
    std::string getEnvVar(const std::string& key, const std::string& default_value = "") const {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : default_value;
    }
};

#endif // UMMAH_CONFIG_H