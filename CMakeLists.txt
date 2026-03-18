cmake_minimum_required(VERSION 3.26)

#set(CMAKE_OSX_SYSROOT "$(xcrun --sdk macosx --show-sdk-path)")
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "-g -Og -Wno-deprecated-volatile -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "-g -Og -Wno-deprecated-volatile -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sand_game)

add_subdirectory(libs/metal-cmake)
add_subdirectory(shaders)

set(METAL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
add_compile_options(-I${METAL_INCLUDE_DIRS})

#include(FetchContent)
#FetchContent_MakeAvailable(glm)

find_package(SDL2 CONFIG REQUIRED)
find_package(GLM REQUIRED)

file(GLOB source_files CONFIGURE_DEPENDS
        include/*.h
        src/*.cpp
    )

add_executable(sand_game 
    ${source_files}
)

target_include_directories(sand_game
    PRIVATE
    include/
    include/3rdparty
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

# Evil hackable engine
target_compile_options(sand_game PRIVATE -Wno-format-security)

target_link_libraries(sand_game
    PRIVATE 
    METAL_CPP 
    ${SDL2_LIBRARIES} 
    ${GLM_INCLUDE_DIRS}
    glm::glm
)

