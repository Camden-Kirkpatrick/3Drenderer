#include <math.h>
#include "vector.h"

///////////////////////////////////////////////////////////////////////
// 2D VECTOR FUNCTIONS
///////////////////////////////////////////////////////////////////////
float vec2_length(vec2_t v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

vec2_t vec2_add(vec2_t a, vec2_t b)
{
    return (vec2_t){a.x + b.x, a.y + b.y};
}

vec2_t vec2_sub(vec2_t a, vec2_t b)
{
    return (vec2_t){a.x - b.x, a.y - b.y};
}

vec2_t vec2_mul(vec2_t v, float s)
{
    return (vec2_t){v.x * s, v.y * s};
}

vec2_t vec2_div(vec2_t v, float s)
{
    return (vec2_t){v.x / s, v.y / s};
}

float vec2_dot(vec2_t a, vec2_t b)
{
    return ((a.x * b.x) + (a.y * b.y));
}

void vec2_normalize(vec2_t *v)
{
    float length = vec2_length(*v);

    v->x /= length;
    v->y /= length;
}

///////////////////////////////////////////////////////////////////////
// 3D VECTOR FUNCTIONS
///////////////////////////////////////////////////////////////////////
float vec3_length(vec3_t v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_add(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3_t vec3_mul(vec3_t v, float s)
{
    return (vec3_t){v.x * s, v.y * s, v.z * s};
}

vec3_t vec3_div(vec3_t v, float s)
{
    return (vec3_t){v.x / s, v.y / s, v.z / s};
}

float vec3_dot(vec3_t a, vec3_t b)
{
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

void vec3_normalize(vec3_t *v)
{
    float length = vec3_length(*v);

    v->x /= length;
    v->y /= length;
    v->z /= length;
}

vec3_t cross(vec3_t a, vec3_t b)
{
    vec3_t result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

// Rotation of vectors
vec3_t vec3_rotate_x(vec3_t v, float angle)
{
    vec3_t rotated_vector = {
        .x = v.x,
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle)};
    return rotated_vector;
}

vec3_t vec3_rotate_y(vec3_t v, float angle)
{
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.z * sin(angle),
        .y = v.y,
        .z = v.x * sin(angle) + v.z * cos(angle)};
    return rotated_vector;
}

vec3_t vec3_rotate_z(vec3_t v, float angle)
{
    vec3_t rotated_vector = {
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z};
    return rotated_vector;
}