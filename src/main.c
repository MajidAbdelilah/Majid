#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "image.h"
#define MATHC_USE_FLOATING_POINT
#include "mathc.h"
#include "model_loader.h"
#include "renderer_structs.h"
#include <GLFW/glfw3.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <vulkan/vulkan.h>
// #define UMATH_IMPLEMENTATION

#include "ufbx.h"
#include "ufbx/umath.h"

#include <time.h>


#define MAX_FRAMES_IN_FLIGHT 2

unsigned int getAttributeDescriptionsSize = 2;

// Vertex vertices_old[3] = {{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

// Vertex vertices[2][4] = {{{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f,
// 0.0f, 1.0f}, {0.0f, 1.0f}}, {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}},

//                         {{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, {{0.5f, 0.5f,
//                         -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}}};

// uint16_t indices[12] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

typedef struct UniformBufferObject {
	alignas(16) struct mat4 model;
	alignas(16) struct mat4 view;
	alignas(16) struct mat4 proj;
} UniformBufferObject;

typedef struct Particle {
	alignas(8) struct vec2 position;
	alignas(8) struct vec2 velocity;
	alignas(16) struct vec4 color;
} Particle;

#define PARTICLE_COUNT 1024

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
	VkQueue computeQueue;
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
	VkCommandPool commandPool_compute;
	VkCommandBuffer *commandBuffers;
	VkSemaphore *imageAvailableSemaphores;
	VkSemaphore *renderFinishedSemaphores;
	VkFence *inFlightFences;
	uint32_t currentFrame;
	bool framebufferResized;
	Vertex *vertices;
	uint32_t *indices;
	uint32_t vertex_count;
	uint32_t index_count;
	Majid_model *models;
	unsigned long models_size;
	unsigned long models_count;
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

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSampleCountFlagBits msaaSamples;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	
	VkBuffer *shaderStorageBuffers;
	VkDeviceMemory *shaderStorageBuffersMemory;
	VkDescriptorSetLayout	computeDescriptorSetLayout;
	VkDescriptorSet *computeDescriptorSets;
	VkPipelineLayout computePipelineLayout;
	VkPipeline computePipeline;
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
	uint32_t computeFamily;
	bool computeFamily_exist;
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

static VkVertexInputAttributeDescription *getAttributeDescriptions() {

	getAttributeDescriptionsSize = 3;

	VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * getAttributeDescriptionsSize);
	memset(attributeDescriptions, 0, sizeof(VkVertexInputAttributeDescription) * getAttributeDescriptionsSize);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

QueueFamilyIndices findQueueFamilies(State *state, VkPhysicalDevice device);

