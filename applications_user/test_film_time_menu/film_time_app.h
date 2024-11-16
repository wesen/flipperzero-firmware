#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>

typedef struct {
    float base_time;
    int8_t push_pull;
    float scale_percent;
    float final_time;
} FilmStep;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* main_menu;
    VariableItemList* step_menu;
    
    FilmStep steps[5];  // Developer, Stop, Fix, Wash 1, Final Wash
    uint8_t selected_step;
    
    View* current_view;
} FilmTimeApp;

typedef enum {
    FilmTimeViewMainMenu,
    FilmTimeViewStepMenu,
} FilmTimeView;

FilmTimeApp* film_time_app_alloc();
void film_time_app_free(FilmTimeApp* app);
int32_t film_time_app(void* p);
