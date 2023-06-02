#pragma once
#include "mathc.h"
#include <stdlib.h>
typedef struct Node{
	char name[64];
	char type[64];
	struct Node *hashMap;
	void *data;
	void *propreties;
}Node;

typedef struct NodeProps{
	char name[64];
}NodeProps;

typedef struct TextureProps{
	char name[64];
	char path[128];
}TextureProps;



typedef struct MeshProps{
	char name[64];
	char path[128];
	TextureProps textureProps;
	struct vec4 color;
}MeshProps;

typedef struct ModelProps{
	char name[64];
	char path[128];
	MeshProps meshProps;
}ModelProps;

Node *hashMap = NULL;
unsigned long HashMapSize = 0;

Node InitHashMap(unsigned long initSize);