State init_state(char *title, bool resizable, unsigned int width, unsigned int hieght) {
	State state = {0};
	state.window_hieght = hieght;
	state.window_width = width;
	state.window_title = title;
	state.window_resizable = resizable;
	state.fps_buffer_max = 512;
	state.fps_buffer = malloc(sizeof(uint32_t) * state.fps_buffer_max);
	state.vertices = NULL;
	state.indices = NULL;
	gettimeofday(&state.program_start_time, NULL);
	state.models_size = 128;
	state.models = malloc(sizeof(Majid_model) * state.models_size);
	memset(state.models, 0, sizeof(Majid_model) * state.models_size);
	state.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
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


int GetRandomInt(int min, int max)
{
	  srand(time(0));
		return  ((rand() % (max - min + 1)) + min);
	
}

float GetRandomfloat(float min, float max){
	
float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
return scale * ( max - min ) + min;      /* [min, max] */
	
}

void createBuffer(State *state, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory);

void copyBuffer(State *state, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

VkShaderModule createShaderModule(State *state, File_S code);

void createShaderStorageBuffers(State *state){
	state->shaderStorageBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	memset(state->shaderStorageBuffers, 0, sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	
	state->shaderStorageBuffersMemory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
	memset(state->shaderStorageBuffersMemory, 0, sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
	
	
	// Initial particle positions on a circle
	Particle *particles = malloc(sizeof(Particle) * PARTICLE_COUNT);
	memset(particles, 0, sizeof(Particle) * PARTICLE_COUNT);
	
	for (unsigned int i=0; i<PARTICLE_COUNT; i++) {
		Particle particle = particles[i];
		float r = 0.25f * sqrt(GetRandomfloat(0.0f, 1.0f));
		
		float theta = GetRandomfloat(0.0f, 1.0f) * 2 * 3.14159265358979323846;
		float x = r * cos(theta) * state->window_hieght / state->window_width;
		float y = r * sin(theta);
		particle.position = (struct vec2){x, y};
		vec2_normalize((mfloat_t*)&particle.velocity, (mfloat_t[2]){x,y});
		particle.velocity.x *= 0.00025f;
		particle.velocity.y *= 0.00025f;
		
		particle.color = (struct vec4){GetRandomfloat(0.0f, 1.0f), GetRandomfloat(0.0f, 1.0f), GetRandomfloat(0.0f, 1.0f), 1.0f};
	}
	
	VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
	
	void* data;
	vkMapMemory(state->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, particles, (size_t)bufferSize);
	vkUnmapMemory(state->device, stagingBufferMemory);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		createBuffer(state, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->shaderStorageBuffers[i], &state->shaderStorageBuffersMemory[i]);
		// Copy data from the staging buffer (host) to the shader storage buffer (GPU)
		copyBuffer(state, stagingBuffer, state->shaderStorageBuffers[i], bufferSize);
	}

}

void createComputeDescriptorSetLayout(State *state) {
	VkDescriptorSetLayoutBinding layoutBindings[3] = {0};
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].pImmutableSamplers = NULL;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	
	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[1].pImmutableSamplers = NULL;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	
	layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[2].pImmutableSamplers = NULL;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	
	VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 3;
	layoutInfo.pBindings = layoutBindings;
	
	if (vkCreateDescriptorSetLayout(state->device, &layoutInfo, NULL, &state->computeDescriptorSetLayout) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create compute descriptor set layout!\n");
	}else{
		printf("SUCCESS: SUCCED to create compute descriptor set layout!\n");		
	}
}


void createComputeDescriptorSets(State *state) {
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {state->computeDescriptorSetLayout, state->computeDescriptorSetLayout};
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = state->descriptorPool;
	allocInfo.descriptorSetCount = (MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts;
	
	state->computeDescriptorSets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(state->device, &allocInfo, state->computeDescriptorSets) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to allocate descriptor sets!\n");
	}
	
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo uniformBufferInfo = {0};
		uniformBufferInfo.buffer = state->uniformBuffers[i];
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = sizeof(UniformBufferObject);
		
		VkWriteDescriptorSet descriptorWrites[3] = {0};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = state->computeDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &uniformBufferInfo;
		
		VkDescriptorBufferInfo storageBufferInfoLastFrame = {0};
		storageBufferInfoLastFrame.buffer = state->shaderStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
		storageBufferInfoLastFrame.offset = 0;
		storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;
		
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = state->computeDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;
		
		VkDescriptorBufferInfo storageBufferInfoCurrentFrame = {0};
		storageBufferInfoCurrentFrame.buffer = state->shaderStorageBuffers[i];
		storageBufferInfoCurrentFrame.offset = 0;
		storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;
		
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = state->computeDescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;
		
		vkUpdateDescriptorSets(state->device, 3, descriptorWrites, 0, NULL);
	}
}

void createComputePipeline(State *state) {
	File_S computeShaderCode = readFile("shaders/comp.spv");
	
	VkShaderModule computeShaderModule = createShaderModule(state, computeShaderCode);
	
	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {0};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = computeShaderModule;
	computeShaderStageInfo.pName = "main";
	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &state->computeDescriptorSetLayout;
	
	if (vkCreatePipelineLayout(state->device, &pipelineLayoutInfo, NULL, &state->computePipelineLayout) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create compute pipeline layout!\n");
	}else{
	printf("SUCCESS: created compute pipeline layout!\n");	
	}
	
	VkComputePipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = state->computePipelineLayout;
	pipelineInfo.stage = computeShaderStageInfo;
	
	if (vkCreateComputePipelines(state->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &state->computePipeline) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create compute pipeline!\n");
	}else{
	printf("SUCCED: created compute pipeline!\n");	
	}
	
	vkDestroyShaderModule(state->device, computeShaderModule, NULL);
}


VkSampleCountFlagBits getMaxUsableSampleCount(State *state) {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(state->physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		printf("device support VK_SAMPLE_COUNT_64_BIT\n");
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) {
		printf("device support VK_SAMPLE_COUNT_32_BIT\n");
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) {
		printf("device support VK_SAMPLE_COUNT_16_BIT\n");
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) {
		printf("device support VK_SAMPLE_COUNT_8_BIT\n");
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		printf("device support VK_SAMPLE_COUNT_4_BIT\n");
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		printf("device support VK_SAMPLE_COUNT_2_BIT\n");
		return VK_SAMPLE_COUNT_2_BIT;
	}
	
	printf("device support VK_SAMPLE_COUNT_1_BIT\n");
	
	return VK_SAMPLE_COUNT_1_BIT;
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

		actualExtent.width =
		    (actualExtent.width < capabilities->minImageExtent.width) * capabilities->minImageExtent.width +
		    (actualExtent.width > capabilities->maxImageExtent.width) * capabilities->maxImageExtent.width +
		    (!(actualExtent.width < capabilities->minImageExtent.width) && !(actualExtent.width > capabilities->maxImageExtent.width)) * actualExtent.width;

		actualExtent.height =
		    (actualExtent.height < capabilities->minImageExtent.height) * capabilities->minImageExtent.height +
		    (actualExtent.height > capabilities->maxImageExtent.height) * capabilities->maxImageExtent.height +
		    (!(actualExtent.height < capabilities->minImageExtent.height) && !(actualExtent.height > capabilities->maxImageExtent.height)) * actualExtent.height;

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

	if (indices.graphicsFamily != indices.presentFamily || indices.graphicsFamily != indices.transferFamily || indices.presentFamily != indices.transferFamily || indices.graphicsFamily!= indices.computeFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 4;
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

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
	createInfo->messageSeverity =
	    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo->messageType =
	    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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
	appInfo.pApplicationName = "Majid Game Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "Majid Game Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//uint32_t glfwExtensionCount = 0;
	//const char **glfwExtensions = NULL;

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
			printf("graphicsFamily_exist = true at index = %d\n", i);
		}
		
		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transferFamily = i;
			indices.transferFamily_exist = true;
			printf("transferFamily_exist = true at index = %d\n", i);
		}
		
		if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
			indices.computeFamily_exist = true;
			printf("computeFamily_exist = true at index = %d\n", i);
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, state->surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
			indices.presentFamily_exist = true;

			printf("presentFamily_exist = true at index = %d\n", i);
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

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
	       deviceFeatures.geometryShader && indices.graphicsFamily_exist && indices.presentFamily_exist && indices.transferFamily_exist && swapChainAdequate &&
	       supportedFeatures.samplerAnisotropy;
}

