#include "test_menu_film.h"
#include <furi.h>

#define TAG "TestMenuFilm"

typedef enum {
    TestMenuFilmViewSubmenuMain,
    TestMenuFilmViewSubmenuFilm,
    TestMenuFilmViewSubmenuDev,
} TestMenuFilmView;

struct TestMenuFilm {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu_main;
    Submenu* submenu_film;
    Submenu* submenu_dev;
    
    // Store selections
    char selected_film[32];
    char selected_developer[32];
};

static void test_menu_film_submenu_callback(void* context, uint32_t index) {
    TestMenuFilm* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, index);
}

static void film_select_callback(void* context, uint32_t index) {
    TestMenuFilm* app = context;
    
    // Store film selection based on index
    switch(index) {
        case 0:
            strncpy(app->selected_film, "HP5+ 400", sizeof(app->selected_film));
            break;
        case 1:
            strncpy(app->selected_film, "Tri-X 400", sizeof(app->selected_film));
            break;
        case 2:
            strncpy(app->selected_film, "FP4+ 125", sizeof(app->selected_film));
            break;
        case 3:
            strncpy(app->selected_film, "Delta 3200", sizeof(app->selected_film));
            break;
    }
    
    FURI_LOG_I(TAG, "Selected film: %s", app->selected_film);
    view_dispatcher_switch_to_view(app->view_dispatcher, TestMenuFilmViewSubmenuMain);
}

static void developer_select_callback(void* context, uint32_t index) {
    TestMenuFilm* app = context;
    
    // Store developer selection based on index
    switch(index) {
        case 0:
            strncpy(app->selected_developer, "ID-11", sizeof(app->selected_developer));
            break;
        case 1:
            strncpy(app->selected_developer, "Rodinal", sizeof(app->selected_developer));
            break;
        case 2:
            strncpy(app->selected_developer, "DD-X", sizeof(app->selected_developer));
            break;
        case 3:
            strncpy(app->selected_developer, "HC-110", sizeof(app->selected_developer));
            break;
    }
    
    FURI_LOG_I(TAG, "Selected developer: %s", app->selected_developer);
    view_dispatcher_switch_to_view(app->view_dispatcher, TestMenuFilmViewSubmenuMain);
}

static uint32_t test_menu_film_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static uint32_t test_menu_film_previous_callback(void* context) {
    UNUSED(context);
    return TestMenuFilmViewSubmenuMain;
}

TestMenuFilm* test_menu_film_alloc() {
    TestMenuFilm* app = malloc(sizeof(TestMenuFilm));

    // Initialize GUI and ViewDispatcher
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize main menu
    app->submenu_main = submenu_alloc();
    View* view = submenu_get_view(app->submenu_main);
    view_set_previous_callback(view, test_menu_film_exit_callback);
    view_dispatcher_add_view(app->view_dispatcher, TestMenuFilmViewSubmenuMain, view);
    submenu_add_item(
        app->submenu_main, "Select Film", TestMenuFilmViewSubmenuFilm, test_menu_film_submenu_callback, app);
    submenu_add_item(
        app->submenu_main,
        "Select Developer",
        TestMenuFilmViewSubmenuDev,
        test_menu_film_submenu_callback,
        app);

    // Initialize film submenu
    app->submenu_film = submenu_alloc();
    view = submenu_get_view(app->submenu_film);
    view_set_previous_callback(view, test_menu_film_previous_callback);
    view_dispatcher_add_view(app->view_dispatcher, TestMenuFilmViewSubmenuFilm, view);
    submenu_add_item(app->submenu_film, "HP5+ 400", 0, film_select_callback, app);
    submenu_add_item(app->submenu_film, "Tri-X 400", 1, film_select_callback, app);
    submenu_add_item(app->submenu_film, "FP4+ 125", 2, film_select_callback, app);
    submenu_add_item(app->submenu_film, "Delta 3200", 3, film_select_callback, app);

    // Initialize developer submenu
    app->submenu_dev = submenu_alloc();
    view = submenu_get_view(app->submenu_dev);
    view_set_previous_callback(view, test_menu_film_previous_callback);
    view_dispatcher_add_view(app->view_dispatcher, TestMenuFilmViewSubmenuDev, view);
    submenu_add_item(app->submenu_dev, "ID-11", 0, developer_select_callback, app);
    submenu_add_item(app->submenu_dev, "Rodinal", 1, developer_select_callback, app);
    submenu_add_item(app->submenu_dev, "DD-X", 2, developer_select_callback, app);
    submenu_add_item(app->submenu_dev, "HC-110", 3, developer_select_callback, app);

    return app;
}

void test_menu_film_free(TestMenuFilm* app) {
    // Remove and free views
    view_dispatcher_remove_view(app->view_dispatcher, TestMenuFilmViewSubmenuMain);
    view_dispatcher_remove_view(app->view_dispatcher, TestMenuFilmViewSubmenuFilm);
    view_dispatcher_remove_view(app->view_dispatcher, TestMenuFilmViewSubmenuDev);
    
    submenu_free(app->submenu_main);
    submenu_free(app->submenu_film);
    submenu_free(app->submenu_dev);

    // Free ViewDispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t test_menu_film_app(void* p) {
    UNUSED(p);
    TestMenuFilm* app = test_menu_film_alloc();

    // Set starting view
    view_dispatcher_switch_to_view(app->view_dispatcher, TestMenuFilmViewSubmenuMain);

    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);

    // Free resources
    test_menu_film_free(app);
    return 0;
} 