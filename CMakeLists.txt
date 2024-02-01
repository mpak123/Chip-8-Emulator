cmake_minimum_required(VERSION 3.14)
project(chip8)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(modules/glad EXCLUDE_FROM_ALL)
add_subdirectory(modules/SDL-release-2.0.20 EXCLUDE_FROM_ALL)

add_executable(
        chip8
        src/Chip8.cpp
        src/Main.cpp
        src/Platform.cpp)

target_compile_options(chip8 PRIVATE -Wall)

target_link_libraries(chip8 PRIVATE glad SDL2)


