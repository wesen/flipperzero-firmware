#include "timer_app_i.h"
#include <furi.h>
#include <furi_hal.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

static bool timer_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    TimerApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool timer_app_back_event_callback(void* context) {
    furi_assert(context);
    TimerApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void timer_app_tick_event_callback(void* context) {
    furi_assert(context);
    TimerApp* app = context;
    
    if(app->timer_running && app->remaining_time > 0) {
        app->remaining_time--;
        timer_view_update_timer(app->timer_view, app->remaining_time);
        
        if(app->remaining_time == 0) {
            notification_message(app->notifications, &sequence_success);
            app->timer_running = false;
            timer_view_set_running(app->timer_view, false);
        }
    }
}

TimerApp* timer_app_alloc() {
    TimerApp* app = malloc(sizeof(TimerApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&timer_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, timer_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, timer_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, timer_app_tick_event_callback, 1000);

    // Allocate views
    app->timer_view = timer_view_alloc();
    app->variable_item_list = variable_item_list_alloc();
    app->submenu = submenu_alloc();
    app->popup = popup_alloc();

    // Add views to the dispatcher
    view_dispatcher_add_view(app->view_dispatcher, TimerViewMain, timer_view_get_view(app->timer_view));
    view_dispatcher_add_view(
        app->view_dispatcher, TimerViewVariableItemList, variable_item_list_get_view(app->variable_item_list));
    view_dispatcher_add_view(app->view_dispatcher, TimerViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->view_dispatcher, TimerViewPopup, popup_get_view(app->popup));

    // Default settings
    app->timer_duration = 60;  // 1 minute default
    app->speed_factor = 1.0f;
    app->timer_running = false;
    app->remaining_time = app->timer_duration;

    return app;
}

void timer_app_free(TimerApp* app) {
    furi_assert(app);

    // Remove views from dispatcher
    view_dispatcher_remove_view(app->view_dispatcher, TimerViewMain);
    view_dispatcher_remove_view(app->view_dispatcher, TimerViewVariableItemList);
    view_dispatcher_remove_view(app->view_dispatcher, TimerViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, TimerViewPopup);

    // Free views
    timer_view_free(app->timer_view);
    variable_item_list_free(app->variable_item_list);
    submenu_free(app->submenu);
    popup_free(app->popup);

    // Free scene manager and view dispatcher
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
}

int32_t timer_app(void* p) {
    UNUSED(p);
    TimerApp* timer_app = timer_app_alloc();

    view_dispatcher_attach_to_gui(
        timer_app->view_dispatcher, timer_app->gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(timer_app->scene_manager, TimerSceneMenu);
    view_dispatcher_run(timer_app->view_dispatcher);

    timer_app_free(timer_app);
    return 0;
}
