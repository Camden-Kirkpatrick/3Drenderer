#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "upng.h"
#include "camera.h"
#include "clipping.h"
#include "keyboard.h"
#include "mesh.h"
#include "app.h"

int previous_frame_time = 0;
int was_paused = 0;

mat4_t world_matrix;
mat4_t view_matrix;
mat4_t proj_matrix;

//triangle_t *triangles_to_render = NULL;

#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

// Used with the animate_rectangles function
int rect_count = 20;

void setup(AppState *app)
{
	app->paused = false,
	app->fps = 60;
	app->frame_target_time = 1000 / app->fps; // time between each frame
	app->delta_time = 0.0f;
    app->arrow_key_mode = 0; // toggle between translating and rotating the mesh
	app->render_method = RENDER_WIRE;
	app->cull = true;

	current_color = colors[color_index];

	camera_init();

	// Projection Matrix for Perspective Projeciton
	float aspectx = (float)app->win.width / app->win.height;
	float aspecty = (float)app->win.height / app->win.width;
	float fovy = M_PI / 3.0f; // 60 degree fov
	float fovx = atanf(tanf(fovy / 2) * aspectx) * 2.0f;
	float z_near = 0.1f;
	float z_far = 100.0f;
	proj_matrix = mat4_make_perspective(fovy, aspectx, z_near, z_far);

	// Initialize frustum planes with a point and a normal
	init_frustum_planes(fovx, fovy, z_near, z_far);

	// Load the mesh in mesh.h
	load_cube_mesh_data();
	// Load an object via an obj file
	//load_obj_file_data("./assets/cube.obj", RED);
	//load_png_texture_data("./assets/cube.png");
}

void update(AppState *app)
{
	// We have to find the new triangles to render each frame, so we want to start with an empty array
	// if (triangles_to_render)
	// {
	// 	array_free(triangles_to_render);
	// 	triangles_to_render = NULL;
	// }

	// Make sure the desired FPS is reached
	int time_to_wait = app->frame_target_time - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < app->frame_target_time)
		SDL_Delay(time_to_wait);

	// Delta time is the time since the previous frame, and it is used for consistent animations, regardless of FPS
	app->delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0f;

	previous_frame_time = SDL_GetTicks();

	num_triangles_to_render = 0;

	// Apply transformations to the mesh every frame
	// mesh.rotation.x += 0.5f * app->delta_time;
	// mesh.rotation.y += 0.5f * app->delta_time;
	// mesh.rotation.z += 0.5f * app->delta_time;

	// Translate the mesh away from the camera
	mesh.translation.z = 5.0f;

	// Apply transformations to the camera every frame
	// camera.position.x += 0.5f * app->delta_time;
	// camera.position.y += 0.5f * app->delta_time;
	// camera.position.z += 0.5f * app->delta_time;

	camera_update_direction();
	view_matrix = mat4_look_at(camera.position, camera.target, camera.up);

	// Create a scale, translation, and rotation matrix that will be used to transform our mesh vertices
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Create a World Matrix that combines our scale, rotation, and translation matrices
	world_matrix = mat4_identity();

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
			// Multiply the View Matrix by the new World Space vector
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

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
		vec3_t origin = {0, 0, 0};
		vec3_t camera_ray = vec3_sub(origin, vec_a);

		// Calculate how alligned the camera ray is with the normal
		float dot_product = vec3_dot(normal, camera_ray);

		if (app->cull)
		{
			if (dot_product <= 0.0f)
			{
				continue;
			}
		}

		// Clip the triangle
		polygon_t polygon = create_polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]),
			vec3_from_vec4(transformed_vertices[1]),
			vec3_from_vec4(transformed_vertices[2])
		);

		// Clip the polygon and return a new polygon that has been modified
		clip_polygon(&polygon);

		// Break the clipped polygon into triangles
		triangle_t triangles_after_clipping[MAX_NUM_POLYGON_TRIANGLES];
		int num_triangles_after_clipping = 0;
		triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

		// Loop all the assembled triangles after clipping
		for (int t = 0; t < num_triangles_after_clipping; t++)
		{
			triangle_t triangle_after_clipping = triangles_after_clipping[t];

			vec4_t projected_points[3];

			for (int j = 0; j < 3; j++)
			{
				// Project the vertex
				projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

				// Invert the y-axis to account for y growing top-down
				projected_points[j].y *= -1;

				// Scale each vertex, which will end up scaling the object
				projected_points[j].x *= (app->win.width / 2.0f);
				projected_points[j].y *= (app->win.height / 2.0f);

				// Translate each vertex so that they are inside our window
				projected_points[j].x += (app->win.width / 2.0f);
				projected_points[j].y += (app->win.height / 2.0f);
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
			triangle_t triangle_to_render = {
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
				triangles_to_render[num_triangles_to_render++] = triangle_to_render;
			}

			// Add the projected triangle to the array of traingles to render
			//array_push(triangles_to_render, projected_triangle);
		}
	}
}

