#ifndef COMMUNITY_MANAGER_H
#define COMMUNITY_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <libpq-fe.h>

struct Community {
    std::string id;
    std::string name;
    std::string description;
    std::string type; // public, private, invite_only
    std::string islamic_focus;
    std::string created_by;
    std::string created_at;
    int member_count;
    
    Community() : member_count(0) {}
};

struct CommunityMember {
    std::string user_id;
    std::string community_id;
    std::string role; // member, moderator, admin
    std::string joined_at;
};

class CommunityManager {
private:
    PGconn* connection;
    std::string connection_string;
    
public:
    CommunityManager(const std::string& conn_str);
    ~CommunityManager();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Community operations
    std::optional<Community> createCommunity(const std::string& name, 
                                            const std::string& description,
                                            const std::string& type,
                                            const std::string& created_by);
    std::vector<Community> getPublicCommunities();
    std::vector<Community> getUserCommunities(const std::string& user_id);
    std::optional<Community> getCommunityById(const std::string& id);
    
    // Membership operations
    bool joinCommunity(const std::string& user_id, const std::string& community_id);
    bool leaveCommunity(const std::string& user_id, const std::string& community_id);
    bool isUserMember(const std::string& user_id, const std::string& community_id);
    std::vector<CommunityMember> getCommunityMembers(const std::string& community_id);
    
private:
    Community parseCommunityFromResult(PGresult* result, int row);
    CommunityMember parseMemberFromResult(PGresult* result, int row);
};

#endif