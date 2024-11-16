#include "../timer_app_i.h"

enum TimerSettingsIndex {
    TimerSettingsIndexDuration,
    TimerSettingsIndexSpeedFactor,
};

static void timer_scene_settings_duration_changed(VariableItem* item) {
    TimerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    const uint32_t durations[] = {30, 60, 120, 300, 600, 1800, 3600};
    app->timer_duration = durations[index];
    app->remaining_time = app->timer_duration;
    char str[16];
    snprintf(str, sizeof(str), "%lu:%02lu", app->timer_duration / 60, app->timer_duration % 60);
    variable_item_set_current_value_text(item, str);
}

static void timer_scene_settings_speed_changed(VariableItem* item) {
    TimerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    const float speeds[] = {0.5f, 1.0f, 2.0f, 4.0f};
    app->speed_factor = speeds[index];
    char str[16];
    snprintf(str, sizeof(str), "%.1f", (double)app->speed_factor);
    variable_item_set_current_value_text(item, str);
}

void timer_scene_settings_on_enter(void* context) {
    TimerApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;
    VariableItem* item;

    // Duration setting
    item = variable_item_list_add(
        variable_item_list, "Duration", 7, timer_scene_settings_duration_changed, app);
    const uint32_t durations[] = {30, 60, 120, 300, 600, 1800, 3600};
    uint8_t duration_index = 0;
    for(uint8_t i = 0; i < COUNT_OF(durations); i++) {
        if(app->timer_duration == durations[i]) {
            duration_index = i;
            break;
        }
    }
    char duration_str[16];
    snprintf(
        duration_str,
        sizeof(duration_str),
        "%lu:%02lu",
        app->timer_duration / 60,
        app->timer_duration % 60);
    variable_item_set_current_value_index(item, duration_index);
    variable_item_set_current_value_text(item, duration_str);

    // Speed factor setting
    item = variable_item_list_add(
        variable_item_list, "Speed", 4, timer_scene_settings_speed_changed, app);
    const float speeds[] = {0.5f, 1.0f, 2.0f, 4.0f};
    uint8_t speed_index = 0;
    for(uint8_t i = 0; i < COUNT_OF(speeds); i++) {
        if(app->speed_factor == speeds[i]) {
            speed_index = i;
            break;
        }
    }
    char speed_str[16];
    snprintf(speed_str, sizeof(speed_str), "%.1f", (double)app->speed_factor);
    variable_item_set_current_value_index(item, speed_index);
    variable_item_set_current_value_text(item, speed_str);

    view_dispatcher_switch_to_view(app->view_dispatcher, TimerViewVariableItemList);
}

bool timer_scene_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void timer_scene_settings_on_exit(void* context) {
    TimerApp* app = context;
    variable_item_list_reset(app->variable_item_list);
}
