#include "../timer_app_i.h"

enum TimerSubmenuIndex {
    TimerSubmenuIndexTimer,
    TimerSubmenuIndexSettings,
    TimerSubmenuIndexAbout,
};

void timer_scene_menu_submenu_callback(void* context, uint32_t index) {
    TimerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void timer_scene_menu_on_enter(void* context) {
    TimerApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "Timer", TimerSubmenuIndexTimer, timer_scene_menu_submenu_callback, app);
    submenu_add_item(
        submenu, "Settings", TimerSubmenuIndexSettings, timer_scene_menu_submenu_callback, app);
    submenu_add_item(
        submenu, "About", TimerSubmenuIndexAbout, timer_scene_menu_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, TimerSceneMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, TimerViewSubmenu);
}

bool timer_scene_menu_on_event(void* context, SceneManagerEvent event) {
    TimerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, TimerSceneMenu, event.event);
        switch(event.event) {
        case TimerSubmenuIndexTimer:
            scene_manager_next_scene(app->scene_manager, TimerSceneMain);
            consumed = true;
            break;
        case TimerSubmenuIndexSettings:
            scene_manager_next_scene(app->scene_manager, TimerSceneSettings);
            consumed = true;
            break;
        case TimerSubmenuIndexAbout:
            // TODO: Implement About scene if needed
            consumed = true;
            break;
        }
    }

    return consumed;
}

void timer_scene_menu_on_exit(void* context) {
    TimerApp* app = context;
    submenu_reset(app->submenu);
} 