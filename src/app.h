#pragma once

#include <stdbool.h>
#include "display.h"

typedef struct {
    bool is_running;
    bool paused;
    bool was_paused;
    int fps;
    int frame_target_time;
    float delta_time;
    float previous_frame_time;
    int arrow_key_mode;
    enum Render_Method render_method;
    bool cull;
    Window win;
} AppState;

void app_init(AppState *app);