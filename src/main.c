#include <stdint.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#include "mathc.h"
#include "renderer_structs.h"

#define MAX_FRAMES_IN_FLIGHT 2

unsigned int getAttributeDescriptionsSize = 2;

Vertex vertices_old[3] = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

Vertex vertices[4] = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

uint16_t indices[6] = {0, 1, 2, 2, 3, 0};

typedef struct UniformBufferObject {
    struct mat4 model;
    struct mat4 view;
    struct mat4 proj;
} UniformBufferObject;

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const unsigned int deviceExtensionsCount = 1;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
int validation_layers_count = 1;
#endif

typedef struct State {
    uint32_t *fps_buffer;
    uint32_t fps_buffer_index;
    uint32_t fps_buffer_max;
    GLFWwindow *window;
    char *window_title;
    unsigned int window_width, window_hieght;
    bool window_resizable;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    unsigned int swapChainImagesCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandPool commandPool_transfer;
    VkCommandBuffer *commandBuffers;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    uint32_t currentFrame;
    bool framebufferResized;
    VkBuffer vertexIndexUniformBuffer;
    VkDeviceMemory vertexIndexUniformBufferMemory;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;
    void **uniformBuffersMapped;
    uint64_t frameTime;
    struct timeval time;  
  struct timeval program_start_time;
  VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
} State;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    unsigned int formatsCount;
    unsigned int presentModesCount;
} SwapChainSupportDetails;

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    bool graphicsFamily_exist;
    uint32_t presentFamily;
    bool presentFamily_exist;
    uint32_t transferFamily;
    bool transferFamily_exist;
} QueueFamilyIndices;

typedef struct File_S {
    char *data;
    unsigned int size;
} File_S;

static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

VkVertexInputAttributeDescription *getAttributeDescriptions() {

    getAttributeDescriptionsSize = 2;

    VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * getAttributeDescriptionsSize);
    memset(attributeDescriptions, 0, sizeof(VkVertexInputAttributeDescription) * getAttributeDescriptionsSize);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

QueueFamilyIndices findQueueFamilies(State *state, VkPhysicalDevice device);

State init_state(char *title, bool resizable, unsigned int width, unsigned int hieght) {
    State state = {0};
    state.window_hieght = hieght;
    state.window_width = width;
    state.window_title = title;
    state.window_resizable = resizable;
    state.fps_buffer_max = 4096;
    state.fps_buffer = malloc(sizeof(uint32_t) * state.fps_buffer_max);
      
    gettimeofday(&state.program_start_time,  NULL);
    return state;
}

