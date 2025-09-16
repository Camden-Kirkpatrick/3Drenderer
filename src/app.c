#include "app.h"
#include <math.h>

void app_init(AppState *app)
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
	app->fovy = M_PI / 3.0f; // 60deg
	app->fovx = atanf(tanf(app->fovy / 2) * app->aspectx) * 2.0f; // approx 91deg, if fovy = 60deg
	app->render_method = RENDER_WIRE;
	app->cull = true;
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
	printf("FOV-Y: %.2f\n", app->fovy * (180.0f / M_PI));
	printf("FOV-X: %.2f\n", app->fovx * (180.0f / M_PI));
}