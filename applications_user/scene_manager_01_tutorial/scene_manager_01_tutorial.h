#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

typedef struct SceneManager01Tutorial {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    View* main_view;
    View* circle_view;
    View* warning_view;
    Widget* widget;
} SceneManager01Tutorial;

typedef enum {
    SceneManager01TutorialViewMain,
    SceneManager01TutorialViewCircle,
    SceneManager01TutorialViewWarning,
    SceneManager01TutorialViewWidget,
    SceneManager01TutorialViewCount,
} SceneManager01TutorialView; 