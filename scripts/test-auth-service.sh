#!/bin/bash

echo "🧪 Testing Ummah Auth Service..."

BASE_URL="http://localhost:8080"
AUTH_TOKEN=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test function
test_endpoint() {
    local method=$1
    local endpoint=$2
    local data=$3
    local expected_status=$4
    local description=$5
    
    echo -e "\n${YELLOW}Testing: $description${NC}"
    echo "  $method $endpoint"
    
    if [ "$method" = "GET" ]; then
        if [ -n "$AUTH_TOKEN" ]; then
            response=$(curl -s -w "HTTPSTATUS:%{http_code}" \
                -H "Authorization: Bearer $AUTH_TOKEN" \
                "$BASE_URL$endpoint")
        else
            response=$(curl -s -w "HTTPSTATUS:%{http_code}" "$BASE_URL$endpoint")
        fi
    else
        if [ -n "$AUTH_TOKEN" ]; then
            response=$(curl -s -w "HTTPSTATUS:%{http_code}" \
                -X "$method" \
                -H "Content-Type: application/json" \
                -H "Authorization: Bearer $AUTH_TOKEN" \
                -d "$data" \
                "$BASE_URL$endpoint")
        else
            response=$(curl -s -w "HTTPSTATUS:%{http_code}" \
                -X "$method" \
                -H "Content-Type: application/json" \
                -d "$data" \
                "$BASE_URL$endpoint")
        fi
    fi
    
    status=$(echo "$response" | grep -o "HTTPSTATUS:[0-9]*" | cut -d: -f2)
    body=$(echo "$response" | sed 's/HTTPSTATUS:[0-9]*$//')
    
    if [ "$status" = "$expected_status" ]; then
        echo -e "  ${GREEN}✅ PASS${NC} (Status: $status)"
    else
        echo -e "  ${RED}❌ FAIL${NC} (Expected: $expected_status, Got: $status)"
        echo "  Response: $body"
        return 1
    fi
    
    # Extract token from registration/login responses
    if [[ "$endpoint" == *"/register"* ]] || [[ "$endpoint" == *"/login"* ]]; then
        AUTH_TOKEN=$(echo "$body" | grep -o '"token":"[^"]*' | cut -d'"' -f4)
        if [ -n "$AUTH_TOKEN" ]; then
            echo "  🔐 Token extracted for future requests"
        fi
    fi
    
    return 0
}

# Check if service is running
echo "🔍 Checking if auth service is running..."
if ! curl -s "$BASE_URL/health" > /dev/null; then
    echo -e "${RED}❌ Auth service is not running on $BASE_URL${NC}"
    echo "Please start the service first: ./backend/auth-service/build/ummah_auth_service"
    exit 1
fi

echo -e "${GREEN}✅ Auth service is running${NC}"

# Test health endpoint
test_endpoint "GET" "/health" "" "200" "Health check"

# Test API info endpoint
test_endpoint "GET" "/api/info" "" "200" "API information"

# Test user registration
test_endpoint "POST" "/api/auth/register" \
    '{"username":"testuser123","email":"test@example.com","password":"TestPass123!","gender":"male"}' \
    "200" "User registration"

# Test duplicate registration (should fail)
test_endpoint "POST" "/api/auth/register" \
    '{"username":"testuser123","email":"test@example.com","password":"TestPass123!","gender":"male"}' \
    "409" "Duplicate user registration (should fail)"

# Test invalid email registration
test_endpoint "POST" "/api/auth/register" \
    '{"username":"testuser456","email":"invalid-email","password":"TestPass123!","gender":"male"}' \
    "400" "Invalid email format (should fail)"

# Test weak password registration
test_endpoint "POST" "/api/auth/register" \
    '{"username":"testuser789","email":"test2@example.com","password":"weak","gender":"female"}' \
    "400" "Weak password (should fail)"

# Test user login
test_endpoint "POST" "/api/auth/login" \
    '{"email":"test@example.com","password":"TestPass123!"}' \
    "200" "User login"

# Test invalid login
test_endpoint "POST" "/api/auth/login" \
    '{"email":"test@example.com","password":"wrongpassword"}' \
    "401" "Invalid password (should fail)"

# Test get profile (requires auth token from login)
test_endpoint "GET" "/api/auth/profile" "" "200" "Get user profile"

# Test update profile
test_endpoint "PUT" "/api/auth/profile" \
    '{"location":"Mecca, Saudi Arabia","prayer_reminders":true,"gender_interaction_preference":"same_gender_only"}' \
    "200" "Update user profile"

# Test token refresh
test_endpoint "POST" "/api/auth/refresh" "" "200" "Token refresh"

# Test unauthorized access (clear token)
AUTH_TOKEN=""
test_endpoint "GET" "/api/auth/profile" "" "401" "Unauthorized access (should fail)"

echo -e "\n${GREEN}🎉 All tests completed!${NC}"
echo "📊 Check the results above for any failures"