#pragma once

#include <stdbool.h>
#include "display.h"

typedef struct AppState {
    bool is_running;
    bool paused;
    bool was_paused;
    int fps;
    float frame_target_time;
    float delta_time;
    float previous_frame_time;
    float znear;    // Near Clipping Plane
    float zfar;     // Far Clipping Plane
    float aspectx;  // (width / height)
    float aspecty;  // (height / width)
    float fovy;     // Vertical Field of View
    float fovx;     // Horizontal Field of View
    enum Render_Method render_method;
    bool cull;
    bool lighting;
    Window win;
} AppState;

void app_init(AppState *app);
void get_app_info(AppState *app);