#pragma once

#include <stb_image.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "turbojpeg.h"

typedef struct ImageHandler {
	tjhandle  tjHandle;	
}ImageHandler;

typedef struct M_image {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels; 
  VkDeviceSize imageSize; 
} M_image;




M_image M_load_image(const char *path);

void InitImageHandler(void);
