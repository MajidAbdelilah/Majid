#pragma once

#include "ufbx.h"
#include "renderer_structs.h"
#include <bits/stdint-uintn.h> 
#include "image.h"
#include "ufbx/sokol_gfx.h"
#include "ufbx/umath.h"
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
    M_image *textures;
 
	  VkBuffer *vertexIndexUniformBuffer;
  VkDeviceMemory *vertexIndexUniformBufferMemory;
  VkBuffer *uniformBuffers;
  VkDeviceMemory *uniformBuffersMemory;
  void **uniformBuffersMapped;

 struct mat4 model_matrix;
} Majid_model;

Majid_model M_loadModel(char *path);

