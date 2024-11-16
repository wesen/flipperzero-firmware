#include "timer_scene.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const timer_on_enter_handlers[])(void*) = {
#include "timer_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const timer_on_event_handlers[])(void*, SceneManagerEvent) = {
#include "timer_scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const timer_on_exit_handlers[])(void*) = {
#include "timer_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers timer_scene_handlers = {
    .on_enter_handlers = timer_on_enter_handlers,
    .on_event_handlers = timer_on_event_handlers,
    .on_exit_handlers = timer_on_exit_handlers,
    .scene_num = TimerSceneNum,
};
