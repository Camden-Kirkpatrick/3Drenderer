#include "app.h"
#include <math.h>

void app_init(AppState *app, int texture_choice)
{
    app->paused = false,
	app->was_paused = false;
	app->fps = 30;
	app->frame_target_time = 1000.0f / app->fps; 
	app->delta_time = 0.0f;
	app->previous_frame_time = 0.0f;
	app->znear = 0.1f;
	app->zfar = 100.0f;
	app->aspectx = (float)app->win.width / app->win.height;
	app->aspecty = (float)app->win.height / app->win.width;
	app->fovy = M_PI / 3.0f; // 60°
	app->fovx = atanf(tanf(app->fovy / 2) * app->aspectx) * 2.0f; // approx 91°, if fovy is 60°
	app->render_method = RENDER_WIRE;
	app->cull = true;
	switch (texture_choice)
	{
		case 1: app->file_name = "cube"; break;
        case 2: app->file_name = "f22";  break;
        case 3: app->file_name = "crab"; break;
		case 4: app->file_name = "grass"; break;
        case 5: app->file_name = "drone";  break;
        case 6: app->file_name = "efa"; break;
		case 7: app->file_name = "f117"; break;
        default: app->file_name = "cube"; break;
    }
}

void get_app_info(AppState *app)
{
	printf("Desired FPS: %d\n", app->fps);
	printf("Actual FPS: %.1f\n", 1.0f / app->delta_time);
	printf("Frame target time: %.2fms\n", app->frame_target_time);
	printf("Delta time: %.4f\n", app->delta_time);
	printf("Z-Near: %.2f\n", app->znear);
	printf("Z-Far: %.2f\n", app->zfar);
	printf("Aspect Ratio: %f\n", app->aspectx);
	printf("FOV-Y: %.2f degrees\n", app->fovy * (180.0f / M_PI));
	printf("FOV-X: %.2f degrees\n", app->fovx * (180.0f / M_PI));
}