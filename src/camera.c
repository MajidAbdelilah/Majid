#include "camera.h"
#include <math.h>
#include "renderer.h"

struct vec3 cameraPos   = (struct vec3){0.0f, 0.0f,  -1.0f};
struct vec3 cameraFront = (struct vec3){0.0f, 0.0f, 1.0f};
struct vec3 cameraUp    = (struct vec3){0.0f, 1.0f,  0.0f};

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float yaw = -90.0f;
float pitch = 0.0f;

float lastX = 400, lastY = 300;

bool firstMouse = true;

void update_camera(Camera3D_r *camera) {

	//camera->camera.pos = (struct vec3){0.0f, 0.0f, 3.0f};
	//camera->camera.look_at = (struct vec3) {
	//	-2.0f, -2.0f, 0.0f
	//};

	
	psvec3_add(&camera->camera.look_at, &camera->camera.look_at, &camera->camera.pos);
	
	psvec3_subtract(&camera->camera.nor_direction, &camera->camera.pos, &camera->camera.look_at);
	psvec3_normalize(&camera->camera.nor_direction, &camera->camera.nor_direction);

	camera->camera.up.y = 1.0f;
	psvec3_cross(&camera->camera.right, &camera->camera.up, &camera->camera.nor_direction);
	psvec3_normalize(&camera->camera.right, &camera->camera.right);

	psvec3_cross(&camera->camera.up, &camera->camera.nor_direction, &camera->camera.right);
	psvec3_normalize(&camera->camera.up, &camera->camera.up);

	//psmat4_look_at(&camera->camera.view, &camera->camera.pos, &camera->camera.nor_direction, &camera->camera.up);

	float radius = 5.0f;
	//camera->camera.pos.x = sin(glfwGetTime()) * radius;
	//camera->camera.pos.z = cos(glfwGetTime()) * radius;
	
	camera->camera.look_at = (struct vec3){0.0f, 0.0f, -1.0f};
	camera->camera.up = (struct vec3){0.0f, 1.0f, 0.0f};;
	
	struct vec3 direction = {0.0f};
	direction.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
	direction.y = sin(to_radians(pitch));
	direction.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
	cameraFront = *psvec3_normalize(EV3, &direction);
	
	psmat4_look_at(&camera->camera.view, &cameraPos, psvec3_add(EV3, &cameraPos, &cameraFront), &cameraUp);
}


void processCameraInput(Camera3D_r *camera, GLFWwindow *window) {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;  
	
	const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		psvec3_add(&cameraPos, &cameraPos, psvec3_multiply_f(EV3, &cameraFront, cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		psvec3_subtract(&cameraPos, &cameraPos, psvec3_multiply_f(EV3, &cameraFront, cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		psvec3_subtract(&cameraPos, &cameraPos,  
			psvec3_multiply_f(EV3, psvec3_normalize(EV3, psvec3_cross(EV3,  &cameraFront, &cameraUp)), cameraSpeed));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		psvec3_add(&cameraPos, &cameraPos,  
			psvec3_multiply_f(EV3, psvec3_normalize(EV3, psvec3_cross(EV3,  &cameraFront, &cameraUp)), cameraSpeed));

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
float xoffset = xpos - lastX;
float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
lastX = xpos;
lastY = ypos;

const float sensitivity = 0.1f;
xoffset *= sensitivity;
yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;  

	if(pitch > 89.0f)
		pitch =  89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;
}
