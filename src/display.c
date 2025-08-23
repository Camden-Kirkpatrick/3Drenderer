#include "display.h"
#include <time.h>
#include <math.h>

int window_width = 800;
int window_height = 600;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

enum Cull_Method cull_method;
enum Render_Method render_method;

bool *inited = NULL;
int *size = NULL;
int *rx = NULL;
int *ry = NULL;
int *rvx = NULL;
int *rvy = NULL;
uint32_t *rcolor = NULL;
int allocated = 0;

bool initialize_window(void)
{
	// Intialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL!\n");
		return false;
	}

	// Use SDL to query what the max width and height of our screen is
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	// Set the window's width and height to the monitor's width and height
	window_width = display_mode.w;
	window_height = display_mode.h;

	// Create SDL window
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS);
	if (!window)
	{
		fprintf(stderr, "Error creating SDL window!\n");
		return false;
	}

	// Create an SDL renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer!\n");
		return false;
	}

	// This changes the video mode to fullscreen
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	return true;
}

void render_color_buffer(void)
{
	// Copy pixel data from the CPU buffer into the SDL texture
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)(window_width * sizeof(uint32_t)));
	// Queue the texture to be drawn on the screen (but not shown yet)
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void draw_pixel(int x, int y, uint32_t color)
{
	// Make sure the pixel is in bounds of the window
	if (x < window_width && x >= 0 && y < window_height && y >= 0)
		color_buffer[(window_width * y) + x] = color;
	// else
	// 	printf("Out of range: (%d, %d)\n", x, y);
}

void draw_rectangle(int x, int y, int width, int height, uint32_t color)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// We always add x/y, since that is the starting horizontal/vertical position of the rectangle
			draw_pixel(x + j, y + i, color);
		}
	}
}

void draw_grid(int x, int y, int width, int height, int line_spacing, uint32_t color)
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
				draw_pixel(current_x, current_y, color);
			}
		}
	}
}

uint32_t generate_random_color(void)
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

void animate_rectangles(int num_rects)
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

		inited = calloc(num_rects, sizeof(bool));
		size = malloc(num_rects * sizeof(int));
		rx = malloc(num_rects * sizeof(int));
		ry = malloc(num_rects * sizeof(int));
		rvx = malloc(num_rects * sizeof(int));
		rvy = malloc(num_rects * sizeof(int));
		rcolor = malloc(num_rects * sizeof(uint32_t));

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
			rx[i] = rand() % (window_width - size[i] + 1);
			ry[i] = rand() % (window_height - size[i] + 1);
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

		// move
		rx[i] += rvx[i];
		ry[i] += rvy[i];

		// bounce off left/right
		if (rx[i] <= 0)
		{
			rx[i] = 0;
			rvx[i] = -rvx[i];
		}
		else if (rx[i] + size[i] >= window_width)
		{
			rx[i] = window_width - size[i];
			rvx[i] = -rvx[i];
		}

		// bounce off top/bottom
		if (ry[i] <= 0)
		{
			ry[i] = 0;
			rvy[i] = -rvy[i];
		}
		else if (ry[i] + size[i] >= window_height)
		{
			ry[i] = window_height - size[i];
			rvy[i] = -rvy[i];
		}

		draw_rectangle(rx[i], ry[i], size[i], size[i], rcolor[i]);
	}
}

void draw_checker_board(int cell_width, int cell_height, int rows, int cols, uint32_t cell_color_1, uint32_t cell_color_2, uint32_t border_color)
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
				draw_rectangle(top_left_x, top_left_y, cell_width, cell_height, border_color);
			// Draw the checkerboard
			else
				draw_rectangle(top_left_x, top_left_y, cell_width, cell_height, color);
		}
	}
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
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
		draw_pixel(round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
}

void clear_color_buffer(uint32_t color)
{
	// Go through each row(y) and each column(x) and set the color of each pixel
	for (int y = 0; y < window_height; y++)
	{
		for (int x = 0; x < window_width; x++)
		{
			// Find the correct offset in the color_buffer for the next pixel
			color_buffer[(window_width * y) + x] = color;
		}
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
	inited = NULL;
	size = NULL;
	rx = NULL;
	ry = NULL;
	rvx = NULL;
	rvy = NULL;
	rcolor = NULL;
	allocated = 0;
}

void destroy_window(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}