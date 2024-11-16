#include "../timer_app_i.h"

static void timer_scene_main_ok_callback(void* context) {
    TimerApp* app = context;
    app->timer_running = !app->timer_running;
    timer_view_set_running(app->timer_view, app->timer_running);
}

static void timer_scene_main_left_callback(void* context) {
    TimerApp* app = context;
    if(!app->timer_running) {
        app->remaining_time = app->timer_duration;
        timer_view_update_timer(app->timer_view, app->remaining_time);
    }
}

static void timer_scene_main_right_callback(void* context) {
    TimerApp* app = context;
    if(!app->timer_running) {
        scene_manager_next_scene(app->scene_manager, TimerSceneSettings);
    }
}

void timer_scene_main_on_enter(void* context) {
    TimerApp* app = context;
    timer_view_set_left_callback(app->timer_view, timer_scene_main_left_callback, app);
    timer_view_set_right_callback(app->timer_view, timer_scene_main_right_callback, app);
    timer_view_set_ok_callback(app->timer_view, timer_scene_main_ok_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, TimerViewMain);
}

bool timer_scene_main_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void timer_scene_main_on_exit(void* context) {
    TimerApp* app = context;
    timer_view_set_left_callback(app->timer_view, NULL, NULL);
    timer_view_set_right_callback(app->timer_view, NULL, NULL);
    timer_view_set_ok_callback(app->timer_view, NULL, NULL);
}
