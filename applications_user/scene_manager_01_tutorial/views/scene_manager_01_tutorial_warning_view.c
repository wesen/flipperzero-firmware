#include "scene_manager_01_tutorial_views.h"
#include "../scene_manager_01_tutorial.h"

typedef struct {
    bool acknowledged;
} WarningViewState;

static void warning_view_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    // Draw warning box
    canvas_draw_rframe(canvas, 14, 8, 100, 48, 4);
    canvas_draw_rframe(canvas, 16, 10, 96, 44, 3);

    // Draw warning text
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, "Warning!");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "This is a warning message");
    canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, "Press Back to close");
}

static bool warning_view_input_callback(InputEvent* event, void* context) {
    UNUSED(context);
    bool handled = false;

    // if(event->type == InputTypeShort) {
    //     handled = true;
    // }
    UNUSED(event);

    return handled;
}

View* scene_manager_01_tutorial_warning_view_alloc(SceneManager01Tutorial* app) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLocking, sizeof(WarningViewState));
    with_view_model(view, WarningViewState * model, { model->acknowledged = false; }, true);
    view_set_context(view, app);
    view_set_draw_callback(view, warning_view_draw_callback);
    view_set_input_callback(view, warning_view_input_callback);
    return view;
}

void scene_manager_01_tutorial_warning_view_free(View* view) {
    view_free_model(view);
    view_free(view);
}
