#pragma once

#include "vector.h"

typedef struct
{
    vec3_t position;         // Where the camera is in the world
    vec3_t direction;        // Which way it’s looking
    vec3_t target;           // What point the camera is looking at
    vec3_t up;               // What direction "up" is for the camera
    vec3_t forward;          // Default camera look direction with no rotation
    float speed;             // How fast the camera moves
    float rotation_speed;    // How fast the camera rotates
    vec3_t forward_velocity; // Used for forward/backward movement
    vec3_t strafe_velocity;  // Used for right/left movement
    float yaw;               // Rotation left/right (y-axis)
    float pitch;             // Rotation up/down (x-axis)
} camera_t;

extern camera_t camera;

void camera_init(void);
void camera_update_direction(void);