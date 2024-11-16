#include "film_time_app.h"
#include <furi.h>

static const char* step_names[] = {
    "Developer",
    "Stop Bath",
    "Fixer",
    "Wash 1",
    "Final Wash"
};

static const float default_times[] = {
    6.5f,  // Developer (ID-11 stock, ISO 100)
    1.0f,  // Stop
    5.0f,  // Fix
    2.0f,  // Wash 1
    5.0f   // Final Wash
};

static void film_time_calculate_final_time(FilmStep* step) {
    float push_factor = 1.0f + (step->push_pull * 0.15f);  // 15% per stop
    float scale_factor = step->scale_percent / 100.0f;
    step->final_time = step->base_time * push_factor * scale_factor;
}

static void push_pull_changed(VariableItem* item) {
    FilmTimeApp* app = variable_item_get_context(item);
    FilmStep* step = &app->steps[app->selected_step];
    
    int8_t value_index = variable_item_get_current_value_index(item);
    step->push_pull = value_index - 3;  // Convert 0-6 to -3 to +3
    
    char str[8];
    if(step->push_pull > 0) {
        snprintf(str, sizeof(str), "+%d", step->push_pull);
    } else {
        snprintf(str, sizeof(str), "%d", step->push_pull);
    }
    variable_item_set_current_value_text(item, str);
    
    film_time_calculate_final_time(step);
}

static void scale_changed(VariableItem* item) {
    FilmTimeApp* app = variable_item_get_context(item);
    FilmStep* step = &app->steps[app->selected_step];
    
    uint8_t value_index = variable_item_get_current_value_index(item);
    step->scale_percent = 50.0f + value_index;  // 50% to 150%
    
    char str[8];
    snprintf(str, sizeof(str), "%d%%", (int)step->scale_percent);
    variable_item_set_current_value_text(item, str);
    
    film_time_calculate_final_time(step);
}

static void step_menu_callback(void* context, uint32_t index) {
    FilmTimeApp* app = context;
    app->selected_step = index;
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmTimeViewStepMenu);
}

static void back_event_callback(void* context) {
    FilmTimeApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmTimeViewMainMenu);
}

static uint32_t step_menu_exit_callback(void* context) {
    UNUSED(context);
    return FilmTimeViewMainMenu;
}

static uint32_t main_menu_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

FilmTimeApp* film_time_app_alloc() {
    FilmTimeApp* app = malloc(sizeof(FilmTimeApp));
    
    // GUI
    app->gui = furi_record_open(RECORD_GUI);
    
    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Main Menu
    app->main_menu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FilmTimeViewMainMenu, submenu_get_view(app->main_menu));
    view_set_previous_callback(submenu_get_view(app->main_menu), main_menu_exit_callback);
    
    // Add steps to main menu
    for(uint8_t i = 0; i < COUNT_OF(step_names); i++) {
        submenu_add_item(app->main_menu, step_names[i], i, step_menu_callback, app);
        app->steps[i].base_time = default_times[i];
        app->steps[i].push_pull = 0;
        app->steps[i].scale_percent = 100.0f;
        film_time_calculate_final_time(&app->steps[i]);
    }
    
    // Step Menu
    app->step_menu = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FilmTimeViewStepMenu, variable_item_list_get_view(app->step_menu));
    view_set_previous_callback(variable_item_list_get_view(app->step_menu), step_menu_exit_callback);
    variable_item_list_set_enter_callback(app->step_menu, back_event_callback, app);
    
    // Add push/pull control
    VariableItem* item = variable_item_list_add(
        app->step_menu, "Push/Pull", 7, push_pull_changed, app);
    variable_item_set_current_value_index(item, 3);  // Default to 0
    variable_item_set_current_value_text(item, "0");
    
    // Add scale control
    item = variable_item_list_add(
        app->step_menu, "Scale", 101, scale_changed, app);  // 50% to 150%
    variable_item_set_current_value_index(item, 50);  // Default to 100%
    variable_item_set_current_value_text(item, "100%");
    
    return app;
}

void film_time_app_free(FilmTimeApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, FilmTimeViewMainMenu);
    view_dispatcher_remove_view(app->view_dispatcher, FilmTimeViewStepMenu);
    
    submenu_free(app->main_menu);
    variable_item_list_free(app->step_menu);
    view_dispatcher_free(app->view_dispatcher);
    
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t film_time_app(void* p) {
    UNUSED(p);
    FilmTimeApp* app = film_time_app_alloc();
    
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmTimeViewMainMenu);
    view_dispatcher_run(app->view_dispatcher);
    
    film_time_app_free(app);
    return 0;
}
