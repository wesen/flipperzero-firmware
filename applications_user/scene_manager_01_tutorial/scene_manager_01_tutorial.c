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

static void view_draw_callback(Canvas* canvas, void* model) {
    ViewState* state = model;
    canvas_draw_str(canvas, state->x, state->y, "Hello Flipper!");
    // canvas_draw_str_aligned(canvas, 10, 10, AlignLeft, AlignTop, "Aligned Flipper!");
    // canvas_draw_str_aligned(canvas, 50, 20, AlignRight, AlignBottom, "Bottom Flipper!");
    canvas_draw_box(canvas, 10, 10, 10, 10);
    canvas_draw_rbox(canvas, 40, 30, 10, 10, 5);
    canvas_draw_frame(canvas, 70, 20, 20, 20);
}

static bool view_input_callback(InputEvent* event, void* context) {
    SceneManager01Tutorial* app = context;
    bool handled = false;

    if(event->type == InputTypeShort) {
        with_view_model(
            app->main_view,
            ViewState * model,
            {
                switch(event->key) {
                case InputKeyUp:
                    model->y -= 2;
                    handled = true;
                    break;
                case InputKeyDown:
                    model->y += 2;
                    handled = true;
                    break;
                case InputKeyRight:
                    model->x += 2;
                    handled = true;
                    break;
                case InputKeyLeft:
                    model->x -= 2;
                    handled = true;
                    break;
                case InputKeyOk:
                case InputKeyBack:
                    break;
                case InputKeyMAX:
                    break;
                }
            },
            handled);
    } else if (event->type == InputTypeLong) {
        with_view_model(
            app->main_view,
            ViewState * model,
            {
                switch(event->key) {
                case InputKeyUp:
                    model->y -= 10;
                    handled = true;
                    break;
                case InputKeyDown:
                    model->y += 10;
                    handled = true;
                    break;
                case InputKeyRight:
                    model->x += 10;
                    handled = true;
                    break;
                case InputKeyLeft:
                    model->x -= 10;
                    handled = true;
                    break;
                case InputKeyOk:
                case InputKeyBack:
                    break;
                case InputKeyMAX:
                    break;
                }
            },
            handled);
    }

    return handled;
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
    view_set_input_callback(app->main_view, view_input_callback);

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
