#include "../can_lin_tools_app_i.h"
#include "../views/can_lin_tools_view_can_hacker2.h"

void can_lin_tools_scene_can_hacker2_callback(CanLinToolsCustomEvent event, void* context) {
    furi_assert(context);
    CanLinToolsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static void can_lin_tools_scene_can_hacker2_update(void* context) {
    furi_assert(context);
    CanLinToolsApp* app = context;
    UNUSED(app);
}

void can_lin_tools_scene_can_hacker2_on_enter(void* context) {
    furi_assert(context);
    CanLinToolsApp* app = context;
    UNUSED(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, CanLinToolsViewCanHacker2);
}

bool can_lin_tools_scene_can_hacker2_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    CanLinToolsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        can_lin_tools_scene_can_hacker2_update(app);
    }

    return consumed;
}

void can_lin_tools_scene_can_hacker2_on_exit(void* context) {
    furi_assert(context);
    CanLinToolsApp* app = context;
    UNUSED(app);
}