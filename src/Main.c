#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "upng.h"

bool is_running = false;
bool paused = false;
int previous_frame_time = 0;

vec3_t camera_position = {0, 0, 0};
mat4_t proj_matrix;

//triangle_t *triangles_to_render = NULL;

#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

// Used with the animate_rectangles function
int rect_count = 1;

int arrow_key_mode = 0;

void setup(void)
{
	current_color = colors[color_index];

	// Allocate the required memory in bytes to hold the color buffer and z-buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	// Creating an SDL texture that is used to display the color buffer
	color_buffer_texture = 
	SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	// Projection Matrix for Perspective Projeciton
	float fov = M_PI / 3.0; // 60 degree fov
	float aspect_ratio = (float)window_width / window_height;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect_ratio, znear, zfar);

	// Load the mesh in mesh.h
	//load_cube_mesh_data();
	// Load an object via an obj file
	load_obj_file_data("./assets/f117.obj", RED);

	load_png_texture_data("./assets/f117.png");
}

void process_input(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{ // always drain the queue
		if (event.type == SDL_QUIT)
		{
			is_running = false;
			break;
		}
		if (event.type == SDL_KEYDOWN)
		{
			SDL_Keycode k = event.key.keysym.sym;

			if (k == SDLK_ESCAPE)
			{
				is_running = false;
				continue;
			}
			if (k == SDLK_p)
			{
				paused = !paused;
				continue;
			}

			// ignore movement / mode changes while paused
			if (paused)
				continue;

			switch (k)
			{
			// Select a rendering method
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
			case SDLK_5:
				render_method = RENDER_FILL_TRIANGLE_WIRE_VERTEX;
				break;
			case SDLK_6:
				render_method = RENDER_TEXTURED;
				break;
			case SDLK_7:
				render_method = RENDER_TEXTURED_WIRE;
				break;
			case SDLK_8:
				render_method = RENDER_TEXTURED_WIRE_VERTEX;
				break;

			// Enable or disable Back-face Culling
			case SDLK_c:
				cull = !cull;
				break;

			// Toggle between modes for translating and rotating
			case SDLK_a:
				arrow_key_mode = (arrow_key_mode == 0 ? 1 : 0);
				break;
				
			// Translate or Rotate the object depending on the current mode
			if (arrow_key_mode == 0)
			{
				case SDLK_RIGHT:
					if (arrow_key_mode == 0)
						mesh.translation.x += 0.03f;
					else
						mesh.rotation.x += 0.05f;
					break;
				case SDLK_LEFT:
					if (arrow_key_mode == 0)
						mesh.translation.x -= 0.03f;
					else
						mesh.rotation.x -= 0.05f;
					break;
				case SDLK_UP:
					if (arrow_key_mode == 0)
						mesh.translation.y += 0.03f;
					else
						mesh.rotation.y += 0.05f;
					break;
				case SDLK_DOWN:
					if (arrow_key_mode == 0)
						mesh.translation.y -= 0.03f;
					else
						mesh.rotation.y -= 0.05f;
					break;
			}

			case SDLK_k:
				mesh.rotation.z += 0.05f;
				break;
			case SDLK_j:
				mesh.rotation.z -= 0.05f;
				break;

			// Grow objects
			case SDLK_EQUALS:
				const float MAX_SCALE = 2.0f;

				if (mesh.scale.x < MAX_SCALE)
					mesh.scale.x += 0.01f;
				if (mesh.scale.x < MAX_SCALE)
					mesh.scale.y += 0.01f;
				if (mesh.scale.x < MAX_SCALE)
					mesh.scale.z += 0.01f;
				break;
			// Shrink objects
			case SDLK_MINUS:
				const float MIN_SCALE = 0.01f;

				if (mesh.scale.x > MIN_SCALE)
					mesh.scale.x -= 0.01f;
				if (mesh.scale.y > MIN_SCALE)
					mesh.scale.y -= 0.01f;
				if (mesh.scale.z > MIN_SCALE)
					mesh.scale.z -= 0.01f;
				break;

			// Cycle through different colors for an object
			case SDLK_m:
				color_index = (color_index + 1) % NUM_COLORS;
				current_color = colors[color_index];
				break;
			case SDLK_n:
				color_index = (color_index - 1) % NUM_COLORS;
				current_color = colors[color_index];
				break;
			// Generate a random color for an object
			case SDLK_r:
				current_color = generate_random_color();
				break;

			// Enable or disable lighting
			case SDLK_l:
				lighting = !lighting;
				break;
			}
		}
	}
}

