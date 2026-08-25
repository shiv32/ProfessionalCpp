cmake_minimum_required(VERSION 4.0)

project(PocoRedisRestApi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Poco REQUIRED
    COMPONENTS
        Net
        JSON
        Redis
)

add_executable(PocoRedisRestApi
    main.cpp
)

target_link_libraries(PocoRedisRestApi
    PRIVATE
        Poco::Net
        Poco::JSON
        Poco::Redis
)