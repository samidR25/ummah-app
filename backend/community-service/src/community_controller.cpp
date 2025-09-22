#include "community_controller.h"
#include <iostream>
#include <sstream>

CommunityController::CommunityController(std::unique_ptr<CommunityManager> cm)
    : community_manager(std::move(cm)) {
    std::cout << "🏘️  Community controller initialized" << std::endl;
}

crow::response CommunityController::createCommunity(const crow::request& req) {
    try {
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        
        // Extract user ID from token (simplified - in production, validate the token)
        std::string user_id = extractUserIdFromToken(req);
        if (user_id.empty()) {
            return createErrorResponse(401, "Authentication required");
        }
        
        // Validate required fields
        if (!json_req.contains("name") || !json_req.contains("description")) {
            return createErrorResponse(400, "Missing required fields: name, description");
        }
        
        std::string name = json_req["name"];
        std::string description = json_req["description"];
        std::string type = json_req.value("type", "public");
        
        // Basic validation
        if (name.length() < 3 || name.length() > 100) {
            return createErrorResponse(400, "Community name must be 3-100 characters");
        }
        
        if (description.length() < 10 || description.length() > 500) {
            return createErrorResponse(400, "Description must be 10-500 characters");
        }
        
        // Create community
        auto community = community_manager->createCommunity(name, description, type, user_id);
        if (!community) {
            return createErrorResponse(500, "Failed to create community");
        }
        
        nlohmann::json response_data;
        response_data["message"] = "Community created successfully";
        response_data["community"] = communityToJson(*community);
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Create community error: " << e.what() << std::endl;
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response CommunityController::getPublicCommunities(const crow::request& req) {
    try {
        auto communities = community_manager->getPublicCommunities();
        
        nlohmann::json communities_json = nlohmann::json::array();
        for (const auto& community : communities) {
            communities_json.push_back(communityToJson(community));
        }
        
        nlohmann::json response_data;
        response_data["communities"] = communities_json;
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Get public communities error: " << e.what() << std::endl;
        return createErrorResponse(500, "Failed to get communities");
    }
}

crow::response CommunityController::getUserCommunities(const crow::request& req) {
    try {
        std::string user_id = extractUserIdFromToken(req);
        if (user_id.empty()) {
            return createErrorResponse(401, "Authentication required");
        }
        
        auto communities = community_manager->getUserCommunities(user_id);
        
        nlohmann::json communities_json = nlohmann::json::array();
        for (const auto& community : communities) {
            communities_json.push_back(communityToJson(community));
        }
        
        nlohmann::json response_data;
        response_data["communities"] = communities_json;
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Get user communities error: " << e.what() << std::endl;
        return createErrorResponse(500, "Failed to get user communities");
    }
}

crow::response CommunityController::joinCommunity(const crow::request& req) {
    try {
        std::string user_id = extractUserIdFromToken(req);
        if (user_id.empty()) {
            return createErrorResponse(401, "Authentication required");
        }
        
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        if (!json_req.contains("community_id")) {
            return createErrorResponse(400, "Missing community_id");
        }
        
        std::string community_id = json_req["community_id"];
        
        // Check if community exists
        auto community = community_manager->getCommunityById(community_id);
        if (!community) {
            return createErrorResponse(404, "Community not found");
        }
        
        // Join community
        if (!community_manager->joinCommunity(user_id, community_id)) {
            return createErrorResponse(500, "Failed to join community");
        }
        
        nlohmann::json response_data;
        response_data["message"] = "Successfully joined community";
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Join community error: " << e.what() << std::endl;
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response CommunityController::leaveCommunity(const crow::request& req) {
    try {
        std::string user_id = extractUserIdFromToken(req);
        if (user_id.empty()) {
            return createErrorResponse(401, "Authentication required");
        }
        
        nlohmann::json json_req = nlohmann::json::parse(req.body);
        if (!json_req.contains("community_id")) {
            return createErrorResponse(400, "Missing community_id");
        }
        
        std::string community_id = json_req["community_id"];
        
        if (!community_manager->leaveCommunity(user_id, community_id)) {
            return createErrorResponse(500, "Failed to leave community");
        }
        
        nlohmann::json response_data;
        response_data["message"] = "Successfully left community";
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Leave community error: " << e.what() << std::endl;
        return createErrorResponse(400, "Invalid request format");
    }
}

crow::response CommunityController::getCommunityDetails(const crow::request& req) {
    try {
        std::string user_id = extractUserIdFromToken(req);
        if (user_id.empty()) {
            return createErrorResponse(401, "Authentication required");
        }
        
        // Extract community ID from URL path
        std::string path = req.url;
        size_t last_slash = path.find_last_of('/');
        if (last_slash == std::string::npos) {
            return createErrorResponse(400, "Invalid URL format");
        }
        
        std::string community_id = path.substr(last_slash + 1);
        
        auto community = community_manager->getCommunityById(community_id);
        if (!community) {
            return createErrorResponse(404, "Community not found");
        }
        
        nlohmann::json response_data;
        response_data["community"] = communityToJson(*community);
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Get community details error: " << e.what() << std::endl;
        return createErrorResponse(500, "Failed to get community details");
    }
}

crow::response CommunityController::createErrorResponse(int code, const std::string& message) {
    nlohmann::json error_response;
    error_response["success"] = false;
    error_response["error"] = {
        {"code", code},
        {"message", message}
    };
    error_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return crow::response(code, error_response.dump());
}

crow::response CommunityController::createSuccessResponse(const nlohmann::json& data) {
    nlohmann::json success_response;
    success_response["success"] = true;
    success_response["data"] = data;
    success_response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return crow::response(200, success_response.dump());
}

nlohmann::json CommunityController::communityToJson(const Community& community) {
    nlohmann::json community_json;
    community_json["id"] = community.id;
    community_json["name"] = community.name;
    community_json["description"] = community.description;
    community_json["type"] = community.type;
    community_json["islamic_focus"] = community.islamic_focus;
    community_json["created_by"] = community.created_by;
    community_json["created_at"] = community.created_at;
    community_json["member_count"] = community.member_count;
    
    return community_json;
}

std::string CommunityController::extractUserIdFromToken(const crow::request& req) {
    // Simplified token extraction - in production, properly validate JWT
    std::string auth_header = req.get_header_value("Authorization");
    if (auth_header.empty()) {
        return "";
    }
    
    // For now, return a dummy user ID - you should implement proper JWT validation
    // that connects to your auth service or validates the token directly
    return "dummy-user-id-123"; // REPLACE with actual JWT validation
}

bool CommunityController::validateToken(const std::string& token) {
    // Implement proper JWT validation here
    // For MVP, we'll return true
    return !token.empty();
}