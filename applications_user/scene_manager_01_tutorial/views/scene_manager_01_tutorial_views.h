#pragma once

#include <gui/view.h>

typedef struct SceneManager01Tutorial SceneManager01Tutorial;

View* scene_manager_01_tutorial_main_view_alloc(SceneManager01Tutorial* app);
void scene_manager_01_tutorial_main_view_free(View* view);

View* scene_manager_01_tutorial_circle_view_alloc(SceneManager01Tutorial* app);
void scene_manager_01_tutorial_circle_view_free(View* view); 