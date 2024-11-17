#include "../scene_manager_01_tutorial.h"
#include "scene_manager_01_tutorial_scene.h"

void scene_manager_01_tutorial_scene_main_on_enter(void* context) {
    SceneManager01Tutorial* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SceneManager01TutorialViewMain);
}

bool scene_manager_01_tutorial_scene_main_on_event(void* context, SceneManagerEvent event) {
    SceneManager01Tutorial* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_another_scene(
            app->scene_manager, SceneManager01TutorialSceneCircle);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SceneManager01TutorialEventOpenWarningScene) {
            scene_manager_next_scene(app->scene_manager, SceneManager01TutorialSceneWarning);
            consumed = true;
        } else if(event.event == SceneManager01TutorialEventOpenInfoScene) {
            scene_manager_next_scene(app->scene_manager, SceneManager01TutorialSceneDolphinInfo);
            consumed = true;
        }
    }

    return consumed;
}

void scene_manager_01_tutorial_scene_main_on_exit(void* context) {
    UNUSED(context);
} 