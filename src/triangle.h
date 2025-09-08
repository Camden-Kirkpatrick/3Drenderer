#pragma once

#include "vector.h"
#include "texture.h"
#include "display.h"
#include <stdint.h>

// This is used to store the vertex indices for each face
typedef struct
{
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
    uint32_t color;
} face_t;

// This stores the actual points of the triangle on the screen
typedef struct
{
    vec4_t points[3];
    tex2_t texcoords[3];
    uint32_t color;
} triangle_t;

void draw_triangle(Window *w, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void draw_triangle_pixel(
    Window *w,
    int x, int y, uint32_t color,
    vec4_t point_a, vec4_t point_b, vec4_t point_c
);

void draw_filled_triangle(
    Window *w,
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t color
);

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

void draw_texel(
    Window *w,
    int x, int y, uint32_t *texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
);

void draw_textured_triangle(
    Window *w,
    uint32_t* texture,
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2
);