void update(void)
{
	// We have to find the new triangles to render each frame, so we want to start with an empty array
	// if (triangles_to_render)
	// {
	// 	array_free(triangles_to_render);
	// 	triangles_to_render = NULL;
	// }

	// Make sure the desired FPS is reached
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	num_triangles_to_render = 0;

	// Change the mesh rotation/scale values per animation frame
	// mesh.rotation.x += 0.005;
	// mesh.rotation.y += 0.005;
	// mesh.rotation.z += 0.005;
	// Translate the vertex away from the camera
	mesh.translation.z = 5.0;

	// Create a scale, translation, and rotation matrix that will be used to transform our mesh vertices
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Create a World Matrix that combines our scale, rotation, and translation matrices
	mat4_t world_matrix = mat4_identity();

	// Order matters: First scale, then rotate, then translate (AxB != BxA)
	world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
	world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

	int num_faces = array_length(mesh.faces);
	// Loop all triangle faces of our mesh
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];
		mesh_face.color = current_color;
		vec3_t face_vertices[3];

		// Get the 3 vertices for each face
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		// Loop each vertice on the face and apply transformations
		for (int j = 0; j < 3; j++)
		{
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
			// Multiply the World Matrix by the original vector
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
			transformed_vertices[j] = transformed_vertex;
		}

		// Backface Culling Algorithm

		// These are the vectors that make up the triangle
		vec3_t vec_a = vec3_from_vec4(transformed_vertices[0]);
		vec3_t vec_b = vec3_from_vec4(transformed_vertices[1]);
		vec3_t vec_c = vec3_from_vec4(transformed_vertices[2]);

		// Find the 2 edge vectors to compute the normal with
		vec3_t vec_ab = vec3_sub(vec_b, vec_a);
		vec3_t vec_ac = vec3_sub(vec_c, vec_a);

		// Compute the normal
		vec3_t normal = vec3_cross(vec_ab, vec_ac);
		vec3_normalize(&normal);

		// Find the camera_ray from point A to the camera
		vec3_t camera_ray = vec3_sub(camera_position, vec_a);

		// Calculate how alligned the camera ray is with the normal
		float dot_product = vec3_dot(normal, camera_ray);

		if (cull)
		{
			if (dot_product <= 0)
			{
				continue;
			}
		}

		vec4_t projected_points[3];

		for (int j = 0; j < 3; j++)
		{
			// Project the vertex
			projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

			// Invert the y-axis to account for y growing top-down
			projected_points[j].y *= -1;

			// Scale each vertex, which will end up scaling the object
			projected_points[j].x *= (window_width / 2.0);
			projected_points[j].y *= (window_height / 2.0);

			// Translate each vertex so that they are inside our window
			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);
		}

		uint32_t triangle_color;
		// Flat Shading
		if (lighting)
		{
			// Calculate the shade intensity based on how alligned the normal and the inverse of the light ray
			float light_intensity_factor = -vec3_dot(normal, light.direction);
			// Calculate the new color
			triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
		}
		else
		{
			triangle_color = mesh_face.color;
		}

		// This will store the final triangle to render
		triangle_t projected_triangle = {
			// Save each projected vertex in the triangle
			.points =
			{
				{projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
				{projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
				{projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w}
			},
			.texcoords =
			{
				{mesh_face.a_uv.u, mesh_face.a_uv.v},
				{mesh_face.b_uv.u, mesh_face.b_uv.v},
				{mesh_face.c_uv.u, mesh_face.c_uv.v}
			},
			.color = triangle_color,
		};

        // Save the projected triangle in the array of triangles to render
        if (num_triangles_to_render < MAX_TRIANGLES) {
            triangles_to_render[num_triangles_to_render++] = projected_triangle;
        }

		// Add the projected triangle to the array of traingles to render
		//array_push(triangles_to_render, projected_triangle);
	}
}

void render(void)
{
	// Background color
	clear_color_buffer(BLACK);
	clear_z_buffer();

	// draw_filled_circle(950, 1000, 200, ORANGE);
	// draw_filled_circle(2900, 1000, 200, CYAN);

	//int num_triangles = array_length(triangles_to_render);
	int num_triangles = num_triangles_to_render;

	// Render each triangle
	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];

		// Draw a filled triangle
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX)
		{
			draw_filled_triangle
			(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
				triangle.color
			);
		}

		if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE || render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			// We need to pass in the z and w components too, so we can have a perspective correct texture
			draw_textured_triangle
			(
				mesh_texture,
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v
			);
		}

		// Draw an unfilled triangle (wireframe)
		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX || render_method == RENDER_TEXTURED_WIRE || render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			draw_triangle
			(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				WHITE
			);
		}

		// Draw the triangle vertices
		if (render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX || render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			draw_rectangle(triangle.points[0].x - 1, triangle.points[0].y - 1, 3, 3, GREEN);
			draw_rectangle(triangle.points[1].x - 1, triangle.points[1].y - 1, 3, 3, GREEN);
			draw_rectangle(triangle.points[2].x - 1, triangle.points[2].y - 1, 3, 3, GREEN);
		}
	}

	// draw_dotted_grid(0, 0, window_width - 1, window_height - 1, 42, RED);

	// draw_filled_circle(1925, 200, 200, GREEN);
	// draw_filled_circle(1925, 1900, 200, YELLOW);

	// animate_rectangles(rect_count);

	// Copy the pixel data over to the SDL texture
	render_color_buffer();

	// Draw the new frame
	SDL_RenderPresent(renderer);
}

void free_resources(void)
{
	free(color_buffer);
	free(z_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
	upng_free(png_texture);
}

int main(int argc, char **argv)
{
	is_running = initialize_window();

	// Used with the animate_rectangles function
	// rect_count = atoi(argv[1]);
	// if (rect_count < 1)
	// 	rect_count = 1;

	srand((unsigned)time(NULL));

	setup();

	while (is_running)
	{
		process_input();
		if (!paused)
		{
			update();
		}
		render();
	}

	// Used with the animate_rectangles function
	// cleanup_rectangles();
	destroy_window();
	free_resources();

	return 0;
}