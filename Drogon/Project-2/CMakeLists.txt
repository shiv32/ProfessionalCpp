cmake_minimum_required(VERSION 4.0)

project(drogon_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon REQUIRED)

add_executable(drogon_test main.cpp)

target_link_libraries(drogon_test
    PRIVATE Drogon::Drogon
)
