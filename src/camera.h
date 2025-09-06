#pragma once

#include "vector.h"

typedef struct
{
    vec3_t position;         // Where the camera is in the world
    vec3_t direction;        // Which way it’s looking
    vec3_t forward_velocity; // Used for forward/backward movement
    float yaw;               // Rotation left/right (y-axis)
} camera_t;

extern camera_t camera;