#include "scene_manager_01_tutorial_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const scene_manager_01_tutorial_on_enter_handlers[])(void*) = {
#include "scene_manager_01_tutorial_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const scene_manager_01_tutorial_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "scene_manager_01_tutorial_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const scene_manager_01_tutorial_on_exit_handlers[])(void* context) = {
#include "scene_manager_01_tutorial_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers scene_manager_01_tutorial_scene_handlers = {
    .on_enter_handlers = scene_manager_01_tutorial_on_enter_handlers,
    .on_event_handlers = scene_manager_01_tutorial_on_event_handlers,
    .on_exit_handlers = scene_manager_01_tutorial_on_exit_handlers,
    .scene_num = SceneManager01TutorialSceneNum,
}; 