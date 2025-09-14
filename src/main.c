#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Window dimensions
#define WIDTH 800
#define HEIGHT 600

// Maximum number of frames in flight
#define MAX_FRAMES_IN_FLIGHT 2

// Validation layers for debugging
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Application structure
typedef struct {
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    uint32_t swapchainImageCount;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkImageView* swapchainImageViews;
    VkFramebuffer* swapchainFramebuffers;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    size_t currentFrame;
    bool framebufferResized;
} VulkanApp;

// Queue family indices
typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool hasGraphicsFamily;
    bool hasPresentFamily;
} QueueFamilyIndices;

// Swapchain support details
typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formatCount;
    VkPresentModeKHR* presentModes;
    uint32_t presentModeCount;
} SwapchainSupportDetails;

// Function declarations
void initWindow(VulkanApp* app);
void initVulkan(VulkanApp* app);
void mainLoop(VulkanApp* app);
void cleanup(VulkanApp* app);
void createInstance(VulkanApp* app);
void pickPhysicalDevice(VulkanApp* app);
void createLogicalDevice(VulkanApp* app);
void createSurface(VulkanApp* app);
void createSwapchain(VulkanApp* app);
void createImageViews(VulkanApp* app);
void createRenderPass(VulkanApp* app);
void createGraphicsPipeline(VulkanApp* app);
void createFramebuffers(VulkanApp* app);
void createCommandPool(VulkanApp* app);
void createCommandBuffers(VulkanApp* app);
void createSyncObjects(VulkanApp* app);
void drawFrame(VulkanApp* app);
void recreateSwapchain(VulkanApp* app);
void cleanupSwapchain(VulkanApp* app);

// Helper functions
bool checkValidationLayerSupport();
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* formats, uint32_t formatCount);
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* presentModes, uint32_t presentModeCount);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* window);
VkShaderModule createShaderModule(VkDevice device, const char* filename);
char* readFile(const char* filename, size_t* size);

// GLFW callbacks
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

int main() {
    VulkanApp app = {0};
    
    initWindow(&app);
    initVulkan(&app);
    mainLoop(&app);
    cleanup(&app);
    
    return 0;
}

void initWindow(VulkanApp* app) {
    // Initialize GLFW
    glfwInit();
    
    // Don't create OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    // Create window
    app->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, framebufferResizeCallback);
}

void initVulkan(VulkanApp* app) {
    createInstance(app);
    createSurface(app);
    pickPhysicalDevice(app);
    createLogicalDevice(app);
    createSwapchain(app);
    createImageViews(app);
    createRenderPass(app);
    createGraphicsPipeline(app);
    createFramebuffers(app);
    createCommandPool(app);
    createCommandBuffers(app);
    createSyncObjects(app);
}

void mainLoop(VulkanApp* app) {
    while (!glfwWindowShouldClose(app->window)) {
        glfwPollEvents();
        drawFrame(app);
    }
    
    vkDeviceWaitIdle(app->device);
}

void cleanup(VulkanApp* app) {
    cleanupSwapchain(app);
    
    // Cleanup sync objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(app->device, app->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(app->device, app->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(app->device, app->inFlightFences[i], NULL);
    }
    free(app->imageAvailableSemaphores);
    free(app->renderFinishedSemaphores);
    free(app->inFlightFences);
    
    vkDestroyCommandPool(app->device, app->commandPool, NULL);
    vkDestroyDevice(app->device, NULL);
    vkDestroySurfaceKHR(app->instance, app->surface, NULL);
    vkDestroyInstance(app->instance, NULL);
    
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

void createInstance(VulkanApp* app) {
    // Check validation layer support
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr, "Validation layers requested, but not available!\n");
        exit(EXIT_FAILURE);
    }
    
    // Application info
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Scop";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    // Instance create info
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    // Get required extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    
    // Enable validation layers if needed
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create instance
    if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create instance!\n");
        exit(EXIT_FAILURE);
    }
}

