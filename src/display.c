#include "display.h"
#include <time.h>
#include <math.h>
#include <stdio.h>

uint32_t colors[NUM_COLORS] = {
	RED,
	GREEN,
	BLUE,
	DARK_BLUE,
	LIGHT_BLUE,
	YELLOW,
	MAGENTA,
	CYAN,
	WHITE,
	BLACK,
	GRAY,
	DARK_GRAY,
	LIGHT_GRAY,
	ORANGE,
	PURPLE,
	PINK,
	BROWN,
};

uint32_t current_color;
size_t color_index = 0;

// Used with the animate_rectangles function
static bool *inited = NULL;
static int *size = NULL;
static int *rx = NULL;
static int *ry = NULL;
static int *rvx = NULL;
static int *rvy = NULL;
static uint32_t *rcolor = NULL;
static int allocated = 0;

static float *ax = NULL;  // subpixel accumulator X
static float *ay = NULL;  // subpixel accumulator Y

bool window_init(Window* w, int req_w, int req_h)
{
    SDL_Init(SDL_INIT_EVERYTHING);


    if (req_w > 0 && req_h > 0)
	{
        w->width  = req_w;
        w->height = req_h;
    }
	else
	{
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        w->width  = dm.w;
        w->height = dm.h;
    }

    w->sdl_window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                					 w->width, w->height, SDL_WINDOW_BORDERLESS);
    if (!w->sdl_window) return false;

    w->renderer = SDL_CreateRenderer(w->sdl_window, -1, 0);
    w->color_buffer = malloc(sizeof(uint32_t) * w->width * w->height);
    w->z_buffer     = malloc(sizeof(float)     * w->width * w->height);
    w->color_buffer_texture = SDL_CreateTexture(
        w->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        w->width, w->height
    );
    return w->renderer && w->color_buffer && w->z_buffer && w->color_buffer_texture;
}



void render_color_buffer(Window *w)
{
	// Copy pixel data from the CPU buffer into the SDL texture
	SDL_UpdateTexture(
		w->color_buffer_texture,
		NULL,
		w->color_buffer,
		(int)(w->width * sizeof(uint32_t)));
	// Queue the texture to be drawn on the screen (but not shown yet)
	SDL_RenderCopy(w->renderer, w->color_buffer_texture, NULL, NULL);
}

void draw_pixel(Window *w, int x, int y, uint32_t color)
{
    if (x < 0 || x >= w->width || y < 0 || y >= w->height)
        return;
    w->color_buffer[(w->width * y) + x] = color;
}

void draw_rectangle(Window *w, int x, int y, int width, int height, uint32_t color)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// We always add x/y, since that is the starting horizontal/vertical position of the rectangle
			draw_pixel(w, x + j, y + i, color);
		}
	}
}

void draw_grid(Window *w, int x, int y, int width, int height, int line_spacing, uint32_t color)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// We always add x/y, since that is the starting horizontal/vertical position of the rectangle
			int current_x = x + j;
			int current_y = y + i;

			if ((current_y % line_spacing == 0) || (current_x % line_spacing == 0))
			{
				draw_pixel(w, current_x, current_y, color);
			}
		}
	}
}

void draw_dotted_grid(Window *w, int x, int y, int width, int height, int dot_spacing, uint32_t color)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// We always add x/y, since that is the starting horizontal/vertical position of the rectangle
			int current_x = x + j;
			int current_y = y + i;

			if ((current_y % dot_spacing == 0) && (current_x % dot_spacing == 0))
			{
				draw_pixel(w, current_x, current_y, color);
			}
		}
	}
}

uint32_t generate_random_color(Window *w)
{
	uint32_t color;

	uint8_t a = 0xFF; // Fixed alpha channel (fully opaque)
	// RGB values are random -> [0, 255]
	uint8_t r = rand() & 0xFF;
	uint8_t g = rand() & 0xFF;
	uint8_t b = rand() & 0xFF;

	// Combine channels into a single 32-bit ARGB value
	color = (a << 24) | (r << 16) | (g << 8) | b;

	return color; // Return the last generated color
}

