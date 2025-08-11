#pragma once

#include "vector.h"
#include <stdint.h>

// This is used to store the vertex indices for each face
typedef struct
{
    int a;
    int b;
    int c;
    uint32_t color;
} face_t;

// This stores the actual points of the triangle on the screen
typedef struct
{
    vec2_t points[3];
    uint32_t color;
    float avg_depth;
} triangle_t;

void int_swap(int *a, int *b);
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);