#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    SDL_Window   *sdl_window;
    SDL_Renderer *renderer;
    SDL_Texture  *color_buffer_texture;
    uint32_t     *color_buffer;
    float        *z_buffer;
    int           width;
    int           height;
} Window;

// ABGR8888 (for SDL_PIXELFORMAT_RGBA32 on little-endian)
#define RED        0xFF0000FF
#define GREEN      0xFF00FF00
#define BLUE       0xFFFF0000
#define DARK_BLUE  0xFF621C15
#define LIGHT_BLUE 0xFFE6D8AD
#define YELLOW     0xFF00FFFF
#define MAGENTA    0xFFFF00FF
#define CYAN       0xFFFFFF00
#define WHITE      0xFFFFFFFF
#define BLACK      0xFF000000
#define GRAY       0xFF808080
#define DARK_GRAY  0xFF404040
#define LIGHT_GRAY 0xFFC0C0C0
#define ORANGE     0xFF00A5FF
#define PURPLE     0xFF800080
#define PINK       0xFFB469FF
#define BROWN      0xFF214365

#define NUM_COLORS 17
extern uint32_t colors[NUM_COLORS];
extern uint32_t current_color;
extern size_t color_index;

enum Render_Method
{
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_FILL_TRIANGLE_WIRE_VERTEX,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE,
    RENDER_TEXTURED_WIRE_VERTEX
};

bool window_init(Window* w, int req_w, int req_h);
void render_color_buffer(Window *w);
void draw_pixel(Window *w, int x, int y, uint32_t color);
void draw_rectangle(Window *w, int x, int y, int width, int height, uint32_t color);
void draw_grid(Window *w, int x, int y, int width, int height, int line_spacing, uint32_t color);
void draw_dotted_grid(Window *w, int x, int y, int width, int height, int dot_spacing, uint32_t color);
uint32_t generate_random_color(Window *w);
void animate_rectangles(Window *w, int num_rects, float dt);
void draw_checker_board(Window *w, int cell_width, int cell_height, int rows, int cols, uint32_t cell_color_1, uint32_t cell_color_2, uint32_t border_color);
void draw_line(Window *w, int x0, int y0, int x1, int y1, uint32_t color);
void draw_filled_circle(Window *w, int cx, int cy, int rad, uint32_t color);
void clear_color_buffer(Window *w, uint32_t color);
void clear_z_buffer(Window *w);
void cleanup_rectangles(void);
void window_destroy(Window* w);