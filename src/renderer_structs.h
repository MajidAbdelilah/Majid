#pragma once

#include "mathc.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "ufbx/sokol_gfx.h"
#include "ufbx/umath.h"
#include "image.h"
#include "ufbx.h"
#include <stdalign.h>

#define MAX_FRAMES_IN_FLIGHT 2


typedef struct Vertex{
	alignas(16)	struct vec3 pos; 
	alignas(8) struct vec2 texCoord;
}Vertex;

typedef struct FragShaderUniform{
	alignas(16) struct vec4 transparency_color;
	alignas(1) bool is_no_texture;	
}FragShaderUniform;

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



typedef struct mesh_vertex {
	um_vec3 position;
	um_vec3 normal;
	um_vec2 uv;
	float f_vertex_index;
} mesh_vertex;

typedef struct skin_vertex {
	uint8_t bone_index[4];
	uint8_t bone_weight[4];
} skin_vertex;

static const sg_layout_desc mesh_vertex_layout = {
	.attrs = {
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT2 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT },
	},
};

static const sg_layout_desc skinned_mesh_vertex_layout = {
	.attrs = {
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT2 },
{ .buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT },
{ .buffer_index = 1, .format = SG_VERTEXFORMAT_BYTE4 },
{ .buffer_index = 1, .format = SG_VERTEXFORMAT_UBYTE4N },
	},
};

typedef struct Mesh{
	Vertex *vertices;
	uint32_t verticesCount;
	uint32_t *indices;
	uint32_t indicesCount;
	VkBuffer vertexIndexUniformBuffer;
	VkDeviceMemory vertexIndexUniformBufferMemory;
	
	VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
	VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
	VkBuffer fsuBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory fsuBuffersMemory[MAX_FRAMES_IN_FLIGHT];
	
	M_image textures;
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	
	UniformBufferObject ubo;
	FragShaderUniform fsu;
	
	bool updateFsu;
	bool updateUbo;
	
}Mesh;


typedef struct Model {
	ufbx_scene *scene;
	Mesh *meshes;
	unsigned int meshCount;
} Majid_model;