// Create a bunch of random rectangles that bounce around the window
void animate_rectangles(Window *w, int num_rects, float dt)
{
	if (num_rects <= 0)
		return;

	// (Re)allocate if we need more slots
	if (allocated < num_rects)
	{
		free(inited);
		free(rx);
		free(ry);
		free(rvx);
		free(rvy);
		free(rcolor);
		free(size);
		free(ax);
		free(ay);

		inited = calloc(num_rects, sizeof(bool));
		size = malloc(num_rects * sizeof(int));
		rx = malloc(num_rects * sizeof(int));
		ry = malloc(num_rects * sizeof(int));
		rvx = malloc(num_rects * sizeof(int));
		rvy = malloc(num_rects * sizeof(int));
		rcolor = malloc(num_rects * sizeof(uint32_t));
		ax = calloc(num_rects, sizeof(float));
		ay = calloc(num_rects, sizeof(float));

		allocated = num_rects;
	}

	for (int i = 0; i < num_rects; i++)
	{
		// initialize each rect only once
		if (!inited[i])
		{
			// Randomize size
			size[i] = (rand() % 100) + 1;
			// Random starting position, ensuring it fits in the window
			rx[i] = rand() % (w->width - size[i] + 1);
			ry[i] = rand() % (w->height - size[i] + 1);
			do
			{
				// Randomize velocity, ensuring it is not zero
				rvx[i] = (rand() % 20) + 1;
				rvy[i] = (rand() % 20) + 1;
			} while (rvx[i] == 0 && rvy[i] == 0);

			// find the average of the two velocities
			int avg_speed = (rvx[i] + rvy[i]) / 2;

			// set the color based on the average speed
			if (avg_speed < 5)
				rcolor[i] = DARK_BLUE;
			else if (avg_speed < 10)
				rcolor[i] = BLUE;
			else if (avg_speed < 15)
				rcolor[i] = LIGHT_BLUE;
			else if (avg_speed < 20)
				rcolor[i] = CYAN;
			else
				rcolor[i] = GREEN;

			// rcolor[i] = generate_random_color();
			inited[i] = true;
		}

		// accumulators for x and y
		ax[i] += 50 * rvx[i] * dt;
		ay[i] += 50 * rvy[i] * dt;

		int dx = (int)ax[i];
		int dy = (int)ay[i];

		ax[i] -= dx;   // keep the fractional remainder
		ay[i] -= dy;

		rx[i] += dx;
		ry[i] += dy;

		// bounce off left/right
		if (rx[i] <= 0)
		{
			rx[i] = 0;
			rvx[i] = -rvx[i];
		}
		else if (rx[i] + size[i] >= w->width)
		{
			rx[i] = w->width - size[i];
			rvx[i] = -rvx[i];
		}

		// bounce off top/bottom
		if (ry[i] <= 0)
		{
			ry[i] = 0;
			rvy[i] = -rvy[i];
		}
		else if (ry[i] + size[i] >= w->height)
		{
			ry[i] = w->height - size[i];
			rvy[i] = -rvy[i];
		}

		draw_rectangle(w, rx[i], ry[i], size[i], size[i], rcolor[i]);
	}
}

void draw_checker_board(Window *w, int cell_width, int cell_height, int rows, int cols, uint32_t cell_color_1, uint32_t cell_color_2, uint32_t border_color)
{
	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			// Determine the color based on the checkerboard pattern
			uint32_t color = (x + y) % 2 == 0 ? cell_color_1 : cell_color_2;

			// Calculate the top-left coordinates for the current cell
			int top_left_x = x * cell_width;
			int top_left_y = y * cell_height;

			// Draw the border of the checkerboard
			if (x == 0 || y == 0 || x == cols - 1 || y == rows - 1)
				draw_rectangle(w, top_left_x, top_left_y, cell_width, cell_height, border_color);
			// Draw the checkerboard
			else
				draw_rectangle(w, top_left_x, top_left_y, cell_width, cell_height, color);
		}
	}
}

void draw_line(Window *w, int x0, int y0, int x1, int y1, uint32_t color)
{
	int delta_x = x1 - x0;
	int delta_y = y1 - y0;

	int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

	float x_inc = delta_x / (float)longest_side_length;
	float y_inc = delta_y / (float)longest_side_length;

	float current_x = (float)x0;
	float current_y = (float)y0;

	for (int i = 0; i <= longest_side_length; i++)
	{
		draw_pixel(w, round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

void draw_filled_circle(Window *w, int cx, int cy, int rad, uint32_t color)
{
	int r2 = rad * rad;
	for (int y = -rad; y <= rad; y++)
	{
		for (int x = -rad; x <= rad; x++)
		{
			if (x * x + y * y <= r2)
			{
				draw_pixel(w, cx + x, cy + y, color);
			}
		}
	}
}

void clear_color_buffer(Window *w, uint32_t color)
{
	for (int i = 0; i < w->width * w->height; i++)
	{
		w->color_buffer[i] = color;
	}
}

void clear_z_buffer(Window *w)
{
	for (int i = 0; i <  w->width * w->height; i++)
	{
		w->z_buffer[i] = 1.0f;
	}
}

void cleanup_rectangles(void)
{
	free(inited);
	free(size);
	free(rx);
	free(ry);
	free(rvx);
	free(rvy);
	free(rcolor);
	free(ax); 
	free(ay); 
	inited = NULL;
	size = NULL;
	rx = NULL;
	ry = NULL;
	rvx = NULL;
	rvy = NULL;
	rcolor = NULL;
	ax = NULL;
	ay = NULL;
	allocated = 0;
}


void window_destroy(Window* w)
{
    if (!w) return;
    cleanup_rectangles(); // keep your rectangles cleanup
    if (w->color_buffer_texture) { SDL_DestroyTexture(w->color_buffer_texture); w->color_buffer_texture = NULL; }
    free(w->color_buffer); w->color_buffer = NULL;
    free(w->z_buffer);     w->z_buffer     = NULL;
    if (w->renderer)   { SDL_DestroyRenderer(w->renderer);   w->renderer   = NULL; }
    if (w->sdl_window) { SDL_DestroyWindow(w->sdl_window);   w->sdl_window = NULL; }
    SDL_Quit();
}
