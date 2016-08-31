#include "input_handler.h"


void handle_keyboard(vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector, const float* movement_speed)
{
  // This is the direction the camera is looking
  vec3 translator;
  vec3 direction = Normalize(VectorSub(*cameraLocation, *lookAtPoint));
  vec3 projected_direction = Normalize((vec3){direction.x, 0, direction.z});

  if ( glutKeyIsDown('w') ) {
    translator = ScalarMult(projected_direction,-*movement_speed);
    *cameraLocation = VectorAdd(*cameraLocation, ScalarMult(projected_direction, -*movement_speed));
    *lookAtPoint = VectorAdd(*lookAtPoint, translator);
  }
  if (glutKeyIsDown('d')) {
    translator = ScalarMult(Normalize(CrossProduct(projected_direction, *upVector)), -*movement_speed);
    *cameraLocation = VectorAdd(*cameraLocation, translator);
    *lookAtPoint = VectorAdd(*lookAtPoint, translator);
  }
  if ( glutKeyIsDown('a') ) {
    translator = ScalarMult(Normalize(CrossProduct(projected_direction, *upVector)), *movement_speed);
    *cameraLocation = VectorAdd(*cameraLocation, translator);
    *lookAtPoint = VectorAdd(*lookAtPoint, translator);
  }
  if ( glutKeyIsDown('s') ) {
    translator = ScalarMult(projected_direction,*movement_speed);
    *cameraLocation = VectorAdd(*cameraLocation, translator);
    *lookAtPoint = VectorAdd(*lookAtPoint, translator);
  }
}

// -----------------------------------

void handle_mouse(int x, int y, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector)
{
  // Only initialized once
  static float last_x = 0.0;
  static float last_y = 0.0;
  static float deltax;
  static float deltay;
  // End of only initialized once

  #ifdef __APPLE__
  int window_center_x = glutGet(GLUT_WINDOW_WIDTH)/2;
  int window_center_y = glutGet(GLUT_WINDOW_HEIGHT)/2;
  #elif defined __linux__
  int window_center_x = 150;
  int window_center_y = 150;
  #endif

  deltax = (float)x - last_x;
  deltay = (float)y - last_y;

  last_x = x;
  last_y = y;

  //if the mouse does large changes quickly (for example during a warp, ignore the change)
  if((abs((int)deltax)>50) || (abs((int)deltay)>50))
  {
    deltax = 0;
    deltay = 0;

    last_x = (float)x;
    last_y = (float)y;

  }

  // Do rotation
  yaw(deltax, mouse_speed, cameraLocation, lookAtPoint, upVector);
  pitch(deltay, mouse_speed, cameraLocation, lookAtPoint, upVector);

  /*Fix for quartz issue found at http://stackoverflow.com/questions/10196603/using-cgeventsourcesetlocaleventssuppressioninterval-instead-of-the-deprecated/17547015#17547015
  */
  // if mouse wander off too much, warp it back.
  int dist = (window_center_x < window_center_y) ? window_center_x/2: window_center_y/2;

  if(x > window_center_x+dist || x < window_center_x-dist || y > window_center_y+dist || y < window_center_y-dist){
    #ifdef __APPLE__ // Fix for OS X >= 10.10, which MicroGlut does not work with
    CGPoint warpPoint = CGPointMake(window_center_x, window_center_y);
    CGWarpMouseCursorPosition(warpPoint);
    CGAssociateMouseAndMouseCursorPosition(true);
    #endif
    #ifndef __APPLE__
    glutWarpPointer( window_center_x, window_center_y );
    #endif
  }
}

// ------------------------


static void yaw(float deltax, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector)
{
  // Does yaw
  vec3 look_at_vector = VectorSub(*lookAtPoint, *cameraLocation);
  mat4 translation_matrix = T(look_at_vector.x, look_at_vector.y, look_at_vector.z);
  *lookAtPoint = MultVec3(Mult(Ry(-deltax*mouse_speed), translation_matrix), *cameraLocation);
}

// ---

static void pitch(float deltay, float mouse_speed, vec3* cameraLocation, vec3* lookAtPoint, const vec3* upVector)
{
  // Do pitch if the angle between upVector and look_at_vector isn't too small
  vec3 look_at_vector = VectorSub(*lookAtPoint, *cameraLocation);

  mat4 translation_matrix = T(look_at_vector.x, look_at_vector.y, look_at_vector.z);
  vec3 rotation_axis = ScalarMult(CrossProduct(look_at_vector, *upVector), -1);
  *lookAtPoint = MultVec3(Mult(ArbRotate(rotation_axis, deltay*mouse_speed), translation_matrix), *cameraLocation);
}


// ------------------------

mat4 move_skybox(vec3* cameraLocation)
{
  return T(cameraLocation->x, cameraLocation->y-0.2, cameraLocation->z);
}
