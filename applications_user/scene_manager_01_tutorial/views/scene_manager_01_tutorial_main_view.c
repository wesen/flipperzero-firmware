#include "applications_user/scene_manager_01_tutorial/scenes/scene_manager_01_tutorial_scene.h"
#include "scene_manager_01_tutorial_views.h"
#include "../scene_manager_01_tutorial.h"

typedef struct {
    int x;
    int y;
} ViewState;

static void view_draw_callback(Canvas* canvas, void* model) {
    ViewState* state = model;
    canvas_draw_str(canvas, state->x, state->y, "Hello Flipper!");
    canvas_draw_box(canvas, 10, 10, 10, 10);
    canvas_draw_rbox(canvas, 40, 30, 10, 10, 5);
    canvas_draw_frame(canvas, 70, 20, 20, 20);
}

static bool view_input_callback(InputEvent* event, void* context) {
    SceneManager01Tutorial* app = context;
    bool handled = false;

    if(event->key == InputKeyOk) {
        if(event->type == InputTypeShort) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, SceneManager01TutorialEventOpenWarningScene);
            return true;
        } else if(event->type == InputTypeLong) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, SceneManager01TutorialEventOpenInfoScene);
            return true;
        }
    }

    if(event->type == InputTypeShort || event->type == InputTypeLong) {
        with_view_model(
            app->main_view,
            ViewState * model,
            {
                int step = (event->type == InputTypeShort) ? 2 : 10;
                switch(event->key) {
                case InputKeyUp:
                    model->y -= step;
                    handled = true;
                    break;
                case InputKeyDown:
                    model->y += step;
                    handled = true;
                    break;
                case InputKeyRight:
                    model->x += step;
                    handled = true;
                    break;
                case InputKeyLeft:
                    model->x -= step;
                    handled = true;
                    break;
                case InputKeyOk:
                    // Send custom event to show warning
                    view_dispatcher_send_custom_event(app->view_dispatcher, InputKeyOk);
                    handled = true;
                    break;
                default:
                    break;
                }
            },
            handled);
    }

    return handled;
}

View* scene_manager_01_tutorial_main_view_alloc(SceneManager01Tutorial* app) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLocking, sizeof(ViewState));
    with_view_model(
        view,
        ViewState * model,
        {
            model->x = 10;
            model->y = 30;
        },
        true);
    view_set_context(view, app);
    view_set_draw_callback(view, view_draw_callback);
    view_set_input_callback(view, view_input_callback);

    return view;
}

void scene_manager_01_tutorial_main_view_free(View* view) {
    view_free_model(view);
    view_free(view);
}
