cmake_minimum_required(VERSION 4.0)

# Set the compiler
set(CMAKE_CXX_STANDARD 17)

# Set the project name
project(mm)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    main.cpp
)

# Add an executable with the above sources
add_executable(mm ${SOURCES})
