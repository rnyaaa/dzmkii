cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(DZMKII)

option(METAL_CPP_BUILD_EXAMPLES "Build examples" OFF)
add_subdirectory(libs/metal-cpp-cmake)
add_subdirectory(libs/assimp)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/0.9.9.8
)

FetchContent_MakeAvailable(glm)

find_package(SDL2 CONFIG REQUIRED)

file(GLOB source_files CONFIGURE_DEPENDS
        include/*.h
        src/*.cpp
    )

add_executable(DZMKII 
    ${source_files}
)

target_include_directories(DZMKII
    PRIVATE
    include/
    include/3rdparty
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    libs/assimp/include
)

# Evil hackable engine
target_compile_options(DZMKII PRIVATE -Wno-format-security)

target_link_libraries(DZMKII
    PRIVATE 
    METAL_CPP 
    ${SDL2_LIBRARIES} 
    glm::glm
    assimp
)

