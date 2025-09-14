cmake_minimum_required(VERSION 3.16)
project(Scop)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)

pkg_check_modules(GLFW3 REQUIRED glfw3)

# Set executable name
set(EXECUTABLE_NAME scop)

# Source files
set(SOURCES
    src/main.c
)

# Create executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Include directories
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${EXECUTABLE_NAME}
    ${Vulkan_LIBRARIES}
    ${GLFW3_LIBRARIES}
)

# Compiler flags
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    ${GLFW3_CFLAGS_OTHER}
    -Wall -Wextra -std=c99
)

# Custom target to compile shaders
add_custom_target(shaders
    COMMAND glslangValidator -V ${CMAKE_SOURCE_DIR}/shaders/triangle.vert -o ${CMAKE_BINARY_DIR}/triangle_vert.spv
    COMMAND glslangValidator -V ${CMAKE_SOURCE_DIR}/shaders/triangle.frag -o ${CMAKE_BINARY_DIR}/triangle_frag.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/triangle.vert ${CMAKE_SOURCE_DIR}/shaders/triangle.frag
    COMMENT "Compiling shaders"
)

# Make sure shaders are compiled before building the executable
add_dependencies(${EXECUTABLE_NAME} shaders)