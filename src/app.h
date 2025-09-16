#pragma once

#include <stdbool.h>
#include "display.h"

typedef struct {
    bool is_running;
    bool paused;
    bool was_paused;
    int fps;
    float frame_target_time;
    float delta_time;
    float previous_frame_time;
    float znear;
    float zfar;
    float aspectx;
    float aspecty;
    float fovy;
    float fovx;
    enum Render_Method render_method;
    bool cull;
    Window win;
} AppState;

void app_init(AppState *app);
void get_app_info(AppState *app);