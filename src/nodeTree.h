#pragma once


typedef struct Node{
	Node *children;
	unsigned int children_num;
	Node *hashMap;
	void *data;
	char type[64];
	void *propreties;
}Node;

typedef struct TextureProps{
	char path[128];
}TextureProps;

typedef struct ModelProps{
	char path[128];
	
}ModelProps;

