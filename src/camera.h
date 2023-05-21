#pragma once

#include "mathc.h"
#include <stdalign.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define EV3 (struct vec3[]){{0.0f}}
typedef struct Camera3D{
	alignas(16) struct vec3 pos;
	alignas(16) struct vec3 up;
	alignas(16) struct vec3 right;
	alignas(16) struct vec3 look_at;
	alignas(16) struct vec3 nor_direction;
	alignas(16) struct mat4 view;
	alignas(16) struct mat4 proj;
}Camera3D;

typedef struct Camera3D_r{
	Camera3D camera;
	
}Camera3D_r;

void update_camera(Camera3D_r *camera);
void processCameraInput(Camera3D_r *camera, GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
