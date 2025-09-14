# Scop - Vulkan Triangle Renderer

A minimal Vulkan application that renders a colored triangle using GLFW for window management.

## Features

- Vulkan instance creation and device selection
- Swapchain and render pass setup
- Graphics pipeline with GLSL shaders
- Command buffer recording and synchronization
- Clean render loop with proper resource cleanup

## Dependencies

- CMake (>= 3.16)
- Vulkan SDK
- GLFW3
- GCC or Clang compiler

## Building

### On Ubuntu/Debian:

```bash
# Install dependencies
sudo apt update
sudo apt install -y cmake libvulkan-dev libglfw3-dev vulkan-tools glslang-tools

# Build the project
mkdir build
cd build
cmake ..
make

# Run the application
./scop
```

### Build Process

The build system automatically:
1. Compiles GLSL shaders to SPIR-V bytecode
2. Links against Vulkan and GLFW libraries
3. Creates the executable with proper dependencies

## Project Structure

```
├── src/
│   └── main.c          # Main Vulkan application
├── shaders/
│   ├── triangle.vert   # Vertex shader
│   └── triangle.frag   # Fragment shader
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

## Shaders

The application uses hardcoded vertex data in the shaders:
- **Vertex shader**: Defines triangle vertices and per-vertex colors
- **Fragment shader**: Outputs interpolated colors

## Controls

- Close window to exit the application
- No other controls - this is a minimal demonstration

## Technical Details

- Uses double buffering with 2 frames in flight
- Implements proper Vulkan synchronization
- Handles device and queue family selection
- Creates optimal swapchain configuration
- Includes comprehensive error handling and cleanup
