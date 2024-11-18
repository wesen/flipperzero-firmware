#include "../scene_manager_01_tutorial.h"
#include "applications_user/scene_manager_01_tutorial/elements/progress_bar.h"
#include "scene_manager_01_tutorial_scene.h"
#include <gui/elements.h>

void scene_manager_01_tutorial_scene_dolphin_info_on_enter(void* context) {
    SceneManager01Tutorial* app = context;

    // Reset widget to clear previous content
    widget_reset(app->widget);

    // Add title
    widget_add_text_box_element(
        app->widget, 0, 0, 128, 12, AlignCenter, AlignCenter, "\e#\e!Dolphin Treatment\e!\n", false);

    // Add scrollable content
    widget_add_text_scroll_element(
        app->widget,
        0,
        12,
        128,
        52,
        "Dolphin ketamine treatment methodology involves careful administration "
        "of controlled substances in a marine environment. This controversial "
        "approach requires precise dosing and monitoring. The procedure must "
        "be conducted by qualified veterinary professionals in appropriate "
        "facilities.\n\n"
        "Note: This is for educational purposes only. Always consult marine "
        "biology experts for actual dolphin care.");

    // WidgetElement* progress_bar = widget_element_progress_bar_create(10, 10, 80, 20, 23);
    // widget_add_element(app->widget, progress_bar);

    view_dispatcher_switch_to_view(app->view_dispatcher, SceneManager01TutorialViewWidget);
}

bool scene_manager_01_tutorial_scene_dolphin_info_on_event(void* context, SceneManagerEvent event) {
    SceneManager01Tutorial* app = context;
    bool consumed = false;

    // if(event.type == SceneManagerEventTypeBack) {
    //     scene_manager_previous_scene(app->scene_manager);
    //     consumed = true;
    // }
    UNUSED(event);
    UNUSED(app);

    return consumed;
}

void scene_manager_01_tutorial_scene_dolphin_info_on_exit(void* context) {
    SceneManager01Tutorial* app = context;
    widget_reset(app->widget);
}
