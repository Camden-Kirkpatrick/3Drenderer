#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

bool is_running = false;
bool paused = false;
int previous_frame_time = 0;

// This scale factor puts space between the points on the 9x9x9 cube
float scale_factor = 1000;

// This is used to move the points of the cube so that they are all visible
vec3_t camera_position = {0, 0, 0};

triangle_t *triangles_to_render = NULL;

int rect_count = 1;
int frame_count = 0;

void setup(void)
{
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

	// Allocate the required memory in bytes to hold the color buffer
	color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

	// Creating an SDL texture that is used to display the color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height);

	//load_cube_mesh_data();
	load_obj_file_data("./assets/f22.obj");
}

void process_input(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			is_running = false;
			break;

		case SDLK_p:
			paused = !paused;
			break;

		case SDLK_1:
			render_method = RENDER_WIRE;
			break;

		case SDLK_2:
			render_method = RENDER_WIRE_VERTEX;
			break;

		case SDLK_3:
			render_method = RENDER_FILL_TRIANGLE;
			break;

		case SDLK_4:
			render_method = RENDER_FILL_TRIANGLE_WIRE;
			break;

		case SDLK_c:
			cull_method = CULL_BACKFACE;
			break;

		case SDLK_d:
			cull_method = CULL_NONE;
			break;

		default:
			// Do nothing for unhandled keys
			break;
		}
		break;
	}
}

// Orthographic Projection
vec2_t ortho_proj(vec3_t point)
{
	vec2_t projected_point = {
		.x = (point.x * scale_factor),
		.y = (point.y * scale_factor)};

	return projected_point;
}

// Perspective Projection
vec2_t persp_proj(vec3_t point)
{
	vec2_t projected_point = {
		.x = (point.x * scale_factor) / point.z,
		.y = (point.y * scale_factor) / point.z,
	};

	return projected_point;
}

void update(void)
{
	frame_count++;

	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	// Initialize the arry of triangles to render
	triangles_to_render = NULL;

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	//mesh.rotation.z += 0.01;

	int num_faces = array_length(mesh.faces);
	// Loop all triangle faces of our mesh
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];
		vec3_t face_vertices[3];

		// Get the 3 vertices for each face
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];

		// Loop each vertice on this face and apply transformations
		for (int j = 0; j < 3; j++)
		{
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// Translate the vertex away from the camera
			transformed_vertex.z += 5;

			transformed_vertices[j] = transformed_vertex;
		}

		// Backface Culling Algorithm
		if (cull_method == CULL_BACKFACE)
		{
			// These are the vectors that make up the triangle
			vec3_t vec_a = transformed_vertices[0];
			vec3_t vec_b = transformed_vertices[1];
			vec3_t vec_c = transformed_vertices[2];

			// Find the 2 edge vectors to compute the normal with
			vec3_t vec_ab = vec3_sub(vec_b, vec_a);
			vec3_t vec_ac = vec3_sub(vec_c, vec_a);

			// Compute the normal
			vec3_t normal = cross(vec_ab, vec_ac);
			vec3_normalize(&normal);

			// Find the camera_ray from point A to the camera
			vec3_t camera_ray = vec3_sub(camera_position, vec_a);

			// Calculate how alligned the camera ray is with the normal
			float dot_product = vec3_dot(normal, camera_ray);

			if (dot_product <= 0)
			{
				continue;
			}
		}

		vec2_t projected_points[3];

		for (int j = 0; j < 3; j++)
		{
			// Project the vertex and translate it
			projected_points[j] = persp_proj(transformed_vertices[j]);
			projected_points[j].x += (window_width / 2);
			projected_points[j].y += (window_height / 2);
		}

		// Calculate the average depth for each face based on the vertices after transformation
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

		// This will store the final triangle to render
		triangle_t projected_triangle = {
			// Save each projected vertex in the triangle
			.points = {
				{projected_points[0].x, projected_points[0].y},
				{projected_points[1].x, projected_points[1].y},
				{projected_points[2].x, projected_points[2].y}},
			.color = mesh_face.color,
			.avg_depth = avg_depth};

		// Add the projected triangle to the array of traingles to render
		array_push(triangles_to_render, projected_triangle);
	}

	// Sort the triangles to render by their avg_depth
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++)
	{
		for (int j = i; j < num_triangles; j++)
		{
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
			{
				// Swap the triangles positions in the array
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}

void render(void)
{
	clear_color_buffer(BLACK);

	int num_triangles = array_length(triangles_to_render);

	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];

		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				LIGHT_GRAY);
		}

		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				WHITE);
		}

		if (render_method == RENDER_WIRE_VERTEX)
		{
			draw_rectangle(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, GREEN);
			draw_rectangle(triangle.points[1].x, triangle.points[1].y, 6, 6, GREEN);
			draw_rectangle(triangle.points[2].x, triangle.points[2].y, 6, 6, GREEN);
		}
	}

	array_free(triangles_to_render);

	//// Back face (z = –1)
	//// (-1, –1, –1)
	// draw_rectangle(1545, 705, 5, 5, 0xFF00FF00);
	//// ( 1, –1, –1)
	// draw_rectangle(2295, 705, 5, 5, 0xFF00FF00);
	//// (–1,  1, –1)
	// draw_rectangle(1545, 1455, 5, 5, 0xFF0000FF);
	//// ( 1,  1, –1)
	// draw_rectangle(2295, 1455, 5, 5, 0xFF0000FF);

	//// Front face (z = +1)
	//// (–1, –1,  1)
	// draw_rectangle(1670, 830, 5, 5, 0xFF00FF00);
	//// ( 1, –1,  1)
	// draw_rectangle(2170, 830, 5, 5, 0xFF00FF00);
	//// (–1,  1,  1)
	// draw_rectangle(1670, 1330, 5, 5, 0xFF0000FF);
	//// ( 1,  1,  1)
	// draw_rectangle(2170, 1330, 5, 5, 0xFF0000FF);

	// Draw a grid
	// draw_grid(500, 500, 500, 500, 10, 0xFF00FF00);
	// draw_grid(1000, 500, 500, 500, 10, 0xFFFFFF00);
	// draw_grid(500, 1000, 500, 500, 10, 0xFF0000FF);
	// draw_grid(1000, 1000, 500, 500, 10, 0xFFFF0000);

	// animate_rectangles(rect_count);

	// draw_checker_board(10, 10, 55, 55, WHITE, BLACK, ORANGE);

	render_color_buffer();

	// Draw the new frame
	SDL_RenderPresent(renderer);
}

void free_resources(void)
{
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char **argv)
{
	is_running = initialize_window();

	// choose how many rectangles via command-line (or default to 5)
	if (argc > 1)
	{
		rect_count = atoi(argv[1]);
		if (rect_count < 1)
			rect_count = 1;
	}

	srand((unsigned)time(NULL));

	setup();

	while (is_running)
	{
		process_input();
		if (!paused)
		{
			update();
			render();
		}
	}

	// cleanup_rectangles();
	destroy_window();
	free_resources();

	return 0;
}