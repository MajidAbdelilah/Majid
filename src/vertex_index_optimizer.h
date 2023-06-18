#pragma once

#include "renderer_structs.h"



void simulate_vertex_index_cache_mises(Vertex *vertices, uint32_t v_count, uint32_t *indesice, uint32_t i_count, uint32_t cache_size);
void optimize_vertex_index_buffers_for_GPU(Vertex *vertices, uint32_t v_count, uint32_t *indesice, uint32_t i_count);

