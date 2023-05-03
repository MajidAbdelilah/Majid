#pragma once

#include "renderer_structs.h"
#include <time.h>
#include <sys/time.h>

void init_renderer();
void renderer_loop();




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
	//VkBuffer *uniformBuffers;
	//VkDeviceMemory *uniformBuffersMemory;
	//void **uniformBuffersMapped;
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



