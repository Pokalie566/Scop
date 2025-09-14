#!/bin/bash

# Build test script for Scop Vulkan Triangle
# This script verifies that the project builds correctly

set -e  # Exit on any error

echo "=== Scop Build Test ==="
echo

# Check for required tools
echo "Checking for required tools..."
command -v cmake >/dev/null 2>&1 || { echo "cmake is required but not installed. Aborting." >&2; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo "gcc is required but not installed. Aborting." >&2; exit 1; }
command -v glslangValidator >/dev/null 2>&1 || { echo "glslangValidator is required but not installed. Aborting." >&2; exit 1; }
echo "✓ All required tools found"
echo

# Check for required packages
echo "Checking for Vulkan and GLFW development packages..."
pkg-config --exists vulkan || { echo "Vulkan development package not found. Install libvulkan-dev" >&2; exit 1; }
pkg-config --exists glfw3 || { echo "GLFW3 development package not found. Install libglfw3-dev" >&2; exit 1; }
echo "✓ All required packages found"
echo

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
    echo "✓ Previous build cleaned"
    echo
fi

# Create build directory
echo "Creating build directory..."
mkdir build
cd build
echo "✓ Build directory created"
echo

# Configure with CMake
echo "Configuring with CMake..."
cmake .. || { echo "CMake configuration failed" >&2; exit 1; }
echo "✓ CMake configuration successful"
echo

# Build the project
echo "Building the project..."
make || { echo "Build failed" >&2; exit 1; }
echo "✓ Build successful"
echo

# Verify outputs
echo "Verifying build outputs..."
[ -f "scop" ] || { echo "Executable 'scop' not found" >&2; exit 1; }
[ -f "vert.spv" ] || { echo "Vertex shader 'vert.spv' not found" >&2; exit 1; }
[ -f "frag.spv" ] || { echo "Fragment shader 'frag.spv' not found" >&2; exit 1; }
echo "✓ All expected files created"
echo

# Check file types
echo "Checking file types..."
file scop | grep -q "ELF.*executable" || { echo "scop is not a valid executable" >&2; exit 1; }
file vert.spv | grep -q "SPIR-V" || { echo "vert.spv is not a valid SPIR-V file" >&2; exit 1; }
file frag.spv | grep -q "SPIR-V" || { echo "frag.spv is not a valid SPIR-V file" >&2; exit 1; }
echo "✓ All files have correct types"
echo

echo "=== Build Test Complete ==="
echo "🎉 All tests passed! The Vulkan triangle project builds successfully."
echo
echo "To run the application (requires a display):"
echo "  ./scop"
echo
echo "Note: The application requires a Vulkan-capable GPU and display to run."