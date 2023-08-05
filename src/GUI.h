#pragma once

#include "mathc.h"


typedef struct Rect{
	struct vec2 min, max;
}Rect;

typedef struct GUI_Rect{
	struct vec2 v[4];
	unsigned char indices[6];
}GUI_Rect;


GUI_Rect Init_Gui_Rect(Rect rect);
