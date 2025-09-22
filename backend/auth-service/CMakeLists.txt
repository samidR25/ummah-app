cmake_minimum_required(VERSION 3.15)
project(UmmahAuthService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "🕌 Configuring Ummah Auth Service")
message(STATUS "📁 Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "📁 Binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

# Find packages
message(STATUS "🔍 Finding packages...")

find_package(PkgConfig REQUIRED)
message(STATUS "✅ PkgConfig found")

find_package(Threads REQUIRED)
message(STATUS "✅ Threads found")

find_package(OpenSSL REQUIRED)
message(STATUS "✅ OpenSSL found: ${OPENSSL_VERSION}")

# Find PostgreSQL
pkg_check_modules(LIBPQ REQUIRED libpq)
message(STATUS "✅ PostgreSQL found: ${LIBPQ_VERSION}")

# Find nlohmann/json
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "✅ nlohmann/json found via find_package")
else()
    # Fallback to manual check
    if(EXISTS "/usr/include/nlohmann/json.hpp")
        message(STATUS "✅ nlohmann/json found: /usr/include")
        set(NLOHMANN_JSON_INCLUDE_DIRS "/usr/include")
    else()
        message(FATAL_ERROR "❌ nlohmann/json not found. Install with: sudo apt install nlohmann-json3-dev")
    endif()
endif()

# Find Crow
find_package(Crow QUIET)
if(Crow_FOUND)
    message(STATUS "✅ Crow found via find_package")
    set(CROW_LIBRARIES Crow::Crow)
else()
    # Check for manual Crow installation
    find_path(CROW_INCLUDE_DIR crow.h PATHS /usr/local/include /usr/include)
    find_library(CROW_LIBRARY NAMES crow PATHS /usr/local/lib /usr/lib)
    
    if(CROW_INCLUDE_DIR)
        message(STATUS "✅ Crow headers found: ${CROW_INCLUDE_DIR}")
        # Crow is typically header-only, no library needed
        set(CROW_LIBRARIES "")
        set(CROW_INCLUDE_DIRS ${CROW_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "❌ Crow framework not found. Run: ./scripts/install-dependencies.sh")
    endif()
endif()

# FIXED: Correct path resolution
message(STATUS "🔍 Checking include directories...")

# Use absolute paths to avoid confusion
get_filename_component(COMMON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/include" ABSOLUTE)
get_filename_component(AUTH_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" ABSOLUTE)

message(STATUS "🔍 Common include path: ${COMMON_INCLUDE_DIR}")
message(STATUS "🔍 Auth include path: ${AUTH_INCLUDE_DIR}")

# Verify directories exist
if(NOT EXISTS "${COMMON_INCLUDE_DIR}")
    message(FATAL_ERROR "❌ Common include directory not found: ${COMMON_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${AUTH_INCLUDE_DIR}")
    message(FATAL_ERROR "❌ Auth include directory not found: ${AUTH_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${COMMON_INCLUDE_DIR}/config.h")
    message(FATAL_ERROR "❌ config.h not found at: ${COMMON_INCLUDE_DIR}/config.h")
endif()

message(STATUS "✅ Include directories verified")

# Set include directories
include_directories(${AUTH_INCLUDE_DIR})
include_directories(${COMMON_INCLUDE_DIR})
include_directories(${LIBPQ_INCLUDE_DIRS})

if(CROW_INCLUDE_DIRS)
    include_directories(${CROW_INCLUDE_DIRS})
endif()

if(NLOHMANN_JSON_INCLUDE_DIRS)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/auth_controller.cpp
    src/database_manager.cpp
    src/jwt_manager.cpp
    src/password_manager.cpp
)

# Verify source files exist
message(STATUS "🔍 Checking source files...")
foreach(SOURCE ${SOURCES})
    get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}" ABSOLUTE)
    if(NOT EXISTS "${SOURCE_PATH}")
        message(FATAL_ERROR "❌ Source file not found: ${SOURCE_PATH}")
    endif()
endforeach()
message(STATUS "✅ All source files found")

# Verify header files exist
set(HEADERS
    include/auth_controller.h
    include/database_manager.h
    include/jwt_manager.h
    include/password_manager.h
)

message(STATUS "🔍 Checking header files...")
foreach(HEADER ${HEADERS})
    get_filename_component(HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}" ABSOLUTE)
    if(NOT EXISTS "${HEADER_PATH}")
        message(FATAL_ERROR "❌ Header file not found: ${HEADER_PATH}")
    endif()
endforeach()
message(STATUS "✅ All header files found")

# Create executable
add_executable(ummah_auth_service ${SOURCES})

# Compiler definitions
target_compile_definitions(ummah_auth_service PRIVATE 
    CROW_ENABLE_SSL
)

# Link libraries
target_link_libraries(ummah_auth_service 
    ${LIBPQ_LIBRARIES}
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
    pthread
)

# Add Crow library if available
if(CROW_LIBRARIES)
    target_link_libraries(ummah_auth_service ${CROW_LIBRARIES})
endif()

# Add nlohmann/json if found via find_package
if(nlohmann_json_FOUND)
    target_link_libraries(ummah_auth_service nlohmann_json::nlohmann_json)
endif()

# Compiler flags
target_compile_options(ummah_auth_service PRIVATE ${LIBPQ_CFLAGS_OTHER})

# Set output directory
set_target_properties(ummah_auth_service PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

message(STATUS "✅ Configuration complete!")