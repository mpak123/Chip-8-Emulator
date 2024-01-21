cmake_minimum_required(VERSION 3.14)
project(chip8)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(modules/glad EXCLUDE_FROM_ALL)
add_subdirectory(modules/SDL EXCLUDE_FROM_ALL)
add_subdirectory(modules/imgui-1.88 EXCLUDE_FROM_ALL)

add_executable(
        chip8
        Source/Chip8.cpp
        Source/Main.cpp
        Source/Platform.cpp)

target_compile_options(chip8 PRIVATE -Wall)

target_link_libraries(chip8 PRIVATE glad SDL imgui)


