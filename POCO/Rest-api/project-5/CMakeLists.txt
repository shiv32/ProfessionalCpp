cmake_minimum_required(VERSION 4.0)

project(PocoJwtRestApi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED COMPONENTS
    Net
    Foundation
    JSON
    JWT
)

add_executable(server
    main.cpp
)

target_link_libraries(server
    PRIVATE
    Poco::Net
    Poco::Foundation
    Poco::JSON
    Poco::JWT
)