File_S readFile(char *file_name) {
    File_S file = {0};
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: file doesnt exist\n");
    }
    fseek(fp, 0L, SEEK_END);
    unsigned int size = ftell(fp);
    fprintf(stderr, "%s file size = %u\n", file_name, size);
    fseek(fp, 0L, SEEK_SET);
    char *result = malloc(size + 1);
    memset(result, 0, size + 1);
    fread(result, size, 1, fp);
    result[size] = '\0';
    file.data = result;
    file.size = size;
    return file;
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, const unsigned int size) {
    for (int i = 0; i < size; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, const unsigned int size) {
    for (int i = 0; i < size; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

VkExtent2D chooseSwapExtent(State *state, const VkSurfaceCapabilitiesKHR *capabilities) {
    if (capabilities->currentExtent.width != 0xFFFFFFFF) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(state->window, &width, &height);

        VkExtent2D actualExtent = {(width), (height)};

        actualExtent.width = (actualExtent.width < capabilities->minImageExtent.width) * capabilities->minImageExtent.width + (actualExtent.width > capabilities->maxImageExtent.width) * capabilities->maxImageExtent.width + (!(actualExtent.width < capabilities->minImageExtent.width) && !(actualExtent.width > capabilities->maxImageExtent.width)) * actualExtent.width;

        actualExtent.height = (actualExtent.height < capabilities->minImageExtent.height) * capabilities->minImageExtent.height + (actualExtent.height > capabilities->maxImageExtent.height) * capabilities->maxImageExtent.height + (!(actualExtent.height < capabilities->minImageExtent.height) && !(actualExtent.height > capabilities->maxImageExtent.height)) * actualExtent.height;

        // actualExtent.width = std::clamp(actualExtent.width,
        // capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        // actualExtent.height = std::clamp(actualExtent.height,
        // capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails querySwapChainSupport(State *state, VkPhysicalDevice device) {
    SwapChainSupportDetails details = {0};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, state->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, state->surface, &formatCount, NULL);

    printf("formatCount = %u\n", formatCount);

    details.formatsCount = formatCount;

    if (formatCount != 0) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        memset(details.formats, 0, sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, state->surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, state->surface, &presentModeCount, NULL);

    printf("presentModeCount = %u\n", presentModeCount);

    details.presentModesCount = presentModeCount;

    if (presentModeCount != 0) {
        details.presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        memset(details.presentModes, 0, sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, state->surface, &presentModeCount, details.presentModes);
    }

    return details;
}

void createSwapChain(State *state) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(state, state->physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesCount);
    VkExtent2D extent = chooseSwapExtent(state, &swapChainSupport.capabilities);

    state->swapChainImageFormat = surfaceFormat.format;
    state->swapChainExtent = extent;

    uint32_t imageCount = 128;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = state->surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(state, state->physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily, indices.transferFamily};

    if (indices.graphicsFamily != indices.presentFamily || indices.graphicsFamily != indices.transferFamily || indices.presentFamily != indices.transferFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 3;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;  // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(state->device, &createInfo, NULL, &state->swapChain) != VK_SUCCESS) {
        fprintf(stderr, "failed to create swap chain!");
    } else {
        printf("swapChane was seccesfully created\n");
    }

    vkGetSwapchainImagesKHR(state->device, state->swapChain, &imageCount, NULL);

    state->swapChainImages = malloc(sizeof(VkImage) * imageCount);
    memset(state->swapChainImages, 0, sizeof(VkImage) * imageCount);

    state->swapChainImagesCount = imageCount;

    vkGetSwapchainImagesKHR(state->device, state->swapChain, &imageCount, state->swapChainImages);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    memset(availableLayers, 0, sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (int i = 0; i < validation_layers_count; i++) {
        bool layerFound = false;

        for (int j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    // cleanup
    free(availableLayers);

    return true;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {

    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = NULL; // Optional
}

const char **getRequiredExtensions(int *size) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const char **extensions = malloc(sizeof(char *) * glfwExtensionCount + 1);
    memset(extensions, 0, sizeof(char *) * glfwExtensionCount + 1);
    for (int i = 0; i < glfwExtensionCount; i++) {
        // memset(extensions[i], 0, 2048);
        extensions[i] = glfwExtensions[i];
    }
    *size = glfwExtensionCount;
    if (enableValidationLayers) {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        *size = glfwExtensionCount + 1;

        printf("correct extensions_count = %d\n", glfwExtensionCount + 1);
    } else {
        printf("correct extensions_count = %d\n", glfwExtensionCount);
    }

    return extensions;
}

void createInstance(State *state) {
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = NULL;

    int extensions_count = 0;
    const char **extensions = getRequiredExtensions(&extensions_count);
    createInfo.enabledExtensionCount = extensions_count;
    createInfo.ppEnabledExtensionNames = extensions;

    printf("returned extensions_count = %d\n", extensions_count);

    // VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};

    createInfo.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validation_layers_count;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &state->instance) != VK_SUCCESS) {
        fprintf(stderr, "failed to create instance!");
        exit(-1);
    }

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        fprintf(stderr, "validation layers requested, but not available!\n");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties *vulkan_extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);

    memset(vulkan_extensions, 0, sizeof(VkExtensionProperties) * extensionCount);

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, vulkan_extensions);
    printf("available extensions: %d\n", extensionCount);

    for (int i = 0; i < extensionCount; i++) {
        printf("\t%s\n", vulkan_extensions[i].extensionName);
    }

    // cleanup
    free(vulkan_extensions);
}

