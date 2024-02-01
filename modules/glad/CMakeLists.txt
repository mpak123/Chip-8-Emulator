# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Define the project and language
project(glad C)

# Add source files and header files
set(SOURCES
        src/gl.c
        )

# Add header files
set(HEADERS
        include/glad/gl.h
        )

# Create a library target
add_library(glad STATIC ${SOURCES} ${HEADERS})

# Specify include directories
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
