#include "app.h"

void app_init(AppState *app)
{
    app->paused = false,
	app->was_paused = false;
	app->fps = 60;
	app->frame_target_time = 1000 / app->fps; // time between each frame
	app->delta_time = 0.0f;
	app->previous_frame_time = 0.0f;
    app->arrow_key_mode = 0; // toggle between translating and rotating the mesh
	app->render_method = RENDER_WIRE;
	app->cull = true;
}