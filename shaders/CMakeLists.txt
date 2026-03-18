# Set the project name and minimum required CMake version
cmake_minimum_required(VERSION 3.16)
project(3DGameShaders)

# Set paths for the shaders and headers
set(SHADER_SOURCE_DIR "${PROJECT_SOURCE_DIR}") # Current directory, no additional /shaders
set(AIR_OUTPUT_DIR "${PROJECT_BINARY_DIR}/air") # Output directory for .air files
set(METALLIB_OUTPUT "${PROJECT_BINARY_DIR}/lib/MyShaders.metallib") # Output for metallib

# Create directories for output if they don't exist
file(MAKE_DIRECTORY ${AIR_OUTPUT_DIR}) # Ensure air output directory exists
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/lib) # Ensure lib output directory exists

# Add include directory to ensure the Metal compiler can find headers
include_directories(${SHADER_SOURCE_DIR})

# Specify shader files 
set(SHADER_FILES
    "${SHADER_SOURCE_DIR}/sand_shader.metal"
)

set(SHADER_HEADER_FILES
    # Add header files here when needed
)

# Check for the existence of each shader file
foreach(SHADER_FILE ${SHADER_FILES})
    if(EXISTS ${SHADER_FILE})
        message("Found shader file: ${SHADER_FILE}")
    else()
        message(FATAL_ERROR "Shader file missing: ${SHADER_FILE}")
    endif()
endforeach()

# Collect all compiled .air files
set(AIR_FILES)

# Loop through each shader file and compile it
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE) # Get the name without extension
    set(AIR_FILE ${AIR_OUTPUT_DIR}/${SHADER_NAME}.air) # Output file for each shader

    add_custom_command(
        OUTPUT ${AIR_FILE}
        COMMAND xcrun metal -I ${SHADER_SOURCE_DIR} -c ${SHADER_FILE} -o ${AIR_FILE} # Compile the shader
        DEPENDS ${SHADER_FILE} ${SHADER_HEADER_FILES}
        COMMENT "Compiling ${SHADER_FILE} to ${AIR_FILE}"
    )

    list(APPEND AIR_FILES ${AIR_FILE}) # Add the generated .air file to the list
endforeach()

# Link all .air files into a single .metallib
add_custom_command(
    OUTPUT ${METALLIB_OUTPUT}
    COMMAND xcrun metallib ${AIR_FILES} -o ${METALLIB_OUTPUT} # Link all .air files
    DEPENDS ${AIR_FILES}
    COMMENT "Linking .air files to create ${METALLIB_OUTPUT}"
)

# Custom target to build shaders
add_custom_target(MetalLibrary ALL DEPENDS ${METALLIB_OUTPUT}) # Ensure this target builds with the project

# Debug output
message("Shader include directory: ${SHADER_SOURCE_DIR}")

