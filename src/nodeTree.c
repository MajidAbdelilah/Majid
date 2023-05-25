#include "nodeTree.h"
#include <stdlib.h>
#include "string.h"
#include "xxhash_header.h"

Node InitHashMap(unsigned long initSize){
	HashMapSize = initSize;
	hashMap = malloc(sizeof(Node) * initSize);
	memset(hashMap, 0, sizeof(Node) * initSize);
	Node parent = {0};
	strcpy(parent.name, "3D Scene\0");
	strcpy(parent.type, "3D Node\0");
	parent.propreties = malloc(sizeof(NodeProps));
	memset(parent.propreties, 0, sizeof(NodeProps));
	strcpy(((NodeProps*)parent.propreties)->name, parent.name);
	parent.hashMap = hashMap;
	XXH64_hash_t index = XXH3_64bits(parent.name, strlen(parent.name)) % initSize;
	hashMap[index] = parent;
	return parent;
}
