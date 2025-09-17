#include "camera.h"
#include "matrix.h"
#include <math.h>

camera_t camera;

void camera_init(void)
{
    camera.position         = (vec3_t){0, 0, 0};
    camera.forward          = (vec3_t){0, 0, 1};
    camera.direction        = camera.forward;
    camera.up               = (vec3_t){0, 1, 0}; // the camera's default up direction is world up
    camera.target           = vec3_add(camera.position, camera.direction);

    camera.yaw              = 0.0f;
    camera.pitch            = 0.0f;
    camera.speed            = 1.0f;
    camera.rotation_speed   = 1.0f;
    camera.forward_velocity = (vec3_t){0, 0, 0};
    camera.strafe_velocity  = (vec3_t){0, 0, 0};
}

void camera_update_direction(void)
{
    // Rotation in X and Y
    mat4_t Rx = mat4_make_rotation_x(camera.pitch);
    mat4_t Ry = mat4_make_rotation_y(camera.yaw);
    mat4_t R  = mat4_mul_mat4(Ry, Rx);

    // Rotate the baseline forward into the actual facing direction
    camera.direction = vec3_from_vec4(mat4_mul_vec4(R, vec4_from_vec3(camera.forward)));
    vec3_normalize(&camera.direction);

    // Update target for look_at
    camera.target = vec3_add(camera.position, camera.direction);
}