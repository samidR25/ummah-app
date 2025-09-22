#include "community_manager.h"
#include <iostream>
#include <cstring>
#include <sstream>

CommunityManager::CommunityManager(const std::string& conn_str) 
    : connection(nullptr), connection_string(conn_str) {
    std::cout << "🏘️  Community manager initialized" << std::endl;
}

CommunityManager::~CommunityManager() {
    disconnect();
}

bool CommunityManager::connect() {
    if (connection && PQstatus(connection) == CONNECTION_OK) {
        return true;
    }
    
    connection = PQconnectdb(connection_string.c_str());
    
    if (PQstatus(connection) != CONNECTION_OK) {
        std::cerr << "❌ Community DB connection failed: " << PQerrorMessage(connection) << std::endl;
        PQfinish(connection);
        connection = nullptr;
        return false;
    }
    
    std::cout << "✅ Community database connected" << std::endl;
    return true;
}

void CommunityManager::disconnect() {
    if (connection) {
        PQfinish(connection);
        connection = nullptr;
    }
}

bool CommunityManager::isConnected() const {
    return connection != nullptr && PQstatus(connection) == CONNECTION_OK;
}

std::optional<Community> CommunityManager::createCommunity(const std::string& name,
                                                          const std::string& description,
                                                          const std::string& type,
                                                          const std::string& created_by) {
    if (!isConnected() && !connect()) {
        return std::nullopt;
    }
    
    std::string query = 
        "INSERT INTO communities (name, description, type, islamic_focus, created_by) "
        "VALUES ($1, $2, $3, $4, $5) "
        "RETURNING id, name, description, type, islamic_focus, created_by, created_at;";
    
    const char* params[5] = {
        name.c_str(),
        description.c_str(),
        type.c_str(),
        "general", // default islamic_focus
        created_by.c_str()
    };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 5, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "❌ Create community failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    Community community = parseCommunityFromResult(result, 0);
    PQclear(result);
    
    // Add creator as admin member
    joinCommunity(created_by, community.id);
    
    std::cout << "✅ Community created: " << name << std::endl;
    return community;
}

std::vector<Community> CommunityManager::getPublicCommunities() {
    std::vector<Community> communities;
    
    if (!isConnected() && !connect()) {
        return communities;
    }
    
    std::string query = 
        "SELECT c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at, "
        "COUNT(cm.user_id) as member_count "
        "FROM communities c "
        "LEFT JOIN community_memberships cm ON c.id = cm.community_id "
        "WHERE c.type = 'public' AND c.is_active = true "
        "GROUP BY c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at "
        "ORDER BY c.created_at DESC;";
    
    PGresult* result = PQexec(connection, query.c_str());
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "❌ Get public communities failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return communities;
    }
    
    int rows = PQntuples(result);
    for (int i = 0; i < rows; i++) {
        Community community = parseCommunityFromResult(result, i);
        
        // Get member count from the query result
        const char* count_str = PQgetvalue(result, i, 7);
        community.member_count = count_str ? std::stoi(count_str) : 0;
        
        communities.push_back(community);
    }
    
    PQclear(result);
    return communities;
}

std::vector<Community> CommunityManager::getUserCommunities(const std::string& user_id) {
    std::vector<Community> communities;
    
    if (!isConnected() && !connect()) {
        return communities;
    }
    
    std::string query = 
        "SELECT c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at, "
        "COUNT(cm2.user_id) as member_count "
        "FROM communities c "
        "INNER JOIN community_memberships cm ON c.id = cm.community_id "
        "LEFT JOIN community_memberships cm2 ON c.id = cm2.community_id "
        "WHERE cm.user_id = $1 AND c.is_active = true "
        "GROUP BY c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at "
        "ORDER BY c.created_at DESC;";
    
    const char* params[1] = { user_id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "❌ Get user communities failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return communities;
    }
    
    int rows = PQntuples(result);
    for (int i = 0; i < rows; i++) {
        Community community = parseCommunityFromResult(result, i);
        
        // Get member count from the query result
        const char* count_str = PQgetvalue(result, i, 7);
        community.member_count = count_str ? std::stoi(count_str) : 0;
        
        communities.push_back(community);
    }
    
    PQclear(result);
    return communities;
}

