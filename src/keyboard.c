#include <SDL2/SDL.h>
#include <stdbool.h>
#include "keyboard.h"
#include "mesh.h"
#include "display.h"
#include "camera.h"
#include "light.h"
#include "app.h"

void process_input(AppState *app)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{ 
		if (event.type == SDL_QUIT)
		{
			app->is_running = false;
			break;
		}
		if (event.type == SDL_KEYDOWN)
		{
			SDL_Keycode k = event.key.keysym.sym;

			if (k == SDLK_ESCAPE)
			{
				app->is_running = false;
				continue;
			}
			if (k == SDLK_p)
			{
				app->paused = !(app->paused);
				continue;
			}

			if (k == SDLK_RIGHTBRACKET)
			{            
				if (app->fps < 240) app->fps += 5;   
				app->frame_target_time = 1000 / app->fps;
				continue;
			}
			if (k == SDLK_LEFTBRACKET)
			{             
				if (app->fps > 1) app->fps -= 5;   
				if (app->fps < 1) app->fps = 1;      
				app->frame_target_time = 1000 / app->fps;
				continue;
			}

			// ignore movement / mode changes while paused
			if (app->paused)
				continue;

			switch (k)
			{
			// Select a rendering method
			case SDLK_1:
				app->render_method = RENDER_WIRE;
				break;
			case SDLK_2:
				app->render_method = RENDER_WIRE_VERTEX;
				break;
			case SDLK_3:
				app->render_method = RENDER_FILL_TRIANGLE;
				break;
			case SDLK_4:
				app->render_method = RENDER_FILL_TRIANGLE_WIRE;
				break;
			case SDLK_5:
				app->render_method = RENDER_FILL_TRIANGLE_WIRE_VERTEX;
				break;
			case SDLK_6:
				app->render_method = RENDER_TEXTURED;
				break;
			case SDLK_7:
				app->render_method = RENDER_TEXTURED_WIRE;
				break;
			case SDLK_8:
				app->render_method = RENDER_TEXTURED_WIRE_VERTEX;
				break;

			// Enable or disable Back-face Culling
			case SDLK_c:
				app->cull = !(app->cull);
				break;

			case SDLK_b:
				app->arrow_key_mode = (app->arrow_key_mode == 0 ? 1 : 0);
				break;

			case SDLK_k:
				mesh.rotation.z += 1.0f * app->delta_time;
				break;
			case SDLK_j:
				mesh.rotation.z -= 0.5f * app->delta_time;
				break;

			case SDLK_EQUALS:
				camera.speed++;
				break;
			case SDLK_MINUS:
				if (camera.speed > 0)
					camera.speed--;
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
				current_color = generate_random_color(&app->win);
				break;

			// Enable or disable lighting
			case SDLK_l:
				lighting = !lighting;
				break;

			case SDLK_SPACE:
				camera.position.y += camera.speed * app->delta_time;
				break;
			case SDLK_LSHIFT:
				camera.position.y -= camera.speed * app->delta_time;
				break;

            case SDLK_w:
                camera.forward_velocity = vec3_mul(camera.direction, camera.speed * app->delta_time);
                camera.position = vec3_add(camera.position, camera.forward_velocity);
                break;
            case SDLK_s:
                camera.forward_velocity = vec3_mul(camera.direction, camera.speed * app->delta_time);
                camera.position = vec3_sub(camera.position, camera.forward_velocity);
                break;

            case SDLK_d:
            {
                vec3_t right = vec3_cross(camera.up, camera.direction);
                vec3_normalize(&right);
                camera.strafe_velocity = vec3_mul(right, camera.speed * app->delta_time);
                camera.position = vec3_add(camera.position, camera.strafe_velocity);
            } break;

            case SDLK_a:
            {
                vec3_t right = vec3_cross(camera.up, camera.direction);
                vec3_normalize(&right);
                camera.strafe_velocity = vec3_mul(right, camera.speed * app->delta_time);
                camera.position = vec3_sub(camera.position, camera.strafe_velocity);
            } break;

			case SDLK_LEFT:
				camera.yaw -= camera.rotation_speed * app->delta_time;
				break;
			case SDLK_RIGHT:
				camera.yaw += camera.rotation_speed * app->delta_time;
				break;

			case SDLK_UP:
				camera.pitch -= camera.rotation_speed * app->delta_time;
				break;
			case SDLK_DOWN:
				camera.pitch += camera.rotation_speed * app->delta_time;
				break;
			}
		}
	}
}