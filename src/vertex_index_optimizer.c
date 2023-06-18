#include "vertex_index_optimizer.h"
#include <stdlib.h>
#include <string.h>
#include "meshoptimizer.h"

void simulate_vertex_index_cache_mises(Vertex *vertices, uint32_t v_count, uint32_t *indesice, uint32_t i_count, uint32_t cache_size) {
	unsigned int vertex_size = sizeof(Vertex);
	unsigned int index = indesice[0];

	unsigned int cache_misses = 0;

	for (uint32_t i = 1; i < i_count; i++) {
		if (indesice[i] > index && ((indesice[i] - index) * vertex_size) > cache_size) {
			
			//cache miss
			cache_misses++;
		}else if (indesice[i] < index && ((indesice[i] - index) * vertex_size) < -(int)cache_size) {
			//cache miss
			cache_misses++;
		}
		index = indesice[i];
	}
	printf("cache size = %u\n", cache_size);
	printf("cache misses = %u\n", cache_misses);
	printf("cache mises percentage = %f\n", (((float)cache_misses) / i_count) * 100);
	
}


void optimize_vertex_index_buffers_for_GPU(Vertex *vertices, uint32_t v_count, uint32_t *indices, uint32_t i_count){
	
	//uint32_t *destIndices = malloc(sizeof(uint32_t) * i_count);
	meshopt_optimizeVertexCache(indices, indices, i_count, v_count);
	//memcpy(indices, destIndices, sizeof(uint32_t) * i_count);

	struct vec3 *vertices_vec3 = malloc(sizeof(struct vec3) * v_count);
	
	for(unsigned int i=0; i<v_count; i++){
		vertices_vec3[i] = (struct vec3){vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z};
	}
	
	meshopt_optimizeOverdraw(indices, indices, i_count, &vertices_vec3[0].x, v_count, sizeof(struct vec3), 1.05f);
	//memcpy(indices, destIndices, sizeof(uint32_t) * i_count);
	
	
	
	meshopt_optimizeVertexFetch(vertices, indices, i_count, vertices, v_count, sizeof(Vertex));
	
	
	//free(destIndices);
	free(vertices_vec3);
	
}