QueueFamilyIndices findQueueFamilies(State *state, VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // Logic to find queue family indices to populate struct with
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    printf("queueFamilyCount = %u\n", queueFamilyCount);

    VkQueueFamilyProperties *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    memset(queueFamilies, 0, sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamily_exist = true;
            printf("graphicsFamily_exist = true\n");
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
            indices.transferFamily_exist = true;
            printf("transferFamily_exist = true\n");
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, state->surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            indices.presentFamily_exist = true;

            printf("presentFamily_exist = true\n");
        }
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties *availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    memset(availableExtensions, 0, sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    char **requiredExtensions = malloc(sizeof(intptr_t) * deviceExtensionsCount);
    memset(requiredExtensions, 0, sizeof(intptr_t) * deviceExtensionsCount);

    for (int i = 0; i < deviceExtensionsCount; i++) {
        requiredExtensions[i] = malloc(512);
        memset(requiredExtensions[i], 0, 512);
        strcpy(requiredExtensions[i], deviceExtensions[i]);
    }

    printf("!!!!!!!!!!!!!!!!!!!!\n");
    for (int i = 0; i < extensionCount; i++) {
        for (int j = 0; j < deviceExtensionsCount; j++) {

            if (!strcmp(deviceExtensions[j], availableExtensions[i].extensionName)) {
                free(requiredExtensions[j]);
                requiredExtensions[j] = NULL;
            }
        }
    }

    printf("!!!!!!!!!!!!!!!!!!!!\n");
    bool isSupported = true;

    for (int j = 0; j < deviceExtensionsCount; j++) {
        if (requiredExtensions[j] != NULL) {
            isSupported = false;
        }
    }

    printf("extentions are %s\n", isSupported ? "supported" : "not supported");

    // cleanup
    free(availableExtensions);

    return isSupported;
}

bool isDeviceSuitable(State *state, VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // printf("!!!!!!!!!!!!!!!!!!!!\n");

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    // printf("!!!!!!!!!!!!!!!!!!!!\n");

    QueueFamilyIndices indices = findQueueFamilies(state, device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(state, device);
        swapChainAdequate = swapChainSupport.formats && swapChainSupport.presentModes;
    }

    printf("swapChain is %s\n", swapChainAdequate ? "Supported" : "not supported");

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && deviceFeatures.geometryShader && indices.graphicsFamily_exist && indices.presentFamily_exist && swapChainAdequate;
}

void pickPhysicalDevice(State *state) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(state->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        fprintf(stderr, "failed to find GPUs with Vulkan support!");
        exit(-1);
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    memset(devices, 0, sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(state->instance, &deviceCount, devices);

    for (int i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(state, devices[i])) {
            state->physicalDevice = devices[i];
            break;
        }
    }

    if (state->physicalDevice == VK_NULL_HANDLE) {
        fprintf(stderr, "failed to find a suitable GPU!");
    }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setupDebugMessenger(State *state) {
    if (!enableValidationLayers)
        return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};

    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(state->instance, &createInfo, NULL, &state->debugMessenger) != VK_SUCCESS) {
        fprintf(stderr, "failed to set up debug messenger!\n");
    }
    printf("DebugMessenger was successfully created\n");
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

void createLogicalDevice(State *state) {
    QueueFamilyIndices indices = findQueueFamilies(state, state->physicalDevice);

    VkPhysicalDeviceFeatures deviceFeatures = {0};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    VkDeviceQueueCreateInfo queueCreateInfos[3];
    uint32_t uniqueQueueFamilies[3] = {indices.graphicsFamily, indices.presentFamily, indices.transferFamily};

    float queuePriority = 1.0f;
    for (int i = 0; i < 3; i++) {
        // printf("!!!!!!!!!!!!!!\n");
        VkDeviceQueueCreateInfo queueCreateInfo_t = {0};
        queueCreateInfo_t.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo_t.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo_t.queueCount = 1;
        queueCreateInfo_t.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo_t;
    }

    createInfo.queueCreateInfoCount = 3;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = deviceExtensionsCount;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validation_layers_count;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    printf("logical device phase one created seccessfully\n");

    if (vkCreateDevice(state->physicalDevice, &createInfo, NULL, &state->device) != VK_SUCCESS) {
        fprintf(stderr, "failed to create logical device!\n");
    } else {
        printf("logical device phase one created seccessfully\n");
    }

    vkGetDeviceQueue(state->device, indices.graphicsFamily, 0, &state->graphicsQueue);
    vkGetDeviceQueue(state->device, indices.presentFamily, 0, &state->presentQueue);

    vkGetDeviceQueue(state->device, indices.transferFamily, 0, &state->transferQueue);

    printf("logical device phase one created seccessfully\n");
}

void createSurface(State *state) {
    if (glfwCreateWindowSurface(state->instance, state->window, NULL, &state->surface) != VK_SUCCESS) {
        fprintf(stderr, "failed to create window surface!\n");
    }
}

void createImageViews(State *state) {
    state->swapChainImageViews = malloc(sizeof(VkImageView) * state->swapChainImagesCount);

    memset(state->swapChainImageViews, 0, sizeof(VkImageView) * state->swapChainImagesCount);

    for (unsigned int i = 0; i < state->swapChainImagesCount; i++) {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = state->swapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = state->swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(state->device, &createInfo, NULL, &state->swapChainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "failed to create imageviews[%u]!\n", i);
        } else {
            printf("imageView[%u] was seccesfully created\n", i);
        }
    }
}

