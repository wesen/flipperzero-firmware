#include "scene_manager_01_tutorial_views.h"
#include "../scene_manager_01_tutorial.h"

typedef struct {
    int x;
    int y;
} CircleViewState;

static void circle_view_draw_callback(Canvas* canvas, void* model) {
    CircleViewState* state = model;
    canvas_draw_circle(canvas, state->x, state->y, 5);
}

static bool circle_view_input_callback(InputEvent* event, void* context) {
    SceneManager01Tutorial* app = context;
    bool handled = false;

    if(event->type == InputTypeShort || event->type == InputTypeLong) {
        with_view_model(
            app->circle_view,
            CircleViewState * model,
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
                default:
                    break;
                }
            },
            handled);
    }

    return handled;
}

View* scene_manager_01_tutorial_circle_view_alloc(SceneManager01Tutorial* app) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLocking, sizeof(CircleViewState));
    with_view_model(
        view,
        CircleViewState * model,
        {
            model->x = 64;
            model->y = 32;
        },
        true);
    view_set_context(view, app);
    view_set_draw_callback(view, circle_view_draw_callback);
    view_set_input_callback(view, circle_view_input_callback);
    return view;
}

void scene_manager_01_tutorial_circle_view_free(View* view) {
    view_free_model(view);
    view_free(view);
} 