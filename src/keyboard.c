#include <SDL2/SDL.h>
#include <stdbool.h>
#include "keyboard.h"
#include "mesh.h"
#include "display.h"
#include "camera.h"
#include "light.h"
#include "app.h"
#include "mathdefs.h"

static const float MAX_FOVY = DEG2RAD(120);
static const float MIN_FOVY = DEG2RAD(30);
static const float INC_FOVY = DEG2RAD(1);

// Clamp pitch to just under +/-90°
static const float MAX_PITCH = DEG2RAD(89.9f);
static const float INC_CAMERA_SPEED = 0.25f;

static bool k_w, k_a, k_s, k_d, k_space, k_lshift;
static bool k_left, k_right, k_up, k_down;

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
            case SDLK_SPACE:  k_space  = true; break;
            case SDLK_LSHIFT: k_lshift = true; break;

            case SDLK_w: k_w = true; break;
            case SDLK_s: k_s = true; break;
            case SDLK_d: k_d = true; break;
            case SDLK_a: k_a = true; break;

            case SDLK_LEFT:  k_left  = true;  break;
            case SDLK_RIGHT: k_right = true;  break;
            case SDLK_UP:    k_up    = true;  break;
            case SDLK_DOWN:  k_down  = true;  break;

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

			case SDLK_9:
				if (app->fovy > MIN_FOVY)
				{
					app->fovy -= INC_FOVY;
					app->fovx = atanf(tanf(app->fovy / 2) * app->aspectx) * 2.0f;
				}
				break;
			case SDLK_0:
				if (app->fovy < MAX_FOVY)
				{
					app->fovy += INC_FOVY;
					app->fovx = atanf(tanf(app->fovy / 2) * app->aspectx) * 2.0f;
				}
				break;
				

			// Enable or disable Back-face Culling
			case SDLK_c:
				app->cull = !(app->cull);
				break;

			case SDLK_EQUALS:
				camera.speed += INC_CAMERA_SPEED;
				break;
			case SDLK_MINUS:
				if (camera.speed > 0)
					camera.speed -= INC_CAMERA_SPEED;
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
				app->lighting = !(app->lighting);
				break;
			}
		}

        else if (event.type == SDL_KEYUP)
        {
            SDL_Keycode k = event.key.keysym.sym;
            switch (k)
            {
                case SDLK_SPACE:  k_space  = false; break;
                case SDLK_LSHIFT: k_lshift = false; break;
                case SDLK_w: k_w = false; break;
                case SDLK_s: k_s = false; break;
                case SDLK_d: k_d = false; break;
                case SDLK_a: k_a = false; break;

                case SDLK_LEFT:  k_left  = false; break;
                case SDLK_RIGHT: k_right = false; break;
                case SDLK_UP:    k_up    = false; break;
                case SDLK_DOWN:  k_down  = false; break;
            }
        }
	}

    if (!app->paused)
    {
        float spd = camera.speed * app->delta_time;

        // vertical
        if (k_space)  camera.position.y += spd;
        if (k_lshift) camera.position.y -= spd;

        // forward/back
        if (k_w)
        {
            vec3_t v = vec3_mul(camera.direction, spd);
            camera.position = vec3_add(camera.position, v);
        }
        if (k_s)
        {
            vec3_t v = vec3_mul(camera.direction, spd);
            camera.position = vec3_sub(camera.position, v);
        }

        // strafe
        if (k_d || k_a)
        {
            vec3_t right = vec3_cross(camera.up, camera.direction); // left-handed
            vec3_normalize(&right);
            if (k_d)
            {
                vec3_t v = vec3_mul(right, spd);
                camera.position = vec3_add(camera.position, v);
            }
            if (k_a)
            {
                vec3_t v = vec3_mul(right, spd);
                camera.position = vec3_sub(camera.position, v);
            }
        }

        float rot = camera.rotation_speed * app->delta_time;

        if (k_left)  camera.yaw   -= rot;
        if (k_right) camera.yaw   += rot;
        if (k_up)    camera.pitch -= rot;
        if (k_down)  camera.pitch += rot;

		if (camera.pitch >  MAX_PITCH) camera.pitch =  MAX_PITCH;
		if (camera.pitch < -MAX_PITCH) camera.pitch = -MAX_PITCH;
    }
}