VkShaderModule createShaderModule(State *state, File_S code) {
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size;
    createInfo.pCode = (uint32_t *)(code.data);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(state->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "failed to create shader module!\n");
    } else {
        printf("shader module has been created secessfully\n");
    }

    return shaderModule;
}

void createGraphicsPipeline(State *state) {

    File_S vertShaderCode = readFile("./triangle_vert_opt.spv");
    File_S fragShaderCode = readFile("./triangle_frag_opt.spv");

    VkShaderModule vertShaderModule = createShaderModule(state, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(state, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    int dynamicStatesSize = 2;

    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStatesSize;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkVertexInputBindingDescription bindingDescription = getBindingDescription();
    VkVertexInputAttributeDescription *attributeDescriptions = getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = getAttributeDescriptionsSize;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)state->swapChainExtent.width;
    viewport.height = (float)state->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = state->swapChainExtent;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = NULL;               // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &state->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    if (vkCreatePipelineLayout(state->device, &pipelineLayoutInfo, NULL, &state->pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "failed to create pipeline layout!\n");
    } else {
        printf("pipeline layout secessfully created\n");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = state->pipelineLayout;

    pipelineInfo.renderPass = state->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    if (vkCreateGraphicsPipelines(state->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &state->graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "failed to create graphics pipeline!\n");
    } else {
        printf("pipeline was secessfully created\n");
    }

    vkDestroyShaderModule(state->device, fragShaderModule, NULL);
    vkDestroyShaderModule(state->device, vertShaderModule, NULL);
}

void createRenderPass(State *state) {

    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = state->swapChainImageFormat;
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

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(state->device, &renderPassInfo, NULL, &state->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "failed to create render pass!\n");
    } else {
        printf("render pass was secesfully created\n");
    }
}

void createFramebuffers(State *state) {
    state->swapChainFramebuffers = malloc(sizeof(VkFramebuffer) * state->swapChainImagesCount);
    memset(state->swapChainFramebuffers, 0, sizeof(VkFramebuffer) * state->swapChainImagesCount);

    for (int i = 0; i < state->swapChainImagesCount; i++) {
        VkImageView attachments[] = {state->swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = state->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = state->swapChainExtent.width;
        framebufferInfo.height = state->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(state->device, &framebufferInfo, NULL, &state->swapChainFramebuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "failed to create framebuffer[%d]!\n", i);
        } else {
            fprintf(stderr, "framebuffer[%d] was secessfully created\n", i);
        }
    }
}

