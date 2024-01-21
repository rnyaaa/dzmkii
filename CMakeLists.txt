cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 17)

project(DZMKII)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(metal-cmake)

find_package(SDL2 REQUIRED)
include_directories(
    include/
    ${SDL2_INCLUDE_DIRS}
    )

add_executable(DZMKII 
    src/main.cpp
    src/renderer.cpp)

target_link_libraries(DZMKII METAL_CPP ${SDL2_LIBRARIES})