void pickPhysicalDevice(State *state) {
	//VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

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
			state->msaaSamples = VK_SAMPLE_COUNT_4_BIT;//getMaxUsableSampleCount(state);
			break;
		}
	}

	if (state->physicalDevice == VK_NULL_HANDLE) {
		fprintf(stderr, "failed to find a suitable GPU!");
	}
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {

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
	deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
	
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	VkDeviceQueueCreateInfo queueCreateInfos[4];
	uint32_t uniqueQueueFamilies[4] = {indices.graphicsFamily, indices.presentFamily, indices.transferFamily, indices.computeFamily};

	float queuePriority = 1.0f;
	for (int i = 0; i < 4; i++) {
		// printf("!!!!!!!!!!!!!!\n");
		VkDeviceQueueCreateInfo queueCreateInfo_t = {0};
		queueCreateInfo_t.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo_t.queueFamilyIndex = uniqueQueueFamilies[i];
		queueCreateInfo_t.queueCount = 1;
		queueCreateInfo_t.pQueuePriorities = &queuePriority;
		queueCreateInfos[i] = queueCreateInfo_t;
	}

	createInfo.queueCreateInfoCount = 4;
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
	vkGetDeviceQueue(state->device, indices.computeFamily, 0, &state->computeQueue);

	printf("logical device phase one created seccessfully\n");
}

void createSurface(State *state) {
	if (glfwCreateWindowSurface(state->instance, state->window, NULL, &state->surface) != VK_SUCCESS) {
		fprintf(stderr, "failed to create window surface!\n");
	}
}

VkImageView createImageView(State *state, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectFlags);

