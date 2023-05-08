#pragma once

#include <stb_image.h>
#include <string.h>
#include <vulkan/vulkan.h>

typedef struct M_image_handler {

} M_image_handler;

typedef struct M_image {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels; 
  VkDeviceSize imageSize; 
} M_image;



M_image M_load_image(const char *path);
 
