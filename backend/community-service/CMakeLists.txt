cmake_minimum_required(VERSION 3.15)
project(UmmahCommunityService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

# Find nlohmann/json and Crow
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    if(EXISTS "/usr/include/nlohmann/json.hpp")
        set(NLOHMANN_JSON_INCLUDE_DIRS "/usr/include")
    endif()
endif()

find_package(Crow QUIET)
if(NOT Crow_FOUND)
    find_path(CROW_INCLUDE_DIR crow.h PATHS /usr/local/include /usr/include)
    if(CROW_INCLUDE_DIR)
        set(CROW_INCLUDE_DIRS ${CROW_INCLUDE_DIR})
    endif()
endif()

# Include directories
include_directories(include)
include_directories(../common/include)
include_directories(${NLOHMANN_JSON_INCLUDE_DIRS})
include_directories(${CROW_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/community_controller.cpp
    src/community_manager.cpp
)

# Create executable
add_executable(ummah_community_service ${SOURCES})

# Link libraries
target_link_libraries(ummah_community_service 
    ${LIBPQ_LIBRARIES}
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
    pthread
)

target_compile_options(ummah_community_service PRIVATE ${LIBPQ_CFLAGS_OTHER})