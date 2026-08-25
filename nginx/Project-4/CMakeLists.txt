cmake_minimum_required(VERSION 3.16)
project(RestApiServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Find required packages
find_package(Poco REQUIRED COMPONENTS Foundation Net Data DataSQLite Util JSON)
find_package(RapidJSON QUIET)
if(NOT RapidJSON_FOUND)
    find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h)
    if(RAPIDJSON_INCLUDE_DIR)
        message(STATUS "Found RapidJSON: ${RAPIDJSON_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "RapidJSON not found. Install with: sudo pacman -S rapidjson")
    endif()
endif()

# Find hiredis - manually if not found by CMake
find_package(hiredis QUIET)
if(NOT hiredis_FOUND)
    find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
    find_library(HIREDIS_LIBRARY hiredis)
    if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
        message(STATUS "Found hiredis: ${HIREDIS_LIBRARY}")
    else()
        message(FATAL_ERROR "hiredis not found. Install with: sudo pacman -S hiredis")
    endif()
else()
    get_target_property(HIREDIS_INCLUDE_DIR hiredis::hiredis INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(HIREDIS_LIBRARY hiredis::hiredis IMPORTED_LOCATION)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${HIREDIS_INCLUDE_DIR})
if(RAPIDJSON_INCLUDE_DIR)
    include_directories(${RAPIDJSON_INCLUDE_DIR})
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/RestServer.cpp
    src/RedisCache.cpp
    src/RequestHandler.cpp
)

# Create executable
add_executable(rest_server ${SOURCES})

# Link libraries
target_link_libraries(rest_server
    Poco::Foundation
    Poco::Net
    Poco::Util
    Poco::JSON
    ${HIREDIS_LIBRARY}
)

# Set output directory
set_target_properties(rest_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
