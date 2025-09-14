cmake_minimum_required(VERSION 3.10)
project(Scop VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
pkg_check_modules(GLFW REQUIRED glfw3)

# Include directories
include_directories(${VULKAN_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})

# Add executable
add_executable(scop src/main.c)

# Link libraries
target_link_libraries(scop ${VULKAN_LIBRARIES} ${GLFW_LIBRARIES})

# Compiler flags
target_compile_options(scop PRIVATE ${VULKAN_CFLAGS_OTHER} ${GLFW_CFLAGS_OTHER})

# Add custom target for compiling shaders
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin)

if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found")
endif()

# Compile vertex shader
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
    COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert -o ${CMAKE_CURRENT_BINARY_DIR}/vert.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    COMMENT "Compiling vertex shader"
)

# Compile fragment shader
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
    COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag -o ${CMAKE_CURRENT_BINARY_DIR}/frag.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
    COMMENT "Compiling fragment shader"
)

# Add shader compilation as dependency
add_custom_target(shaders DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vert.spv ${CMAKE_CURRENT_BINARY_DIR}/frag.spv)
add_dependencies(scop shaders)

# Set build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")