void recordCommandBuffer(State *state, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;               // Optional
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        fprintf(stderr, "failed to begin recording command buffer!\n");
    }

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = state->renderPass;
    renderPassInfo.framebuffer = state->swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = state->swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(state->swapChainExtent.width);
    viewport.height = (float)(state->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = state->swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {state->vertexIndexUniformBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, state->vertexIndexUniformBuffer, sizeof(vertices), VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipelineLayout, 0, 1, &state->descriptorSets[state->currentFrame], 0, NULL);

    vkCmdDrawIndexed(commandBuffer, sizeof(indices) / sizeof(uint16_t), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        fprintf(stderr, "failed to record command buffer!\n");
    }
}

void createCommandPool(State *state) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(state, state->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(state->device, &poolInfo, NULL, &state->commandPool) != VK_SUCCESS) {
        fprintf(stderr, "failed to create command pool!\n");
    } else {
        printf("command pool was secessfully created\n");
    }

    VkCommandPoolCreateInfo poolInfo_transfer = {0};
    poolInfo_transfer.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo_transfer.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo_transfer.queueFamilyIndex = queueFamilyIndices.transferFamily;

    if (vkCreateCommandPool(state->device, &poolInfo_transfer, NULL, &state->commandPool_transfer) != VK_SUCCESS) {
        fprintf(stderr, "failed to create commandPool_transfer!\n");
    } else {
        printf("commandPool_transfer was secessfully created\n");
    }
}

void createCommandBuffers(State *state) {
    state->commandBuffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    memset(state->commandBuffers, 0, sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = state->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(state->device, &allocInfo, state->commandBuffers) != VK_SUCCESS) {
        fprintf(stderr, "failed to allocate command buffers!\n");
    } else {
        printf("command buffers were allocate secessfully!\n");
    }
}

