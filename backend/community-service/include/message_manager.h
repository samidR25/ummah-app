#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <libpq-fe.h>

struct Message {
    std::string id;
    std::string sender_id;
    std::string sender_username;
    std::string community_id;
    std::string content;
    std::string created_at;
    bool is_moderated;
    
    Message() : is_moderated(false) {}
};

class MessageManager {
private:
    PGconn* connection;
    std::string connection_string;
    
public:
    MessageManager(const std::string& conn_str);
    ~MessageManager();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Message operations
    std::optional<Message> sendMessage(const std::string& sender_id,
                                     const std::string& community_id,
                                     const std::string& content);
    std::vector<Message> getCommunityMessages(const std::string& community_id, int limit = 50);
    bool deleteMessage(const std::string& message_id, const std::string& user_id);
    
private:
    Message parseMessageFromResult(PGresult* result, int row);
    bool isContentAppropriate(const std::string& content);
};

#endif