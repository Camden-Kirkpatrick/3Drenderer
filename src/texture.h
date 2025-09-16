#pragma once

#include <stdint.h>
#include "upng.h"

typedef struct
{
    float u;
    float v;
} tex2_t;

extern upng_t* png_texture;
extern uint32_t* mesh_texture;

extern int texture_width;
extern int texture_height;

void load_png_texture_data(const char* filepath);
tex2_t tex2_clone(tex2_t *t);