void render(AppState *app)
{
	// Background color
	clear_color_buffer(&app->win, BLACK); 
	// Set every pixels depth to 1.0
	clear_z_buffer(&app->win);

	// draw_filled_circle(950, 1000, 200, ORANGE);
	// draw_filled_circle(2900, 1000, 200, CYAN);

	draw_dotted_grid(&app->win, 0, 0, app->win.width - 1, app->win.height - 1, 30, DARK_GRAY);

	//int num_triangles = array_length(triangles_to_render);
	int num_triangles = num_triangles_to_render;

	// Render each triangle
	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];

		// Draw a filled triangle
		if (app->render_method == RENDER_FILL_TRIANGLE || app->render_method == RENDER_FILL_TRIANGLE_WIRE || app->render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX)
		{
			draw_filled_triangle
			(
				&app->win,
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
				triangle.color
			);
		}

		if (app->render_method == RENDER_TEXTURED || app->render_method == RENDER_TEXTURED_WIRE || app->render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			// We need to pass in the z and w components too, so we can have a perspective correct texture
			draw_textured_triangle
			(
				&app->win,
				mesh_texture,
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v
			);
		}

		// Draw an unfilled triangle (wireframe)
		if (app->render_method == RENDER_WIRE || app->render_method == RENDER_WIRE_VERTEX || app->render_method == RENDER_FILL_TRIANGLE_WIRE || app->render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX || app->render_method == RENDER_TEXTURED_WIRE || app->render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			draw_triangle
			(
				&app->win,
				triangle.points[0].x, triangle.points[0].y,
				triangle.points[1].x, triangle.points[1].y,
				triangle.points[2].x, triangle.points[2].y,
				WHITE
			);
		}

		// Draw the triangle vertices
		if (app->render_method == RENDER_WIRE_VERTEX || app->render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX || app->render_method == RENDER_TEXTURED_WIRE_VERTEX)
		{
			draw_rectangle(&app->win, triangle.points[0].x - 1, triangle.points[0].y - 1, 3, 3, GREEN);
			draw_rectangle(&app->win, triangle.points[1].x - 1, triangle.points[1].y - 1, 3, 3, GREEN);
			draw_rectangle(&app->win, triangle.points[2].x - 1, triangle.points[2].y - 1, 3, 3, GREEN);
		}
	}

	//animate_rectangles(&app->win, rect_count, app->paused ? 0.0f : app->delta_time);

	// draw_filled_circle(&app->win, 1925, 200, 200, GREEN);
	// draw_filled_circle(&app->win, 1925, 1900, 200, YELLOW);

	// draw_checker_board(&app->win, 40, 40, app->win.height/40, app->win.width/40, LIGHT_GRAY, DARK_GRAY, WHITE);

	// Copy the pixel data over to the SDL texture
	render_color_buffer(&app->win);

	// Draw the new frame
	SDL_RenderPresent(app->win.renderer);
}

void free_resources(void)
{
	array_free(mesh.faces);
	array_free(mesh.vertices);
	if (png_texture)
	{
        upng_free(png_texture);
        png_texture = NULL;
    }
}

int main(int argc, char **argv)
{
	AppState app;

	app.is_running = 
	(argc > 2) ? window_init(&app.win, atoi(argv[1]), atoi(argv[2]))
                                : window_init(&app.win, 0, 0);

	srand((unsigned)time(NULL));

	setup(&app);

	while (app.is_running)
	{
		process_input(&app);

		// This prevents the next delta_time calculation from including the whole
		// pause duration (which would otherwise make dt huge on the first update).
		if (was_paused && !app.paused)
			previous_frame_time = SDL_GetTicks();

    	was_paused = app.paused;

		if (!(app.paused))
		{
			update(&app);
		}
		render(&app);
	}

	window_destroy(&app.win);
	free_resources();

	return 0;
}