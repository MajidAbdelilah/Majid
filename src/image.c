#define STB_IMAGE_IMPLEMENTATION
#include "image.h"
#include "io.h"

ImageHandler IH = {0};

void InitImageHandler(void){
	IH.tjHandle = tj3Init(TJINIT_DECOMPRESS);
}

M_image M_load_image(const char *path){
  M_image image = {0};
    /*
			
	image.pixels = stbi_load(path, &image.texWidth, &image.texHeight, &image.texChannels, STBI_rgb_alpha);
    
  image.imageSize = image.texWidth * image.texHeight * 4;

    if (!image.pixels) {
        fprintf(stderr, "ERROR: failed to load texture image!\n");
    }else {
        printf("SUCCED to load texture image!\n"); 
    }
  */
	
	File_S jpegfile = readFile(path);
	tj3DecompressHeader(IH.tjHandle, (const unsigned char*)jpegfile.data, jpegfile.size);
	image.texWidth = tj3Get(IH.tjHandle, TJPARAM_JPEGWIDTH);
	image.texHeight = tj3Get(IH.tjHandle, TJPARAM_JPEGHEIGHT);
	
	printf("image.texWidth = %u, image.texHeight = %u\n", image.texWidth, image.texHeight);
	
	unsigned char *RGBpixels = malloc(image.texHeight * image.texWidth * 3);
	memset(RGBpixels, 0, image.texHeight * image.texWidth * 3);
	
	
	
	tj3Decompress8(IH.tjHandle, (const unsigned char*)jpegfile.data, jpegfile.size, RGBpixels, image.texWidth * 3, TJPF_RGB);
	
	/*
		
	for(unsigned int i=0; i<image.texHeight; i++){
		for(unsigned int j=0; j<image.texWidth; j++){
			printf("pixel[%u][%u] = {%u, %u, %u}", i, j, RGBpixels[i*image.texWidth+j + 0], RGBpixels[i*image.texWidth+j + 1], RGBpixels[i*image.texWidth+j + 2]);
		}
	}
	
	 */
	
image.pixels = RGBpixels;
	image.imageSize = image.texHeight * image.texWidth * 3;
	
	return image;
}

