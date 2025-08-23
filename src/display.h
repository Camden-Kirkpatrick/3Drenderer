#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define FPS 60
// This is how long each frame should last
#define FRAME_TARGET_TIME (1000 / FPS)

// Colors in the ARGB color space
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF
#define DARK_BLUE 0xFF151C62
#define LIGHT_BLUE 0xFFADD8E6
#define YELLOW 0xFFFFFF00
#define MAGENTA 0xFFFF00FF
#define CYAN 0xFF00FFFF
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define GRAY 0xFF808080
#define DARK_GRAY 0xFF404040
#define LIGHT_GRAY 0xFFC0C0C0
#define ORANGE 0xFFFFA500
#define PURPLE 0xFF800080
#define PINK 0xFFFF69B4
#define BROWN 0xFF654321

enum Cull_Method
{
    CULL_NONE,
    CULL_BACKFACE
};

enum Render_Method
{
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_GOURAUD,
    RENDER_GOURAUD_WIRE
};

extern enum Cull_Method cull_method;
extern enum Render_Method render_method;

extern int window_width;
extern int window_height;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern uint32_t *color_buffer;
extern SDL_Texture *color_buffer_texture;

bool initialize_window(void);
void render_color_buffer(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_rectangle(int x, int y, int width, int height, uint32_t color);
void draw_grid(int x, int y, int width, int height, int line_spacing, uint32_t color);
uint32_t generate_random_color(void);
void animate_rectangles(int num_rects);
void draw_checker_board(int cell_width, int cell_height, int rows, int cols, uint32_t cell_color_1, uint32_t cell_color_2, uint32_t border_color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void clear_color_buffer(uint32_t color);
void cleanup_rectangles(void);
void destroy_window(void);