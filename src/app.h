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
    int texture_choice;
    char *file_name;
    Window win;
} AppState;

void app_init(AppState *app, int texture_choice);
void get_app_info(AppState *app);