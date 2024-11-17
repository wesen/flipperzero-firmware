#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) SceneManager01TutorialScene##id,
typedef enum {
#include "scene_manager_01_tutorial_scene_config.h"
    SceneManager01TutorialSceneNum,
} SceneManager01TutorialScene;
#undef ADD_SCENE

extern const SceneManagerHandlers scene_manager_01_tutorial_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "scene_manager_01_tutorial_scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "scene_manager_01_tutorial_scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "scene_manager_01_tutorial_scene_config.h"
#undef ADD_SCENE 