void createSurface(VulkanApp* app) {
    if (glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create window surface!\n");
        exit(EXIT_FAILURE);
    }
}

void pickPhysicalDevice(VulkanApp* app) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
    
    if (deviceCount == 0) {
        fprintf(stderr, "Failed to find GPUs with Vulkan support!\n");
        exit(EXIT_FAILURE);
    }
    
    VkPhysicalDevice* devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices);
    
    app->physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], app->surface)) {
            app->physicalDevice = devices[i];
            break;
        }
    }
    
    free(devices);
    
    if (app->physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "Failed to find a suitable GPU!\n");
        exit(EXIT_FAILURE);
    }
}

void createLogicalDevice(VulkanApp* app) {
    QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app->surface);
    
    // Create queue create infos
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    uint32_t uniqueQueueFamilies[] = {indices.graphicsFamily, indices.presentFamily};
    uint32_t queueCreateInfoCount = (indices.graphicsFamily == indices.presentFamily) ? 1 : 2;
    
    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < queueCreateInfoCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }
    
    // Device features
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    
    // Device create info
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create logical device
    if (vkCreateDevice(app->physicalDevice, &createInfo, NULL, &app->device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device!\n");
        exit(EXIT_FAILURE);
    }
    
    // Get queue handles
    vkGetDeviceQueue(app->device, indices.graphicsFamily, 0, &app->graphicsQueue);
    vkGetDeviceQueue(app->device, indices.presentFamily, 0, &app->presentQueue);
}

void createSwapchain(VulkanApp* app) {
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(app->physicalDevice, app->surface);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats, swapchainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, swapchainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(&swapchainSupport.capabilities, app->window);
    
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = app->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = findQueueFamilies(app->physicalDevice, app->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
    
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }
    
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(app->device, &createInfo, NULL, &app->swapchain) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create swap chain!\n");
        exit(EXIT_FAILURE);
    }
    
    // Get swapchain images
    vkGetSwapchainImagesKHR(app->device, app->swapchain, &app->swapchainImageCount, NULL);
    app->swapchainImages = malloc(app->swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(app->device, app->swapchain, &app->swapchainImageCount, app->swapchainImages);
    
    app->swapchainImageFormat = surfaceFormat.format;
    app->swapchainExtent = extent;
    
    // Cleanup support details
    free(swapchainSupport.formats);
    free(swapchainSupport.presentModes);
}