void createSyncObjects(State *state) {
    state->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    memset(state->imageAvailableSemaphores, 0, sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);

    state->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    memset(state->renderFinishedSemaphores, 0, sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);

    state->inFlightFences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    memset(state->inFlightFences, 0, sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        if (vkCreateSemaphore(state->device, &semaphoreInfo, NULL, &state->imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(state->device, &semaphoreInfo, NULL, &state->renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(state->device, &fenceInfo, NULL, &state->inFlightFences[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: failed to create semaphores and fence!!!!!!!!\n");
        } else {
            printf("secessefully created semaphores and fences\n");
        }
    }
}

void cleanupSwapChain(State *state) {
    for (int i = 0; i < state->swapChainImagesCount; i++) {
        vkDestroyFramebuffer(state->device, state->swapChainFramebuffers[i], NULL);
    }
    vkDestroyRenderPass(state->device, state->renderPass, NULL);

    for (unsigned int i = 0; i < state->swapChainImagesCount; i++) {
        vkDestroyImageView(state->device, state->swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(state->device, state->swapChain, NULL);
}

void recreateSwapChain(State *state) {

    int width = 0, height = 0;
    glfwGetFramebufferSize(state->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(state->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(state->device);

    cleanupSwapChain(state);

    createSwapChain(state);
    createImageViews(state);
    createRenderPass(state);
    createFramebuffers(state);
}

void printf_UBO(UniformBufferObject ubo){
    printf("model = [%f %f %f %f\n", ubo.model.m11, ubo.model.m12, ubo.model.m13, ubo.model.m14);
    printf("         %f %f %f %f\n", ubo.model.m21, ubo.model.m22, ubo.model.m23, ubo.model.m24);
    printf("         %f %f %f %f\n", ubo.model.m31, ubo.model.m32, ubo.model.m33, ubo.model.m34);
    printf("         %f %f %f %f]\n\n", ubo.model.m41, ubo.model.m42, ubo.model.m43, ubo.model.m44);
    
    printf("view = [%f %f %f %f\n", ubo.view.m11, ubo.view.m12, ubo.view.m13, ubo.view.m14);
    printf("        %f %f %f %f\n", ubo.view.m21, ubo.view.m22, ubo.view.m23, ubo.view.m24);
    printf("        %f %f %f %f\n", ubo.view.m31, ubo.view.m32, ubo.view.m33, ubo.view.m34);
    printf("        %f %f %f %f]\n\n", ubo.view.m41, ubo.view.m42, ubo.view.m43, ubo.view.m44);
    
    printf("proj = [%f %f %f %f\n", ubo.proj.m11, ubo.proj.m12, ubo.proj.m13, ubo.proj.m14);
    printf("        %f %f %f %f\n", ubo.proj.m21, ubo.proj.m22, ubo.proj.m23, ubo.proj.m24);
    printf("        %f %f %f %f\n", ubo.proj.m31, ubo.proj.m32, ubo.proj.m33, ubo.proj.m34);
    printf("        %f %f %f %f]\n\n", ubo.proj.m41, ubo.proj.m42, ubo.proj.m43, ubo.proj.m44);
    
  

}
  UniformBufferObject ubo = {0};
   
void updateUniformBuffer(State *state, uint32_t currentImage) {
   mat4_rotation_axis((mfloat_t *)&ubo.model, (mfloat_t[]){0.0f, 0.0f, 1.0f}, (((state->time.tv_sec * 1000000 + state->time.tv_usec ))/1000000.0f * to_radians(90)));
  
   
     mat4_look_at((mfloat_t *)(&ubo.view), (mfloat_t[]){0.0f, 1.0f, 1.5f}, (mfloat_t[]){0.0f, 0.0f, 0.0f}, (mfloat_t[]){0.0f, 1.0f, 0.0f});


    mat4_perspective((mfloat_t *)(&ubo.proj), to_radians(45.0f), (float)state->swapChainExtent.width / (float)state->swapChainExtent.height, 0.1f, 10.0f);
    
   
    memcpy(state->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    
  printf_UBO(ubo);
}

void drawFrame(State *state) {
    vkWaitForFences(state->device, 1, &state->inFlightFences[state->currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 1;
    VkResult result = vkAcquireNextImageKHR(state->device, state->swapChain, 100000000, state->imageAvailableSemaphores[state->currentFrame], VK_NULL_HANDLE, &imageIndex);

    updateUniformBuffer(state, state->currentFrame);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || state->framebufferResized) {
        state->framebufferResized = false;
        recreateSwapChain(state);
    } else if (result != VK_SUCCESS) {
        fprintf(stderr, "failed to present swap chain image!\n");
    }

    vkResetFences(state->device, 1, &state->inFlightFences[state->currentFrame]);

    vkResetCommandBuffer(state->commandBuffers[state->currentFrame], 0);
    recordCommandBuffer(state, state->commandBuffers[state->currentFrame], imageIndex);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {state->imageAvailableSemaphores[state->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &state->commandBuffers[state->currentFrame];

    VkSemaphore signalSemaphores[] = {state->renderFinishedSemaphores[state->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(state->graphicsQueue, 1, &submitInfo, state->inFlightFences[state->currentFrame]) != VK_SUCCESS) {
        fprintf(stderr, "failed to submit draw command buffer!\n");
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {state->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = NULL; // Optional

    vkQueuePresentKHR(state->presentQueue, &presentInfo);

    state->currentFrame = (state->currentFrame + 1) * ((state->currentFrame + 1) < MAX_FRAMES_IN_FLIGHT);
    // printf("state->currentFrame = %u\n", state->currentFrame);
}

uint32_t findMemoryType(State *state, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(state->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    fprintf(stderr, "failed to find suitable memory type!\n");
}

void copyBuffer(State *state, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = state->commandPool_transfer;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(state->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {0};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(state->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(state->transferQueue);

    vkFreeCommandBuffers(state->device, state->commandPool_transfer, 1, &commandBuffer);
}

void createBuffer(State *state, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    QueueFamilyIndices indices = findQueueFamilies(state, state->physicalDevice);
    uint32_t queueFamilyIndices[3] = {indices.graphicsFamily, indices.presentFamily, indices.transferFamily};

    if (indices.graphicsFamily != indices.presentFamily || indices.graphicsFamily != indices.transferFamily || indices.presentFamily != indices.transferFamily) {
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 3;
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = NULL;
    }

    if (vkCreateBuffer(state->device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        fprintf(stderr, "failed to create vertex buffer!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(state->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(state, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(state->device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "failed to allocate vertex buffer memory!\n");
    }

    vkBindBufferMemory(state->device, *buffer, *bufferMemory, 0);
}

/*
void createVertexBuffer(State *state) {
  VkDeviceSize bufferSize = sizeof(vertices);

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               &stagingBuffer, &stagingBufferMemory);

  void *data;
  vkMapMemory(state->device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices, (size_t)bufferSize);
  vkUnmapMemory(state->device, stagingBufferMemory);

  createBuffer(state, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->vertexBuffer,
               &state->vertexBufferMemory);

  copyBuffer(state, stagingBuffer, state->vertexBuffer, bufferSize);

  vkDestroyBuffer(state->device, stagingBuffer, NULL);
  vkFreeMemory(state->device, stagingBufferMemory, NULL);
}
*/

void createUniformBuffers(State *state) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    state->uniformBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    state->uniformBuffersMemory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
    state->uniformBuffersMapped = malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(state, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &state->uniformBuffers[i], &state->uniformBuffersMemory[i]);

        vkMapMemory(state->device, state->uniformBuffersMemory[i], 0, bufferSize, 0, &state->uniformBuffersMapped[i]);
    }
}

void createVertexIndexUniformBuffer(State *state) {
    VkDeviceSize bufferSize = sizeof(vertices) + sizeof(indices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(state->device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, vertices, sizeof(vertices));
    void *indices_buffer = ((Vertex *)data) + (sizeof(vertices) / sizeof(Vertex));
    memcpy(indices_buffer, indices, sizeof(indices));
    void *uniform_buffer = ((uint16_t *)indices_buffer) + (sizeof(indices) / sizeof(uint16_t));
    // memcpy(uniform_buffer, UniformBufferObject, sizeof(UniformBufferObject));

    vkUnmapMemory(state->device, stagingBufferMemory);

    createBuffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->vertexIndexUniformBuffer, &state->vertexIndexUniformBufferMemory);

    copyBuffer(state, stagingBuffer, state->vertexIndexUniformBuffer, bufferSize);

    vkDestroyBuffer(state->device, stagingBuffer, NULL);
    vkFreeMemory(state->device, stagingBufferMemory, NULL);
}

void createDescriptorSetLayout(State *state) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    uboLayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(state->device, &layoutInfo, NULL, &state->descriptorSetLayout) != VK_SUCCESS) {
        fprintf(stderr, "failed to create descriptor set layout!\n");
    } else {
        printf("SUCCESSED to create descriptor set layout!\n");
    }
}

void createDescriptorPool(State *state) {
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = (MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = (MAX_FRAMES_IN_FLIGHT);

    poolInfo.flags = 0; // optional

    if (vkCreateDescriptorPool(state->device, &poolInfo, NULL, &state->descriptorPool) != VK_SUCCESS) {
        fprintf(stderr, "ERROR, failed to create descriptor pool!\n");
    } else {
        printf("SUCCEED  to create descriptor pool!\n");
    }
}

void createDescriptorSets(State *state) {
    VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);
    memset(layouts, 0, sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = state->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = state->descriptorPool;
    allocInfo.descriptorSetCount = (MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    state->descriptorSets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

    memset(state->descriptorSets, 0, sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(state->device, &allocInfo, state->descriptorSets) != VK_SUCCESS) {
        fprintf(stderr, "ERROR, failed to allocate descriptor sets!\n");
    } else {
        printf("SUCCED to allocate descriptor sets!\n");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = state->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = state->descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = NULL;       // Optional
        descriptorWrite.pTexelBufferView = NULL; // Optional

        vkUpdateDescriptorSets(state->device, 1, &descriptorWrite, 0, NULL);
    }
}

void init_vulkan(State *state) {
    createInstance(state);
    setupDebugMessenger(state);
    createSurface(state);
    pickPhysicalDevice(state);
    createLogicalDevice(state);
    createSwapChain(state);
    createImageViews(state);
    createRenderPass(state);
    createDescriptorSetLayout(state);
    createGraphicsPipeline(state);
    createFramebuffers(state);
    createCommandPool(state);
    // createVertexBuffer(state);
    createVertexIndexUniformBuffer(state);
    createUniformBuffers(state);
    createDescriptorPool(state);
    createDescriptorSets(state);
    createCommandBuffers(state);
    createSyncObjects(state);
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    // State *app = (State*)(glfwGetWindowUserPointer(window));
    // app->framebufferResized = true;
}

void create_window(State *state) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE * state->window_resizable + GLFW_FALSE * !state->window_resizable);
    state->window = glfwCreateWindow(state->window_width, state->window_hieght, state->window_title, NULL, NULL);
    glfwSetFramebufferSizeCallback(state->window, framebufferResizeCallback);
}
void loop(State *state) {
    while (!glfwWindowShouldClose(state->window)) {
        struct timeval stop, start;
        gettimeofday(&start, NULL);

        glfwPollEvents();
        drawFrame(state);

        // usleep(1000000 / 60);
        gettimeofday(&stop, NULL);
//        printf("took %lu us\n", ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec));

        state->frameTime = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

//        printf("current fps %lu fps\n", 1000000 / ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec));

        state->fps_buffer[state->fps_buffer_index++] = 1000000 / ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

        if (state->fps_buffer_index >= (state->fps_buffer_max - 1)) {
            state->fps_buffer_index = 0;
        }

        uint32_t fps = 0;
        for (uint32_t i = 0; i < state->fps_buffer_max; i++) {
            fps += state->fps_buffer[i];
        }
        fps /= state->fps_buffer_max;
//        printf("avg fps = %u\n", fps);
     gettimeofday(&state->time, NULL);
    state->time.tv_sec = state->time.tv_sec - state->program_start_time.tv_sec; 
    }


   vkDeviceWaitIdle(state->device);
}

void cleanup(State *state) {
    cleanupSwapChain(state);

    vkDestroyDescriptorPool(state->device, state->descriptorPool, NULL);

    vkDestroyDescriptorSetLayout(state->device, state->descriptorSetLayout, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(state->device, state->uniformBuffers[i], NULL);
        vkFreeMemory(state->device, state->uniformBuffersMemory[i], NULL);
    }

    vkDestroyDescriptorSetLayout(state->device, state->descriptorSetLayout, NULL);

    vkDestroyBuffer(state->device, state->vertexIndexUniformBuffer, NULL);
    vkFreeMemory(state->device, state->vertexIndexUniformBufferMemory, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(state->device, state->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(state->device, state->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(state->device, state->inFlightFences[i], NULL);
    }

    vkDestroyPipeline(state->device, state->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(state->device, state->pipelineLayout, NULL);

    vkDestroyCommandPool(state->device, state->commandPool, NULL);

    vkDestroyDevice(state->device, NULL);

    vkDestroySurfaceKHR(state->instance, state->surface, NULL);
    vkDestroyInstance(state->instance, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(state->instance, state->debugMessenger, NULL);
    }

    glfwDestroyWindow(state->window);
    glfwTerminate();
}

int main(int argnum, char **args) {
    State state = init_state("vulkan", true, 800, 600);
    printf("wellcome to vulkan\n");

    glfwInit();
    create_window(&state);
    init_vulkan(&state);
    loop(&state);
    return 0;
}
