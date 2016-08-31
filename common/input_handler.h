#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "VectorUtils3.h"
#include "GL_utilities.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "mac/MicroGlut.h"
#include <ApplicationServices/ApplicationServices.h>
#elif defined __linux__
#define GL_GLEXT_PROTOTYPES
#include "Linux/MicroGlut.h"
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
#endif


void handle_keyboard(vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector, const float* movement_speed);

void handle_mouse(int x, int y, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector);
static void yaw(float deltax, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector);
static void pitch(float deltay, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector);

mat4 move_skybox(vec3* cameraLocation);

#endif // INPUT_HANDLER_H
