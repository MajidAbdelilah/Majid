#include "renderer.h"
#include "GLFW/glfw3.h"
#include "image.h"

int main(int argnum, char **args) {
		InitImageHandler();
		init_renderer();
		exit(0);
		renderer_loop();
	
	 
	return 0;
}
