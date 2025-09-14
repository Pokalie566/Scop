# Scop - Vulkan Triangle

A minimal Vulkan application that renders a colored triangle using GLFW and C. This project demonstrates the fundamental concepts of Vulkan graphics programming including instance creation, device selection, swapchain setup, render passes, graphics pipelines, and the render loop.

## Features

- **Vulkan Instance**: Creates a Vulkan instance with proper validation layers (in debug builds)
- **Device Selection**: Automatically selects a suitable physical device and creates a logical device
- **Swapchain**: Sets up a swapchain for presenting rendered images to the window
- **Render Pass**: Configures a basic render pass for drawing
- **Graphics Pipeline**: Creates a complete graphics pipeline with vertex and fragment shaders
- **Command Buffers**: Records and submits rendering commands
- **Synchronization**: Implements proper synchronization using semaphores and fences
- **Window Resizing**: Handles window resize events with swapchain recreation
- **Clean Architecture**: Well-organized code with comprehensive comments

## Triangle Details

The rendered triangle features:
- **Hardcoded Vertices**: Triangle vertices are defined directly in the vertex shader
- **Gradient Colors**: Each vertex has a different color (red, green, blue) creating a smooth gradient
- **Centered Position**: Triangle is positioned in the center of the window

## Prerequisites

- **Vulkan SDK**: Required for Vulkan development
- **GLFW3**: Window and input handling library
- **CMake**: Build system (version 3.10 or higher)
- **GCC**: C compiler with C99 support
- **GLSL Compiler**: `glslangValidator` for compiling shaders

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y cmake gcc vulkan-tools vulkan-utility-libraries-dev libvulkan-dev libglfw3-dev glslang-tools
```

### Installing Dependencies (Other Systems)

- **Windows**: Install Vulkan SDK from LunarG, GLFW from official website
- **macOS**: Install using Homebrew: `brew install vulkan-sdk glfw`

## Build Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd Scop
   ```

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Build the project**:
   ```bash
   make
   ```

5. **Run the application**:
   ```bash
   ./scop
   ```

## Project Structure

```
Scop/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── src/
│   └── main.c             # Main application source code
└── shaders/
    ├── shader.vert        # Vertex shader (GLSL)
    └── shader.frag        # Fragment shader (GLSL)
```

## Shader Compilation

The build system automatically compiles GLSL shaders to SPIR-V bytecode:
- `shaders/shader.vert` → `build/vert.spv`
- `shaders/shader.frag` → `build/frag.spv`

The application loads these compiled shaders at runtime.

## Code Architecture

### Main Components

1. **Window Management** (`initWindow`): GLFW window creation and event handling
2. **Vulkan Initialization** (`initVulkan`): Complete Vulkan setup pipeline
3. **Render Loop** (`mainLoop`): Main application loop with frame rendering
4. **Cleanup** (`cleanup`): Proper resource deallocation

### Vulkan Pipeline

1. **Instance Creation**: Creates Vulkan instance with validation layers
2. **Surface Creation**: Creates window surface for rendering
3. **Device Selection**: Finds and selects suitable graphics device
4. **Swapchain Creation**: Sets up image presentation chain
5. **Render Pass**: Defines rendering operations
6. **Pipeline Creation**: Compiles shaders and creates graphics pipeline
7. **Command Recording**: Records rendering commands
8. **Frame Rendering**: Executes render loop with proper synchronization

### Key Features

- **Error Handling**: Comprehensive error checking and reporting
- **Memory Management**: Proper allocation and deallocation of resources
- **Validation Layers**: Debug builds include Vulkan validation for development
- **Swapchain Recreation**: Handles window resize events correctly
- **Multi-frame Rendering**: Implements frame-in-flight synchronization

## Debugging

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Validation Layers
Debug builds automatically enable Vulkan validation layers for enhanced debugging and error detection.

### Common Issues

1. **No suitable GPU found**: Ensure Vulkan drivers are installed
2. **Shader compilation errors**: Check that `glslangValidator` is available
3. **Window creation fails**: Verify GLFW installation and display server

## Learning Resources

This project demonstrates fundamental Vulkan concepts suitable for learning:

- Vulkan instance and device management
- Swapchain and presentation
- Render passes and framebuffers
- Graphics pipeline creation
- Command buffer recording
- Synchronization primitives
- Resource management

## Extensions

This basic triangle can be extended with:
- Vertex buffers for complex geometry
- Uniform buffers for transformations
- Texture mapping
- Depth testing
- Multiple render passes
- Compute shaders

## License

This project is a learning implementation for educational purposes.
