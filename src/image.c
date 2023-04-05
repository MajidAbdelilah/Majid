#define STB_IMAGE_IMPLEMENTATION
#include "image.h"

M_image M_load_image(char *path){
  M_image image = {0};
    image.pixels = stbi_load(path, &image.texWidth, &image.texHeight, &image.texChannels, STBI_rgb_alpha);
    
  image.imageSize = image.texWidth * image.texHeight * 4;

    if (!image.pixels) {
        fprintf(stderr, "ERROR: failed to load texture image!\n");
    }else {
        printf("SUCCED to load texture image!\n"); 
    }
  return image;
}

