#include "../scene_manager_01_tutorial.h"
#include "scene_manager_01_tutorial_scene.h"

void scene_manager_01_tutorial_scene_circle_on_enter(void* context) {
    SceneManager01Tutorial* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SceneManager01TutorialViewCircle);
}

bool scene_manager_01_tutorial_scene_circle_on_event(void* context, SceneManagerEvent event) {
    SceneManager01Tutorial* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(app->scene_manager, SceneManager01TutorialSceneMain);
        consumed = true;
    }

    return consumed;
}

void scene_manager_01_tutorial_scene_circle_on_exit(void* context) {
    UNUSED(context);
} 