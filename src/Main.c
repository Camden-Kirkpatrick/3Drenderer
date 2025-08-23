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
#include "matrix.h"
#include "light.h"

bool is_running = false;
bool paused = false;
int previous_frame_time = 0;

vec3_t camera_position = {0, 0, 0};
mat4_t proj_matrix;

triangle_t *triangles_to_render = NULL;

// Used with the animate_rectangles function
// int rect_count = 1;

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

	// Projection Matrix for Perspective Projeciton
	float fov = M_PI / 3.0; // 60 degree fov
	float aspect_ratio = (float)window_width / window_height;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect_ratio, znear, zfar);

	// Projection Matrix for Orthographic Projection
	// float ortho_left = -1.0f;
	// float ortho_right = 1.0f;
	// float ortho_bottom = -1.0f;
	// float ortho_top = 1.0f;
	// proj_matrix = mat4_make_orthographic(
	// 	ortho_left, ortho_right,
	// 	ortho_bottom, ortho_top,
	// 	znear, zfar);

	// Load the mesh in mesh.h
	// load_cube_mesh_data();
	//  Load an object via an obj file
	load_obj_file_data("./assets/cube.obj", RED);
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

		case SDLK_5:
			render_method = RENDER_GOURAUD;
			break;

		case SDLK_6:
			render_method = RENDER_GOURAUD_WIRE;
			break;

		case SDLK_c:
			cull_method = CULL_BACKFACE;
			break;

		case SDLK_d:
			cull_method = CULL_NONE;
			break;

		case SDLK_RIGHT:
			mesh.translation.x += 0.03;
			break;

		case SDLK_LEFT:
			mesh.translation.x -= 0.03;
			break;

		case SDLK_UP:
			mesh.translation.y -= 0.03;
			break;

		case SDLK_DOWN:
			mesh.translation.y += 0.03;
			break;

		default:
			break;
		}
		break;
	}
}

void update(void)
{
	// We have to find the new triangles to render each frame, so we want to start with an empty array
	triangles_to_render = NULL;

	// Make sure the desired FPS is reached
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	// Change the mesh rotation/scale values per animation frame
	mesh.rotation.x += 0.005;
	mesh.rotation.y += 0.005;
	mesh.rotation.z += 0.005;
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
		vec3_t normal = cross(vec_ab, vec_ac);
		vec3_normalize(&normal);

		// Find the camera_ray from point A to the camera
		vec3_t camera_ray = vec3_sub(camera_position, vec_a);

		// Calculate how alligned the camera ray is with the normal
		float dot_product = vec3_dot(normal, camera_ray);

		if (cull_method == CULL_BACKFACE)
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
			// This is for Orthographic Projection
			// projected_points[j] = mat4_mul_vec4(proj_matrix, transformed_vertices[j]);

			// Scale each vertex, which will end up scaling the object
			projected_points[j].x *= (window_width / 2.0);
			projected_points[j].y *= (window_height / 2.0);
			// Translate each vertex so that they are inside our window
			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);
		}

		// Painter's Algorithm
		// Calculate the average depth for each face based on the vertices after transformation (Painter's Algorithm)
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

		// Flat Shading
		// Calculate the shade intensity based on how alligned the normal and the inverse of the light ray
		float light_intensity_factor = -vec3_dot(normal, light.direction);
		// Calculate the new color
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

		// This will store the final triangle to render
		triangle_t projected_triangle = {
			// Save each projected vertex in the triangle
			.points = {
				{projected_points[0].x, projected_points[0].y},
				{projected_points[1].x, projected_points[1].y},
				{projected_points[2].x, projected_points[2].y}},
			.color = triangle_color,
			.avg_depth = avg_depth};

		// Add the projected triangle to the array of traingles to render
		array_push(triangles_to_render, projected_triangle);
	}

	// Painter's Algorithm continued
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
	// Background color
	clear_color_buffer(BLACK);

	int num_triangles = array_length(triangles_to_render);

	// Render each triangle
	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];

		// Draw a filled triangle
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				triangle.color);
		}

		// Draw an unfilled triangle (wireframe)
		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				WHITE);
		}

		// Draw the traingle vertices
		if (render_method == RENDER_WIRE_VERTEX)
		{
			draw_rectangle(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, GREEN);
			draw_rectangle(triangle.points[1].x, triangle.points[1].y, 6, 6, GREEN);
			draw_rectangle(triangle.points[2].x, triangle.points[2].y, 6, 6, GREEN);
		}
	}

	// Free the triangles, since we have to recalculate them for the next frame
	array_free(triangles_to_render);

	// Copy the pixel data over to the SDL texture
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
			render();
		}
	}

	// Used with the animate_rectangles function
	// cleanup_rectangles();
	destroy_window();
	free_resources();

	return 0;
}