cmake_minimum_required(VERSION 4.0)

project(PocoRestApi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED
    COMPONENTS
    Net
    JSON
)

add_executable(PocoRestApi
    main2.cpp
)

target_link_libraries(PocoRestApi
    PRIVATE
    Poco::Net
    Poco::JSON
)
