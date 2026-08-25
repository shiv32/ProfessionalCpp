cmake_minimum_required(VERSION 4.0)

# Set the project name
project(nginx)

# Set the compiler
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Poco REQUIRED Net Util JSON)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES main.cpp)

# Add an executable with the above sources
add_executable(nginx ${SOURCES})

target_link_libraries(nginx Poco::Net Poco::Util Poco::JSON hiredis)
