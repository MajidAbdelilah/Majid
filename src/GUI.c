#include "renderer.h"
#include "renderer_structs.h"
#include "GUI.h"



GUI_Rect Init_Gui_Rect(Rect rect){
	GUI_Rect g_rect = {0};
	
	g_rect.v[0] = (struct vec2){rect.min.x, rect.min.y};
	g_rect.v[1] = (struct vec2){rect.min.x, rect.max.y};
	g_rect.v[2] = (struct vec2){rect.max.x, rect.max.y};
	g_rect.v[3] = (struct vec2){rect.max.x, rect.min.y};
	
	memcpy(g_rect.indices, (unsigned char[]){0, 1, 3, 3, 1, 2}, 6);
	
	return g_rect;
}


