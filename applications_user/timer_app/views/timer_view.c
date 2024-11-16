#include "timer_view.h"
#include <gui/elements.h>
#include <furi.h>

struct TimerView {
    View* view;
    TimerViewCallback left_callback;
    TimerViewCallback right_callback;
    TimerViewCallback ok_callback;
    TimerViewCallback back_callback;
    void* left_context;
    void* right_context;
    void* ok_context;
    void* back_context;
};

typedef struct {
    uint32_t remaining_time;
    bool timer_running;
} TimerViewModel;

static void timer_view_draw_callback(Canvas* canvas, void* _model) {
    TimerViewModel* model = _model;
    
    // Draw time in center
    canvas_set_font(canvas, FontBigNumbers);
    uint32_t minutes = model->remaining_time / 60;
    uint32_t seconds = model->remaining_time % 60;
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02lu:%02lu", minutes, seconds);
    
    canvas_draw_str_aligned(
        canvas, 64, 24, AlignCenter, AlignCenter, time_str);

    // Draw status and controls
    canvas_set_font(canvas, FontSecondary);
    if(model->timer_running) {
        canvas_draw_str_aligned(canvas, 64, 12, AlignCenter, AlignCenter, "Running");
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter, "Press OK to pause");
    } else {
        canvas_draw_str_aligned(canvas, 64, 12, AlignCenter, AlignCenter, "Paused");
        canvas_draw_str_aligned(canvas, 10, 50, AlignLeft, AlignBottom, "Restart");
        canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignBottom, "Continue");
        canvas_draw_str_aligned(canvas, 118, 50, AlignRight, AlignBottom, "Settings");
    }
}

static bool timer_view_input_callback(InputEvent* event, void* context) {
    TimerView* timer_view = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyLeft:
            if(timer_view->left_callback) {
                timer_view->left_callback(timer_view->left_context);
                consumed = true;
            }
            break;
        case InputKeyRight:
            if(timer_view->right_callback) {
                timer_view->right_callback(timer_view->right_context);
                consumed = true;
            }
            break;
        case InputKeyOk:
            if(timer_view->ok_callback) {
                timer_view->ok_callback(timer_view->ok_context);
                consumed = true;
            }
            break;
        case InputKeyBack:
            if(timer_view->back_callback) {
                timer_view->back_callback(timer_view->back_context);
                consumed = true;
            }
            break;
        default:
            break;
        }
    }

    return consumed;
}

TimerView* timer_view_alloc() {
    TimerView* timer_view = malloc(sizeof(TimerView));
    timer_view->view = view_alloc();
    view_set_context(timer_view->view, timer_view);
    view_allocate_model(timer_view->view, ViewModelTypeLocking, sizeof(TimerViewModel));
    view_set_draw_callback(timer_view->view, timer_view_draw_callback);
    view_set_input_callback(timer_view->view, timer_view_input_callback);
    
    with_view_model(
        timer_view->view,
        TimerViewModel * model,
        {
            model->remaining_time = 0;
            model->timer_running = false;
        },
        true);
    
    return timer_view;
}

void timer_view_free(TimerView* timer_view) {
    view_free(timer_view->view);
    free(timer_view);
}

View* timer_view_get_view(TimerView* timer_view) {
    return timer_view->view;
}

void timer_view_set_left_callback(TimerView* timer_view, TimerViewCallback callback, void* context) {
    timer_view->left_callback = callback;
    timer_view->left_context = context;
}

void timer_view_set_right_callback(TimerView* timer_view, TimerViewCallback callback, void* context) {
    timer_view->right_callback = callback;
    timer_view->right_context = context;
}

void timer_view_set_ok_callback(TimerView* timer_view, TimerViewCallback callback, void* context) {
    timer_view->ok_callback = callback;
    timer_view->ok_context = context;
}

void timer_view_set_back_callback(TimerView* timer_view, TimerViewCallback callback, void* context) {
    timer_view->back_callback = callback;
    timer_view->back_context = context;
}

void timer_view_update_timer(TimerView* timer_view, uint32_t remaining_time) {
    with_view_model(
        timer_view->view,
        TimerViewModel * model,
        {
            model->remaining_time = remaining_time;
        },
        true);
}

void timer_view_set_running(TimerView* timer_view, bool running) {
    with_view_model(
        timer_view->view,
        TimerViewModel * model,
        {
            model->timer_running = running;
        },
        true);
}