void createImageViews(VulkanApp* app) {
    app->swapchainImageViews = malloc(app->swapchainImageCount * sizeof(VkImageView));
    
    for (uint32_t i = 0; i < app->swapchainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app->swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app->swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(app->device, &createInfo, NULL, &app->swapchainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create image views!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createRenderPass(VulkanApp* app) {
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = app->swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(app->device, &renderPassInfo, NULL, &app->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render pass!\n");
        exit(EXIT_FAILURE);
    }
}

void createGraphicsPipeline(VulkanApp* app) {
    // Load shaders
    VkShaderModule vertShaderModule = createShaderModule(app->device, "vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(app->device, "frag.spv");
    
    // Vertex shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    // Fragment shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input (no vertex buffers since we hardcode vertices)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport and scissor
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) app->swapchainExtent.width;
    viewport.height = (float) app->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = app->swapchainExtent;
    
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    
    if (vkCreatePipelineLayout(app->device, &pipelineLayoutInfo, NULL, &app->pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create pipeline layout!\n");
        exit(EXIT_FAILURE);
    }
    
    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = NULL;
    pipelineInfo.layout = app->pipelineLayout;
    pipelineInfo.renderPass = app->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    if (vkCreateGraphicsPipelines(app->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &app->graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline!\n");
        exit(EXIT_FAILURE);
    }
    
    // Cleanup shader modules
    vkDestroyShaderModule(app->device, fragShaderModule, NULL);
    vkDestroyShaderModule(app->device, vertShaderModule, NULL);
}

void createFramebuffers(VulkanApp* app) {
    app->swapchainFramebuffers = malloc(app->swapchainImageCount * sizeof(VkFramebuffer));
    
    for (uint32_t i = 0; i < app->swapchainImageCount; i++) {
        VkImageView attachments[] = {
            app->swapchainImageViews[i]
        };
        
        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = app->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = app->swapchainExtent.width;
        framebufferInfo.height = app->swapchainExtent.height;
        framebufferInfo.layers = 1;
        
        if (vkCreateFramebuffer(app->device, &framebufferInfo, NULL, &app->swapchainFramebuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create framebuffer!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void createCommandPool(VulkanApp* app) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(app->physicalDevice, app->surface);
    
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    
    if (vkCreateCommandPool(app->device, &poolInfo, NULL, &app->commandPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create command pool!\n");
        exit(EXIT_FAILURE);
    }
}

void createCommandBuffers(VulkanApp* app) {
    app->commandBuffers = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));
    
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = app->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    
    if (vkAllocateCommandBuffers(app->device, &allocInfo, app->commandBuffers) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate command buffers!\n");
        exit(EXIT_FAILURE);
    }
}

void createSyncObjects(VulkanApp* app) {
    app->imageAvailableSemaphores = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    app->renderFinishedSemaphores = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    app->inFlightFences = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
    
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(app->device, &semaphoreInfo, NULL, &app->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(app->device, &fenceInfo, NULL, &app->inFlightFences[i]) != VK_SUCCESS) {
            
            fprintf(stderr, "Failed to create synchronization objects for a frame!\n");
            exit(EXIT_FAILURE);
        }
    }
}

void drawFrame(VulkanApp* app) {
    // Wait for the previous frame to finish
    vkWaitForFences(app->device, 1, &app->inFlightFences[app->currentFrame], VK_TRUE, UINT64_MAX);
    
    // Acquire an image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(app->device, app->swapchain, UINT64_MAX, 
                                           app->imageAvailableSemaphores[app->currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(app);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "Failed to acquire swap chain image!\n");
        exit(EXIT_FAILURE);
    }
    
    // Only reset the fence if we are submitting work
    vkResetFences(app->device, 1, &app->inFlightFences[app->currentFrame]);
    
    // Record command buffer
    vkResetCommandBuffer(app->commandBuffers[app->currentFrame], 0);
    
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;
    
    if (vkBeginCommandBuffer(app->commandBuffers[app->currentFrame], &beginInfo) != VK_SUCCESS) {
        fprintf(stderr, "Failed to begin recording command buffer!\n");
        exit(EXIT_FAILURE);
    }
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = app->renderPass;
    renderPassInfo.framebuffer = app->swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = app->swapchainExtent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(app->commandBuffers[app->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind graphics pipeline
    vkCmdBindPipeline(app->commandBuffers[app->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, app->graphicsPipeline);
    
    // Draw triangle (3 vertices, 1 instance)
    vkCmdDraw(app->commandBuffers[app->currentFrame], 3, 1, 0, 0);
    
    // End render pass
    vkCmdEndRenderPass(app->commandBuffers[app->currentFrame]);
    
    if (vkEndCommandBuffer(app->commandBuffers[app->currentFrame]) != VK_SUCCESS) {
        fprintf(stderr, "Failed to record command buffer!\n");
        exit(EXIT_FAILURE);
    }
    
    // Submit command buffer
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {app->imageAvailableSemaphores[app->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &app->commandBuffers[app->currentFrame];
    
    VkSemaphore signalSemaphores[] = {app->renderFinishedSemaphores[app->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(app->graphicsQueue, 1, &submitInfo, app->inFlightFences[app->currentFrame]) != VK_SUCCESS) {
        fprintf(stderr, "Failed to submit draw command buffer!\n");
        exit(EXIT_FAILURE);
    }
    
    // Present
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[] = {app->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;
    
    result = vkQueuePresentKHR(app->presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || app->framebufferResized) {
        app->framebufferResized = false;
        recreateSwapchain(app);
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to present swap chain image!\n");
        exit(EXIT_FAILURE);
    }
    
    app->currentFrame = (app->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void recreateSwapchain(VulkanApp* app) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(app->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(app->window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(app->device);
    
    cleanupSwapchain(app);
    
    createSwapchain(app);
    createImageViews(app);
    createFramebuffers(app);
}

void cleanupSwapchain(VulkanApp* app) {
    for (uint32_t i = 0; i < app->swapchainImageCount; i++) {
        vkDestroyFramebuffer(app->device, app->swapchainFramebuffers[i], NULL);
    }
    free(app->swapchainFramebuffers);
    
    for (uint32_t i = 0; i < app->swapchainImageCount; i++) {
        vkDestroyImageView(app->device, app->swapchainImageViews[i], NULL);
    }
    free(app->swapchainImageViews);
    
    free(app->swapchainImages);
    
    vkDestroySwapchainKHR(app->device, app->swapchain, NULL);
}

// Helper function implementations
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    
    VkLayerProperties* availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    
    for (size_t i = 0; i < sizeof(validationLayers) / sizeof(validationLayers[0]); i++) {
        bool layerFound = false;
        
        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            free(availableLayers);
            return false;
        }
    }
    
    free(availableLayers);
    return true;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {0};
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    
    VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        
        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }
        
        if (indices.hasGraphicsFamily && indices.hasPresentFamily) {
            break;
        }
    }
    
    free(queueFamilies);
    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapchainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device, surface);
        swapchainAdequate = swapchainSupport.formatCount > 0 && swapchainSupport.presentModeCount > 0;
        free(swapchainSupport.formats);
        free(swapchainSupport.presentModes);
    }
    
    return indices.hasGraphicsFamily && indices.hasPresentFamily && extensionsSupported && swapchainAdequate;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    
    VkExtensionProperties* availableExtensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);
    
    for (size_t i = 0; i < sizeof(deviceExtensions) / sizeof(deviceExtensions[0]); i++) {
        bool found = false;
        for (uint32_t j = 0; j < extensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            free(availableExtensions);
            return false;
        }
    }
    
    free(availableExtensions);
    return true;
}

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupportDetails details = {0};
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);
    if (details.formatCount != 0) {
        details.formats = malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.presentModes = malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
    }
    
    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* formats, uint32_t formatCount) {
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    
    return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* presentModes, uint32_t presentModeCount) {
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentModes[i];
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        VkExtent2D actualExtent = {
            (uint32_t)width,
            (uint32_t)height
        };
        
        actualExtent.width = actualExtent.width < capabilities->minImageExtent.width ? capabilities->minImageExtent.width : actualExtent.width;
        actualExtent.width = actualExtent.width > capabilities->maxImageExtent.width ? capabilities->maxImageExtent.width : actualExtent.width;
        
        actualExtent.height = actualExtent.height < capabilities->minImageExtent.height ? capabilities->minImageExtent.height : actualExtent.height;
        actualExtent.height = actualExtent.height > capabilities->maxImageExtent.height ? capabilities->maxImageExtent.height : actualExtent.height;
        
        return actualExtent;
    }
}

VkShaderModule createShaderModule(VkDevice device, const char* filename) {
    size_t codeSize;
    char* code = readFile(filename, &codeSize);
    
    if (!code) {
        fprintf(stderr, "Failed to read shader file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (const uint32_t*)code;
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create shader module!\n");
        free(code);
        exit(EXIT_FAILURE);
    }
    
    free(code);
    return shaderModule;
}

char* readFile(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);
    
    char* buffer = malloc(*size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t result = fread(buffer, 1, *size, file);
    if (result != *size) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return buffer;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    (void)width;   // Suppress unused parameter warning
    (void)height;  // Suppress unused parameter warning
    VulkanApp* app = glfwGetWindowUserPointer(window);
    app->framebufferResized = true;
}