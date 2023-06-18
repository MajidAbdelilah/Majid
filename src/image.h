#pragma once

#include <string.h>
#include <vulkan/vulkan.h>
#include "turbojpeg.h"

typedef struct ImageHandler {
	tjhandle  tjHandle;	
}ImageHandler;

typedef struct M_image {
  int texWidth, texHeight, texChannels;
	unsigned char *pixels; 
  VkDeviceSize imageSize; 
} M_image;




M_image M_load_image(const char *path);

void InitImageHandler(void);