void createImageViews(State *state) {
	state->swapChainImageViews = malloc(sizeof(VkImageView) * state->swapChainImagesCount);

	memset(state->swapChainImageViews, 0, sizeof(VkImageView) * state->swapChainImagesCount);

	for (uint32_t i = 0; i < state->swapChainImagesCount; i++) {
		state->swapChainImageViews[i] = createImageView(state, state->swapChainImages[i], 1, state->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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


	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f;          // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother
	multisampling.rasterizationSamples = state->msaaSamples;
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


	VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;

	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = (VkStencilOpState) {
		0
	}; // Optional
	depthStencil.back = (VkStencilOpState) {
		0
	};  // Optional


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
	pipelineInfo.pDepthStencilState = &depthStencil;
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

VkFormat findDepthFormat(State *state);

void createRenderPass(State *state) {

	VkAttachmentDescription depthAttachment = {0};
	depthAttachment.format = findDepthFormat(state);
	depthAttachment.samples = state->msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {0};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = state->swapChainImageFormat;
	colorAttachment.samples = state->msaaSamples;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkAttachmentDescription colorAttachmentResolve = {0};
	colorAttachmentResolve.format = state->swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colorAttachmentResolveRef = {0};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
  subpass.pResolveAttachments = &colorAttachmentResolveRef;
	
	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[3] = {colorAttachment, depthAttachment, colorAttachmentResolve};
	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
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
		VkImageView attachments[3] = {state->colorImageView, state->depthImageView, state->swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo = {0};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = state->renderPass;
		framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
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

	VkClearValue clearValues[2] = {0};
	clearValues[0].color = (VkClearColorValue) {
		{
			0.0f, 0.0f, 0.0f, 1.0f
		}
	};
	clearValues[1].depthStencil = (VkClearDepthStencilValue) {
		1.0f, 0
	};

	VkRenderPassBeginInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = state->renderPass;
	renderPassInfo.framebuffer = state->swapChainFramebuffers[imageIndex];

	renderPassInfo.renderArea.offset = (VkOffset2D) {
		0, 0
	};
	renderPassInfo.renderArea.extent = state->swapChainExtent;

	// VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount = sizeof(clearValues) / sizeof(VkClearValue);
	renderPassInfo.pClearValues = clearValues;

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
	scissor.offset = (VkOffset2D) {
		0, 0
	};
	scissor.extent = state->swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	for (unsigned long i = 0; i < state->models_count; i++) {
		for (uint32_t j = 0; j < state->models[i].vertices_count_count; j++) {

			VkBuffer vertexBuffers[] = {state->models[i].vertexIndexUniformBuffer[j]};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, state->models[i].vertexIndexUniformBuffer[j], sizeof(Vertex) * state->models[i].vertices_count[j],
			                     VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipelineLayout, 0, 1, &state->descriptorSets[state->currentFrame], 0,
			                        NULL);

			vkCmdDrawIndexed(commandBuffer, state->models[i].indices_count[j], 1, 0, 0, 0);
		}
	}

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
	
	VkCommandPoolCreateInfo poolInfo_compute = {0};
	poolInfo_compute.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo_compute.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo_compute.queueFamilyIndex = queueFamilyIndices.computeFamily;
	
	if (vkCreateCommandPool(state->device, &poolInfo_compute, NULL, &state->commandPool_compute) != VK_SUCCESS) {
		fprintf(stderr, "failed to create commandPool_compute!\n");
	} else {
		printf("commandPool_compute was secessfully created\n");
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

		if (vkCreateSemaphore(state->device, &semaphoreInfo, NULL, &state->imageAvailableSemaphores[i]) != VK_SUCCESS ||
		    vkCreateSemaphore(state->device, &semaphoreInfo, NULL, &state->renderFinishedSemaphores[i]) != VK_SUCCESS ||
		    vkCreateFence(state->device, &fenceInfo, NULL, &state->inFlightFences[i]) != VK_SUCCESS) {
			fprintf(stderr, "ERROR: failed to create semaphores and fence!!!!!!!!\n");
		} else {
			printf("secessefully created semaphores and fences\n");
		}
	}
}

void cleanupSwapChain(State *state) {

	vkDestroyImageView(state->device, state->colorImageView, NULL);
	vkDestroyImage(state->device, state->colorImage, NULL);
	vkFreeMemory(state->device, state->colorImageMemory, NULL);
	
	vkDestroyImageView(state->device, state->depthImageView, NULL);
	vkDestroyImage(state->device, state->depthImage, NULL);
	vkFreeMemory(state->device, state->depthImageMemory, NULL);

	for (int i = 0; i < state->swapChainImagesCount; i++) {
		vkDestroyFramebuffer(state->device, state->swapChainFramebuffers[i], NULL);
	}
	vkDestroyRenderPass(state->device, state->renderPass, NULL);

	for (unsigned int i = 0; i < state->swapChainImagesCount; i++) {
		vkDestroyImageView(state->device, state->swapChainImageViews[i], NULL);
	}

	vkDestroySwapchainKHR(state->device, state->swapChain, NULL);
}

void createDepthResources(State *state);
void createColorResources(State *state);

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
	createColorResources(state);
	createDepthResources(state);
	createFramebuffers(state);
}

void printf_UBO(UniformBufferObject ubo) {
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

	ubo.model = state->models->model_matrix;
	mat4_rotation_axis((mfloat_t *)&ubo.model, (mfloat_t[]) {
		0.0f, 0.0f, 1.0f
	},
	(((state->time.tv_sec * 1000000 + state->time.tv_usec)) / 3000000.0f * to_radians(90)));
	// mat4_rotation_y((mfloat_t *)&ubo, to_radians(0));
	// mat4_rotation_x((mfloat_t *)&ubo, to_radians(0));

	// mat4_rotation_z((mfloat_t *)&ubo, to_radians(0));

	mat4_look_at((mfloat_t *)(&ubo.view), (mfloat_t[]) {
		1.0f, 1.0f, 1.0f
	}, (mfloat_t[]) {
		0.0f, 0.0f, 0.0f
	}, (mfloat_t[]) {
		0.0f, 0.0f, 1.0f
	});

	mat4_perspective((mfloat_t *)(&ubo.proj), to_radians(90.0f), (float)state->swapChainExtent.width / (float)state->swapChainExtent.height, 0.1f, 10.0f);

	ubo.proj.m22 *= -1;

	memcpy(state->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

	// printf_UBO(ubo);
}

void drawFrame(State *state) {
	vkWaitForFences(state->device, 1, &state->inFlightFences[state->currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 1;
	VkResult result =
	    vkAcquireNextImageKHR(state->device, state->swapChain, 100000000, state->imageAvailableSemaphores[state->currentFrame], VK_NULL_HANDLE, &imageIndex);

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

VkCommandBuffer beginSingleTimeCommands(State *state, VkCommandPool command_pool) {
	VkCommandBufferAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(state->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {0};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(State *state, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool command_pool) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(state->device, command_pool, 1, &commandBuffer);
}

void copyBuffer(State *state, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(state, state->commandPool_transfer);

	VkBufferCopy copyRegion = {0};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(state, commandBuffer, state->transferQueue, state->commandPool_transfer);
}

void copyBufferToImage(State *state, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(state, state->commandPool_transfer);

	VkBufferImageCopy region = {0};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = (VkOffset3D) {
		0, 0, 0
	};
	region.imageExtent = (VkExtent3D) {
		width, height, 1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(state, commandBuffer, state->transferQueue, state->commandPool_transfer);
}

bool hasStencilComponent(VkFormat format);

void generateMipmaps(State *state, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(state->physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		fprintf(stderr, "texture image format does not support linear blitting!\n");
	} else {
		printf("texture image format support linear blitting!\n");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(state, state->commandPool);

	VkImageMemoryBarrier barrier = {0};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		VkImageBlit blit = {0};
		blit.srcOffsets[0] = (VkOffset3D) {
			0, 0, 0
		};
		blit.srcOffsets[1] = (VkOffset3D) {
			mipWidth, mipHeight, 1
		};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = (VkOffset3D) {
			0, 0, 0
		};
		blit.dstOffsets[1] = (VkOffset3D) {
			mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1
		};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
	                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
	                     0, NULL,
	                     0, NULL,
	                     1, &barrier);

	endSingleTimeCommands(state, commandBuffer, state->graphicsQueue, state->commandPool);
}

void transitionImageLayout(State *state, VkImage image, uint32_t mipLevels, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(state, state->commandPool_transfer);

	VkImageMemoryBarrier barrier = {0};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = {0};
	VkPipelineStageFlags destinationStage = {0};

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		fprintf(stderr, "ERROR: unsupported layout transition!\n");
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

	endSingleTimeCommands(state, commandBuffer, state->transferQueue, state->commandPool_transfer);
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
		createBuffer(state, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		             &state->uniformBuffers[i], &state->uniformBuffersMemory[i]);

		vkMapMemory(state->device, state->uniformBuffersMemory[i], 0, bufferSize, 0, &state->uniformBuffersMapped[i]);
	}
}

void createVertexIndexUniformBuffer(State *state, unsigned long index) {

	state->models[index].vertexIndexUniformBuffer = malloc(sizeof(VkBuffer) * state->models[index].vertices_count_count);
	memset(state->models[index].vertexIndexUniformBuffer, 0, sizeof(VkBuffer) * state->models[index].vertices_count_count);

	state->models[index].vertexIndexUniformBufferMemory = malloc(sizeof(VkDeviceMemory) * state->models[index].vertices_count_count);
	memset(state->models[index].vertexIndexUniformBufferMemory, 0, sizeof(VkDeviceMemory) * state->models[index].vertices_count_count);

	for (unsigned long i = 0; i < state->models[index].vertices_count_count; i++) {
		VkDeviceSize bufferSize = sizeof(Vertex) * state->models[index].vertices_count[i] + sizeof(uint32_t) * state->models[index].indices_count[i];
		printf("state->models[index].vertices_count[i] = %u, state->models[index].indices_count[i] = %u", state->models[index].vertices_count[i],
		       state->models[index].indices_count[i]);
		printf("!!!!!!!!!!!!!!! createVertexIndexUniformBuffer loop buffer size = %lu !!!!!!!!!!!!!!!", bufferSize);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(state, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		             &stagingBuffer, &stagingBufferMemory);

		void *data;
		vkMapMemory(state->device, stagingBufferMemory, 0, bufferSize, 0, &data);

		memcpy(data, state->models[index].vertices[i], sizeof(Vertex) * state->models[index].vertices_count[i]);
		void *indices_buffer = ((Vertex *)data) + state->models[index].vertices_count[i];
		memcpy(indices_buffer, state->models[index].indices[i], sizeof(uint32_t) * state->models[index].indices_count[i]);
		//	void *uniform_buffer = ((uint16_t *)indices_buffer) + state->models[index].indices_count[i];
		// memcpy(uniform_buffer, UniformBufferObject, sizeof(UniformBufferObject));

		vkUnmapMemory(state->device, stagingBufferMemory);

		createBuffer(state, bufferSize,
		             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->models[index].vertexIndexUniformBuffer[i],
		             &state->models[index].vertexIndexUniformBufferMemory[i]);

		copyBuffer(state, stagingBuffer, state->models[index].vertexIndexUniformBuffer[i], bufferSize);

		vkDestroyBuffer(state->device, stagingBuffer, NULL);
		vkFreeMemory(state->device, stagingBufferMemory, NULL);
	}
}

void createVertexIndexUniformBuffer_for_all_models(State *state) {

	for (unsigned long i = 0; i < state->models_count; i++) {
		createVertexIndexUniformBuffer(state, i);
	}
}

void createDescriptorSetLayout(State *state) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;

	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	uboLayoutBinding.pImmutableSamplers = NULL; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = sizeof(bindings) / sizeof(VkDescriptorSetLayoutBinding);
	layoutInfo.pBindings = bindings;

	if (vkCreateDescriptorSetLayout(state->device, &layoutInfo, NULL, &state->descriptorSetLayout) != VK_SUCCESS) {
		fprintf(stderr, "failed to create descriptor set layout!\n");
	} else {
		printf("SUCCESSED to create descriptor set layout!\n");
	}
}

void createDescriptorPool(State *state) {
	VkDescriptorPoolSize poolSizes[2] = {0};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = (MAX_FRAMES_IN_FLIGHT);

	//poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//poolSizes[1].descriptorCount = (MAX_FRAMES_IN_FLIGHT);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = (MAX_FRAMES_IN_FLIGHT) * 2;

	VkDescriptorPoolCreateInfo poolInfo = {0};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
	poolInfo.pPoolSizes = poolSizes;

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

		VkDescriptorImageInfo imageInfo = {0};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = state->textureImageView;
		imageInfo.sampler = state->textureSampler;

		VkWriteDescriptorSet descriptorWrites[2] = {0};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = state->descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = state->descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(state->device, sizeof(descriptorWrites) / sizeof(VkWriteDescriptorSet), descriptorWrites, 0, NULL);
	}
}

void createImage(State *state, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory) {
	VkImageCreateInfo imageInfo = {0};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(state->device, &imageInfo, NULL, image) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create image!\n");
	} else {
		printf("SUCCED to create image!\n");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(state->device, *image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(state, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(state->device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to allocate image memory!\n");
	} else {
		printf("SUCCED to allocate image memory!\n");
	}

	vkBindImageMemory(state->device, *image, *imageMemory, 0);
}



void createColorResources(State *state) {
	VkFormat colorFormat = state->swapChainImageFormat;

	createImage(state, state->swapChainExtent.width, state->swapChainExtent.height, 1, state->msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->colorImage, &state->colorImageMemory);
	state->colorImageView = createImageView(state, state->colorImage, 1, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}


void createTextureImage(State *state) {
	M_image image = M_load_image("./viking_room/viking_room.png\0");

	state->mipLevels = floorf(log2f(um_max(image.texWidth, image.texHeight))) + 1;

	VkBuffer stagingBuffer = {0};
	VkDeviceMemory stagingBufferMemory = {0};
	createBuffer(state, image.imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	             &stagingBuffer, &stagingBufferMemory);

	void *data;
	vkMapMemory(state->device, stagingBufferMemory, 0, image.imageSize, 0, &data);
	memcpy(data, image.pixels, image.imageSize);
	vkUnmapMemory(state->device, stagingBufferMemory);

	stbi_image_free(image.pixels);

	createImage(state, image.texWidth, image.texHeight, state->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
	            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	            &state->textureImage, &state->textureImageMemory);

	transitionImageLayout(state, state->textureImage, state->mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(state, stagingBuffer, state->textureImage, image.texWidth, image.texHeight);

	generateMipmaps(state, state->textureImage, VK_FORMAT_R8G8B8A8_SRGB, image.texWidth, image.texHeight, state->mipLevels);

	//transitionImageLayout(state, state->textureImage, state->mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(state->device, stagingBuffer, NULL);
	vkFreeMemory(state->device, stagingBufferMemory, NULL);
}

VkImageView createImageView(State *state, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo = {0};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	if (vkCreateImageView(state->device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create texture image view!\n");
	} else {
		printf("SUCCED to create texture image view!\n");
	}

	return imageView;
}

void createTextureImageView(State *state) {
	state->textureImageView = createImageView(state, state->textureImage, state->mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler(State *state) {
	VkSamplerCreateInfo samplerInfo = {0};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkPhysicalDeviceProperties properties = {0};
	vkGetPhysicalDeviceProperties(state->physicalDevice, &properties);

	samplerInfo.anisotropyEnable = VK_TRUE;
	// samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; if you want the maximum qualitty
	samplerInfo.maxAnisotropy = 4.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = (float)(state->mipLevels);
	samplerInfo.mipLodBias = 0.0f; // Optional

	if (vkCreateSampler(state->device, &samplerInfo, NULL, &state->textureSampler) != VK_SUCCESS) {
		fprintf(stderr, "ERROR: failed to create texture sampler!\n");
	} else {
		printf("SUCCED to create texture sampler!\n");
	}
}

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findSupportedFormat(State *state, VkFormat *candidates, int candidates_count, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (int i = 0; i < candidates_count; i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(state->physicalDevice, candidates[i], &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidates[i];
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return candidates[i];
		}

		fprintf(stderr, "failed to find supported format!\n");
	}
}

VkFormat findDepthFormat(State *state) {
	return findSupportedFormat(state, (VkFormat[]) {
		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
	}, 3, VK_IMAGE_TILING_OPTIMAL,
	VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void createDepthResources(State *state) {

	VkFormat depthFormat = findDepthFormat(state);

	createImage(state, state->swapChainExtent.width, state->swapChainExtent.height, 1, state->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
	            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &state->depthImage, &state->depthImageMemory);

	state->depthImageView = createImageView(state, state->depthImage, 1, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	transitionImageLayout(state, state->depthImage, 1, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void printf_vertex(Vertex vertex) {
	printf("vertex = {pos ={%f, %f, %f}\n", vertex.pos.x, vertex.pos.y, vertex.pos.z);
	printf("        texcoords ={%f, %f}\n", vertex.texCoord.x, vertex.texCoord.y);
	printf("          color ={%f, %f, %f}}\n\n", vertex.color.x, vertex.color.y, vertex.color.z);
}

um_vec2 ufbx_to_um_vec2(ufbx_vec2 v) {
	return um_v2((float)v.x, (float)v.y);
}
um_vec3 ufbx_to_um_vec3(ufbx_vec3 v) {
	return um_v3((float)v.x, (float)v.y, (float)v.z);
}
um_quat ufbx_to_um_quat(ufbx_quat v) {
	return um_quat_xyzw((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}
um_mat ufbx_to_um_mat(ufbx_matrix m) {
	return um_mat_rows((float)m.m00, (float)m.m01, (float)m.m02, (float)m.m03, (float)m.m10, (float)m.m11, (float)m.m12, (float)m.m13, (float)m.m20, (float)m.m21,
	                   (float)m.m22, (float)m.m23, 0, 0, 0, 1, );
}

void pars_model(State *state, Majid_model model) {

	if (state->models_size <= (state->models_count + model.scene->nodes.count)) {
		state->models = realloc(state->models, sizeof(Majid_model) * (state->models_size + 128));
	}

	uint32_t mi = 0;
	for (unsigned long ni = 0; ni < model.scene->nodes.count; ni++) {
		ufbx_node *node = model.scene->nodes.data[ni];

		if (node->is_root)
			continue;

		if (node->mesh) {

			ufbx_mesh *mesh = node->mesh;

			state->models[state->models_count + mi].vertices = malloc(sizeof(Vertex *) * mesh->materials.count);
			memset(state->models[state->models_count + mi].vertices, 0, sizeof(Vertex *) * mesh->materials.count);
			state->models[state->models_count + mi].vertices_count_count = mesh->materials.count;

			state->models[state->models_count + mi].indices = malloc(sizeof(uint32_t *) * mesh->materials.count);
			memset(state->models[state->models_count + mi].indices, 0, sizeof(uint32_t *) * mesh->materials.count);
			state->models[state->models_count + mi].indices_count_count = mesh->materials.count;

			state->models[state->models_count + mi].vertices_count = malloc(sizeof(uint32_t) * mesh->materials.count);
			memset(state->models[state->models_count + mi].vertices_count, 0, sizeof(uint32_t) * mesh->materials.count);

			state->models[state->models_count + mi].indices_count = malloc(sizeof(uint32_t) * mesh->materials.count);
			memset(state->models[state->models_count + mi].indices_count, 0, sizeof(uint32_t) * mesh->materials.count);

			// state->models_count++;
			mi++;
		}
	}

	for (unsigned long ni = 0; ni < model.scene->nodes.count; ni++) {
		ufbx_node *node = model.scene->nodes.data[ni];

		if (node->is_root)
			continue;

		if (node->mesh) {
			printf("parsing new mesh !!!\n");
			ufbx_mesh *mesh = node->mesh;

			model.model_matrix.m11 = node->geometry_to_world.m00;

			model.model_matrix.m12 = node->geometry_to_world.m01;

			model.model_matrix.m13 = node->geometry_to_world.m02;

			model.model_matrix.m14 = node->geometry_to_world.m03;

			model.model_matrix.m21 = node->geometry_to_world.m10;

			model.model_matrix.m22 = node->geometry_to_world.m11;

			model.model_matrix.m23 = node->geometry_to_world.m12;

			model.model_matrix.m24 = node->geometry_to_world.m13;

			model.model_matrix.m31 = node->geometry_to_world.m20;

			model.model_matrix.m32 = node->geometry_to_world.m21;

			model.model_matrix.m33 = node->geometry_to_world.m22;

			model.model_matrix.m34 = node->geometry_to_world.m23;

			size_t max_triangles = 0;

			// We need to render each material of the mesh in a separate part, so let's
			// count the number of parts and maximum number of triangles needed.
			for (size_t pi = 0; pi < mesh->materials.count; pi++) {
				ufbx_mesh_material *mesh_mat = &mesh->materials.data[pi];
				if (mesh_mat->num_triangles == 0)
					continue;
				max_triangles = max_triangles < mesh_mat->num_triangles ? mesh_mat->num_triangles : max_triangles;
			}

			size_t num_tri_indices = node->mesh->max_face_triangles * 3;
			uint32_t *tri_indices = malloc(num_tri_indices * sizeof(uint32_t));

			uint32_t vertices_max = max_triangles * 3;
			printf("vertices_max = %u\n", vertices_max);
			mesh_vertex *vertices = malloc(sizeof(mesh_vertex) * vertices_max);
			memset(vertices, 0, sizeof(mesh_vertex) * vertices_max);

			uint32_t indices_max = max_triangles * 3;
			uint32_t *indices = malloc(sizeof(uint32_t) * indices_max);
			memset(indices, 0, sizeof(uint32_t) * indices_max);

			for (size_t pi = 0; pi < mesh->materials.count; pi++) {
				ufbx_mesh_material *mesh_mat = &mesh->materials.data[pi];
				if (mesh_mat->num_triangles == 0)
					continue;

				uint32_t num_indices = 0;
				memset(vertices, 0, sizeof(mesh_vertex) * vertices_max);

				for (size_t fi = 0; fi < mesh_mat->num_faces; fi++) {
					ufbx_face face = mesh->faces.data[mesh_mat->face_indices.data[fi]];
					size_t num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);

					ufbx_vec2 default_uv = {0};

					// Iterate through every vertex of every triangle in the triangulated result
					for (size_t vi = 0; vi < num_tris * 3; vi++) {
						uint32_t ix = tri_indices[vi];
						mesh_vertex *vert = &vertices[num_indices];

						ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
						ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);
						ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix) : default_uv;
						// ufbx_to_um_vec3(pos);
						vert->position = ufbx_to_um_vec3(pos);
						vert->normal = um_normalize3(ufbx_to_um_vec3(normal));
						vert->uv = ufbx_to_um_vec2(uv);
						vert->f_vertex_index = (float)mesh->vertex_indices.data[ix];

						// The skinning vertex stream is pre-calculated above so we just need to
						// copy the right one by the vertex index.
						// if (skin) {
						//   skin_vertices[num_indices] = mesh_skin_vertices[mesh->vertex_indices.data[ix]];
						// }

						num_indices++;
					}
				}

				printf("num_indices = %u\n", num_indices);
				ufbx_vertex_stream streams[2];
				size_t num_streams = 1;

				streams[0].data = vertices;
				streams[0].vertex_size = sizeof(mesh_vertex);

				/*
				if (skin) {
				       streams[1].data = skin_vertices;
				       streams[1].vertex_size = sizeof(skin_vertex);
				       num_streams = 2;
				}
				*/

				// Optimize the flat vertex buffer into an indexed one. `ufbx_generate_indices()`
				// compacts the vertex buffer and returns the number of used vertices.
				ufbx_error error;
				size_t num_vertices = ufbx_generate_indices(streams, num_streams, indices, num_indices, NULL, &error);
				if (error.type != UFBX_ERROR_NONE) {
					fprintf(stderr, "Failed to generate index buffer = %s", error.description.data);
					exit(1);
				}

				printf("num_indices = %u\n", num_indices);

				state->models[state->models_count].scene = model.scene;

				state->models[state->models_count].vertices_count[pi] = num_vertices;

				printf("state->models[state->models_count].vertices_count[%lu] = %u\n", pi, state->models[state->models_count].vertices_count[pi]);
				state->models[state->models_count].vertices[pi] = malloc(sizeof(Vertex) * num_vertices);
				memset(state->models[state->models_count].vertices[pi], 0, sizeof(Vertex) * num_vertices);

				state->models[state->models_count].indices_count[pi] = num_indices;
				state->models[state->models_count].indices[pi] = malloc(sizeof(uint32_t) * num_indices);
				memset(state->models[state->models_count].indices[pi], 0, sizeof(uint32_t) * num_indices);

				printf("state->models[state->models_count].indices_count[%lu] = %u\n", pi, state->models[state->models_count].indices_count[pi]);

				/*
				        state->vertices = malloc(sizeof(Vertex) * num_vertices);
				        memset(state->vertices, 0, sizeof(Vertex) * num_vertices);
				        state->indices = malloc(sizeof(uint32_t) * num_indices);
				        memset(state->indices, 0, sizeof(uint32_t) * num_indices);

				        state->vertex_count = num_vertices;
				        state->index_count = num_indices;
				*/
				// memcpy(state->indices, indices, sizeof(uint32_t) * num_indices);

				memcpy(state->models[state->models_count].indices[pi], indices, sizeof(uint32_t) * num_indices);

				for (uint32_t j = 0; j < num_vertices; j++) {
					/*state->vertices[j].pos = (struct vec3){vertices[j].position.x, vertices[j].position.y, vertices[j].position.z};
					state->vertices[j].texCoord = (struct vec2){vertices[j].uv.x, -vertices[j].uv.y};

					state->vertices[j].color = (struct vec3){1.0f, 1.0f, 1.0f};
					*/
					state->models[state->models_count].vertices[pi][j].pos = (struct vec3) {
						vertices[j].position.x, vertices[j].position.y, vertices[j].position.z
					};
					state->models[state->models_count].vertices[pi][j].texCoord = (struct vec2) {
						vertices[j].uv.x, -vertices[j].uv.y
					};

					state->models[state->models_count].vertices[pi][j].color = (struct vec3) {
						1.0f, 1.0f, 1.0f
					};

					// printf_vertex(state->vertices[i]);

					// state->models[state->models_count].indices[i][j] = j;
				}
			}

			// state->models[state->models_count].vertices = realloc(state->models[state->models_count].vertices, sizeof(Vertex *) * num_meshes);
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! state->models[state->models_count].vertices_count_count = %u\n",
			       state->models[state->models_count].vertices_count_count);
			// state->models[state->models_count].indices = realloc(state->models[state->models_count].indices, sizeof(uint32_t *) * num_meshes);
			printf("state->models[state->models_count].vertices_count_count = %u\n", state->models[state->models_count].vertices_count_count);

			// state->models[state->models_count].vertices_count = realloc(state->models[state->models_count].vertices_count, sizeof(uint32_t) * num_meshes);

			// state->models[state->models_count].indices_count = realloc(state->models[state->models_count].indices_count, sizeof(uint32_t) * num_meshes);
			// printf("num_meshes = %u\n", num_meshes);
			printf("state->models[state->models_count].vertices_count[%lu] = %u\n", state->models_count, state->models[state->models_count].vertices_count[0]);
			printf("state->models[state->models_count].indices_count[%lu] = %u\n", state->models_count, state->models[state->models_count].indices_count[0]);

			state->models_count++;
		}
	}
}

void init_vulkan(State *state) {

	Majid_model model = M_loadModel("./viking_room/viking_room.obj");
	pars_model(state, model);
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

	createCommandPool(state);
	// createVertexBuffer(state);
	createColorResources(state);
	createDepthResources(state);
	createFramebuffers(state);
	createTextureImage(state);
	createTextureImageView(state);
	createTextureSampler(state);

	createVertexIndexUniformBuffer_for_all_models(state);

	//  createVertexIndexUniformBuffer(state);
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
		// printf("avg fps = %u\n", fps);
		
		gettimeofday(&state->time, NULL);
		state->time.tv_sec = state->time.tv_sec - state->program_start_time.tv_sec;
	}

	vkDeviceWaitIdle(state->device);
}

void cleanup(State *state) {
	cleanupSwapChain(state);

	vkDestroySampler(state->device, state->textureSampler, NULL);

	vkDestroyImageView(state->device, state->textureImageView, NULL);

	vkDestroyImage(state->device, state->textureImage, NULL);
	vkFreeMemory(state->device, state->textureImageMemory, NULL);

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
	State state = init_state("vulkan", true, 1280, 720);

	glfwInit();
	create_window(&state);
	init_vulkan(&state);
	loop(&state);
	return 0;
}
