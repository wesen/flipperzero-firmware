#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/number_input.h>

typedef struct {
    float base_time;
    int8_t push_pull;
    float scale_percent;
    float final_time;
    uint8_t step_index;
    char display_name[32]; // For showing time in menu
} FilmStep;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* main_menu;
    VariableItemList* step_menu;
    NumberInput* time_input;

    FilmStep steps[5]; // Developer, Stop, Fix, Wash 1, Final Wash
    uint8_t selected_step;

    View* current_view;
} FilmTimeApp;

typedef enum {
    FilmTimeViewMainMenu,
    FilmTimeViewStepMenu,
    FilmTimeViewTimeInput,
} FilmTimeView;

FilmTimeApp* film_time_app_alloc(void);
void film_time_app_free(FilmTimeApp* app);
int32_t film_time_app(FilmTimeApp* app);
