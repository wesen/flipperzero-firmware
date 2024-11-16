#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/variable_item_list.h>
#include <notification/notification.h>
#include "views/timer_view.h"
#include "scenes/timer_scene.h"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    VariableItemList* variable_item_list;
    Submenu* submenu;
    Popup* popup;
    TimerView* timer_view;
    NotificationApp* notifications;

    // Timer settings
    uint32_t timer_duration; // in seconds
    float speed_factor;
    bool timer_running;
    uint32_t remaining_time;
} TimerApp;

typedef enum {
    TimerViewMain,
    TimerViewSubmenu,
    TimerViewPopup,
    TimerViewVariableItemList,
} TimerViewId;
