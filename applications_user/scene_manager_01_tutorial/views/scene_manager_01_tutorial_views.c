#include "scene_manager_01_tutorial_views.h"
#include "../scene_manager_01_tutorial.h"

typedef struct {
    int x;
    int y;
} ViewState;

typedef struct {
    int x;
    int y;
} CircleViewState;

typedef struct {
    bool acknowledged;
} WarningViewState;

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

static void warning_view_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    // Draw warning box
    canvas_draw_rframe(canvas, 14, 8, 100, 48, 4);
    canvas_draw_rframe(canvas, 16, 10, 96, 44, 3);
    
    // Draw warning text
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, "Warning!");
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas, 64, 32, AlignCenter, AlignCenter, "This is a warning message");
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, "Press Back to close");
}

static bool warning_view_input_callback(InputEvent* event, void* context) {
    UNUSED(context);
    bool handled = false;

    if(event->type == InputTypeShort) {
        handled = true;
    }

    return handled;
}

View* scene_manager_01_tutorial_warning_view_alloc(SceneManager01Tutorial* app) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLocking, sizeof(WarningViewState));
    with_view_model(
        view,
        WarningViewState * model,
        { model->acknowledged = false; },
        true);
    view_set_context(view, app);
    view_set_draw_callback(view, warning_view_draw_callback);
    view_set_input_callback(view, warning_view_input_callback);
    return view;
}

void scene_manager_01_tutorial_warning_view_free(View* view) {
    view_free_model(view);
    view_free(view);
} 