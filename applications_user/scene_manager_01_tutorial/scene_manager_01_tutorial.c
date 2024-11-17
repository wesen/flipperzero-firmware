#include "gui/gui.h"
#include "gui/view_dispatcher.h"
#include <furi.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* main_view;
} SceneManager01Tutorial;

typedef struct {
    int x;
    int y;
} ViewState;

static void view_draw_callback(Canvas* canvas, void* context) {
    ViewState* state = context;
    canvas_draw_str(canvas, state->x, state->y, "Hello Flipper!");
}

int32_t scene_manager_01_tutorial(void*) {
    SceneManager01Tutorial* app = malloc(sizeof(SceneManager01Tutorial));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->main_view = view_alloc();

    view_allocate_model(app->main_view, ViewModelTypeLocking, sizeof(ViewState));
    with_view_model(
        app->main_view,
        ViewState * model,
        {
            model->x = 10;
            model->y = 30;
        },
        true);
    view_set_context(app->main_view, app);
    view_set_draw_callback(app->main_view, view_draw_callback);

    view_dispatcher_add_view(app->view_dispatcher, 0, app->main_view);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_run(app->view_dispatcher);

    // cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    view_free_model(app->main_view);
    view_free(app->main_view);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
    return 0;
}