std::optional<Community> CommunityManager::getCommunityById(const std::string& id) {
    if (!isConnected() && !connect()) {
        return std::nullopt;
    }
    
    std::string query = 
        "SELECT c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at, "
        "COUNT(cm.user_id) as member_count "
        "FROM communities c "
        "LEFT JOIN community_memberships cm ON c.id = cm.community_id "
        "WHERE c.id = $1 AND c.is_active = true "
        "GROUP BY c.id, c.name, c.description, c.type, c.islamic_focus, c.created_by, c.created_at;";
    
    const char* params[1] = { id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "❌ Get community by ID failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return std::nullopt;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return std::nullopt;
    }
    
    Community community = parseCommunityFromResult(result, 0);
    
    // Get member count
    const char* count_str = PQgetvalue(result, 0, 7);
    community.member_count = count_str ? std::stoi(count_str) : 0;
    
    PQclear(result);
    return community;
}

bool CommunityManager::joinCommunity(const std::string& user_id, const std::string& community_id) {
    if (!isConnected() && !connect()) {
        return false;
    }
    
    // Check if already a member
    if (isUserMember(user_id, community_id)) {
        return true; // Already a member
    }
    
    std::string query = 
        "INSERT INTO community_memberships (user_id, community_id, role) "
        "VALUES ($1, $2, 'member');";
    
    const char* params[2] = { user_id.c_str(), community_id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 2, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "❌ Join community failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    std::cout << "✅ User " << user_id << " joined community " << community_id << std::endl;
    return true;
}

bool CommunityManager::leaveCommunity(const std::string& user_id, const std::string& community_id) {
    if (!isConnected() && !connect()) {
        return false;
    }
    
    std::string query = 
        "DELETE FROM community_memberships WHERE user_id = $1 AND community_id = $2;";
    
    const char* params[2] = { user_id.c_str(), community_id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 2, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "❌ Leave community failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    std::cout << "✅ User " << user_id << " left community " << community_id << std::endl;
    return true;
}

bool CommunityManager::isUserMember(const std::string& user_id, const std::string& community_id) {
    if (!isConnected() && !connect()) {
        return false;
    }
    
    std::string query = 
        "SELECT 1 FROM community_memberships WHERE user_id = $1 AND community_id = $2;";
    
    const char* params[2] = { user_id.c_str(), community_id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 2, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        PQclear(result);
        return false;
    }
    
    bool is_member = PQntuples(result) > 0;
    PQclear(result);
    return is_member;
}

std::vector<CommunityMember> CommunityManager::getCommunityMembers(const std::string& community_id) {
    std::vector<CommunityMember> members;
    
    if (!isConnected() && !connect()) {
        return members;
    }
    
    std::string query = 
        "SELECT user_id, community_id, role, joined_at "
        "FROM community_memberships "
        "WHERE community_id = $1 "
        "ORDER BY joined_at ASC;";
    
    const char* params[1] = { community_id.c_str() };
    
    PGresult* result = PQexecParams(connection, query.c_str(), 1, nullptr, params, nullptr, nullptr, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "❌ Get community members failed: " << PQerrorMessage(connection) << std::endl;
        PQclear(result);
        return members;
    }
    
    int rows = PQntuples(result);
    for (int i = 0; i < rows; i++) {
        CommunityMember member = parseMemberFromResult(result, i);
        members.push_back(member);
    }
    
    PQclear(result);
    return members;
}

Community CommunityManager::parseCommunityFromResult(PGresult* result, int row) {
    Community community;
    community.id = PQgetvalue(result, row, 0) ? PQgetvalue(result, row, 0) : "";
    community.name = PQgetvalue(result, row, 1) ? PQgetvalue(result, row, 1) : "";
    community.description = PQgetvalue(result, row, 2) ? PQgetvalue(result, row, 2) : "";
    community.type = PQgetvalue(result, row, 3) ? PQgetvalue(result, row, 3) : "";
    community.islamic_focus = PQgetvalue(result, row, 4) ? PQgetvalue(result, row, 4) : "";
    community.created_by = PQgetvalue(result, row, 5) ? PQgetvalue(result, row, 5) : "";
    community.created_at = PQgetvalue(result, row, 6) ? PQgetvalue(result, row, 6) : "";
    
    return community;
}

CommunityMember CommunityManager::parseMemberFromResult(PGresult* result, int row) {
    CommunityMember member;
    member.user_id = PQgetvalue(result, row, 0) ? PQgetvalue(result, row, 0) : "";
    member.community_id = PQgetvalue(result, row, 1) ? PQgetvalue(result, row, 1) : "";
    member.role = PQgetvalue(result, row, 2) ? PQgetvalue(result, row, 2) : "";
    member.joined_at = PQgetvalue(result, row, 3) ? PQgetvalue(result, row, 3) : "";
    
    return member;
}