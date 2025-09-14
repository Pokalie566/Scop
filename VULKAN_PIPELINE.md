# Vulkan Pipeline Documentation

This document explains the Vulkan rendering pipeline implemented in the Scop triangle project.

## Overview

The application implements a complete Vulkan rendering pipeline that displays a colored triangle. The implementation follows Vulkan best practices and demonstrates fundamental concepts.

## Core Components

### 1. Application Structure (`VulkanApp`)

```c
typedef struct {
    GLFWwindow* window;              // GLFW window handle
    VkInstance instance;             // Vulkan instance
    VkPhysicalDevice physicalDevice; // Selected GPU
    VkDevice device;                 // Logical device
    VkQueue graphicsQueue;           // Graphics command queue
    VkQueue presentQueue;            // Presentation queue
    VkSurfaceKHR surface;           // Window surface
    VkSwapchainKHR swapchain;       // Image presentation chain
    // ... other components
} VulkanApp;
```

### 2. Initialization Pipeline

The initialization follows this exact order (critical for Vulkan):

1. **Window Creation** → Creates GLFW window
2. **Instance Creation** → Establishes Vulkan context
3. **Surface Creation** → Links window to Vulkan
4. **Device Selection** → Chooses suitable GPU
5. **Logical Device** → Creates device interface
6. **Swapchain** → Sets up image presentation
7. **Image Views** → Creates views into swapchain images
8. **Render Pass** → Defines rendering operations
9. **Pipeline** → Compiles shaders and creates graphics pipeline
10. **Framebuffers** → Creates render targets
11. **Command Pool** → Allocates command memory
12. **Command Buffers** → Creates command recording buffers
13. **Sync Objects** → Creates semaphores and fences

## Key Vulkan Concepts

### Instance and Validation Layers

```c
// Debug builds enable validation for error checking
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
```

Validation layers provide detailed error messages and debugging information during development.

### Device Selection

The application automatically selects a GPU that supports:
- Graphics operations (`VK_QUEUE_GRAPHICS_BIT`)
- Surface presentation (window rendering)
- Required extensions (`VK_KHR_swapchain`)

### Swapchain

The swapchain manages multiple images for smooth rendering:
- **Double/Triple buffering** for smooth animation
- **Format selection** (prefers `VK_FORMAT_B8G8R8A8_SRGB`)
- **Present mode** (prefers mailbox for reduced latency)
- **Automatic recreation** on window resize

### Render Pass

Defines how rendering operations are structured:
```c
VkAttachmentDescription colorAttachment = {
    .format = swapchainImageFormat,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,  // Clear before rendering
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // Store result
    // ...
};
```

### Graphics Pipeline

The pipeline processes vertices through these stages:

1. **Vertex Shader** (`shader.vert`)
   - Hardcoded triangle vertices
   - Per-vertex color assignment
   - Transforms to clip space

2. **Rasterization**
   - Converts triangles to pixels
   - Interpolates vertex attributes

3. **Fragment Shader** (`shader.frag`)
   - Receives interpolated colors
   - Outputs final pixel colors

### Command Recording

Commands are recorded into buffers and submitted to GPU:
```c
vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
vkCmdDraw(commandBuffer, 3, 1, 0, 0);  // Draw 3 vertices, 1 instance
vkCmdEndRenderPass(commandBuffer);
```

### Synchronization

Multiple frames can be "in-flight" simultaneously using:
- **Semaphores**: GPU-GPU synchronization
  - `imageAvailableSemaphore`: Swapchain image ready
  - `renderFinishedSemaphore`: Rendering complete
- **Fences**: CPU-GPU synchronization
  - `inFlightFence`: Frame completion signal

## Shader Details

### Vertex Shader (`shader.vert`)

```glsl
// Hardcoded triangle vertices (no vertex buffers)
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Bottom
    vec2(0.5, 0.5),    // Top right  
    vec2(-0.5, 0.5)    // Top left
);

// Per-vertex colors
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // Red
    vec3(0.0, 1.0, 0.0), // Green
    vec3(0.0, 0.0, 1.0)  // Blue
);
```

Uses `gl_VertexIndex` to select vertex data without vertex buffers.

### Fragment Shader (`shader.frag`)

```glsl
layout(location = 0) in vec3 fragColor;    // Interpolated color
layout(location = 0) out vec4 outColor;    // Final pixel color

void main() {
    outColor = vec4(fragColor, 1.0);       // RGB + full alpha
}
```

## Memory Management

The application properly manages all Vulkan resources:
- **Creation order**: Dependencies created first
- **Destruction order**: Reverse of creation order
- **No memory leaks**: All allocations freed
- **Swapchain recreation**: Handles window resize

## Error Handling

Comprehensive error checking:
- All Vulkan functions checked for success
- Meaningful error messages
- Graceful failure handling
- Debug validation in development builds

## Performance Considerations

- **Multi-frame rendering**: 2 frames in flight
- **Efficient synchronization**: Minimal CPU-GPU stalls
- **Optimal present mode**: Mailbox for reduced latency
- **Proper resource reuse**: Command buffers reset, not recreated

This implementation serves as a solid foundation for more complex Vulkan applications while demonstrating all essential concepts correctly.