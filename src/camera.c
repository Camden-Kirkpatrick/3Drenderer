#include "camera.h"

camera_t camera = {
    .position = {0.0f, 0.0f, 0.0f},
    .direction = {0.0f, 0.0f, 1.0f},
    .forward_velocity = {0.0f, 0.0f, 0.0f},
    .yaw = 0.0f
};