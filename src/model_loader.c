#define UMATH_IMPLEMENTATION
#include "model_loader.h"


#include <stdio.h>
#include <stdlib.h>


Majid_model M_loadModel(char *path) {
	Majid_model model = {0};

	ufbx_load_opts opts = { 0 }; // Optional, pass NULL for defaults
	ufbx_error error; // Optional, pass NULL if you don't care about errors
	model.scene = ufbx_load_file(path, &opts, &error);
	if (!model.scene) {
		fprintf(stderr, "Failed to load: %s\n", error.description.data);
	}



	// Let's just list all objects within the scene for example:
	for (size_t i = 0; i < model.scene->nodes.count; i++) {
		ufbx_node *node = model.scene->nodes.data[i];
		if (node->is_root) continue;

		printf("Object: %s\n", node->name.data);
		if (node->mesh) {

			printf("-> mesh with %zu faces\n", node->mesh->faces.count);
			printf("mesh with %lu vertices\n", node->mesh->vertices.count);

			printf("mesh with %lu triangles\n", node->mesh->num_triangles);


			printf("mesh with %lu materials\n", node->mesh->materials.count);


			printf("mesh with %lu indices\n", node->mesh->vertex_indices.count);

			printf("\n");

		}
	}

	printf("\n");
	return model;
}
