#pragma once

#include "mathc.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "ufbx/sokol_gfx.h"
#include "ufbx/umath.h"
#include "image.h"
#include "ufbx.h"
#include <stdalign.h>

typedef struct Vertex{struct vec3 pos; struct vec3 color; struct vec2 texCoord;}Vertex;

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


typedef struct Majid_model {
	ufbx_scene *scene;
	Vertex **vertices;
	uint32_t *vertices_count;
	uint32_t vertices_count_count;
	uint32_t **indices;
	uint32_t *indices_count;
	uint32_t indices_count_count;

	VkBuffer *vertexIndexUniformBuffer;
	VkDeviceMemory *vertexIndexUniformBufferMemory;
	VkBuffer *uniformBuffers;
	VkDeviceMemory *uniformBuffersMemory;
	//void **uniformBuffersMapped;

	
	VkDescriptorSet *descriptorSets;
	uint32_t descriptorSets_count;
	

	M_image *textures;
	uint32_t mipLevels;
	VkImage *textureImage;
	VkDeviceMemory *textureImageMemory;
	VkImageView *textureImageView;
	VkSampler *textureSampler;
	uint32_t texture_count;
	bool update_ubo;
	
	UniformBufferObject ubo;
	FragShaderUniform *fsu;
	
	VkBuffer *fsuBuffers;
	VkDeviceMemory *fsuBuffersMemory;
	bool update_fsu;
	
	struct mat4 model_matrix;
} Majid_model;

