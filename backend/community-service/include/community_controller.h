#ifndef COMMUNITY_CONTROLLER_H
#define COMMUNITY_CONTROLLER_H

#include <crow.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "community_manager.h"

class CommunityController {
private:
    std::unique_ptr<CommunityManager> community_manager;
    
public:
    CommunityController(std::unique_ptr<CommunityManager> cm);
    
    // Route handlers
    crow::response createCommunity(const crow::request& req);
    crow::response getPublicCommunities(const crow::request& req);
    crow::response getUserCommunities(const crow::request& req);
    crow::response joinCommunity(const crow::request& req);
    crow::response leaveCommunity(const crow::request& req);
    crow::response getCommunityDetails(const crow::request& req);
    
private:
    crow::response createErrorResponse(int code, const std::string& message);
    crow::response createSuccessResponse(const nlohmann::json& data);
    nlohmann::json communityToJson(const Community& community);
    std::string extractUserIdFromToken(const crow::request& req);
    bool validateToken(const std::string& token);
};

#endif