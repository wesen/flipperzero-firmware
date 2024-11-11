#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

typedef struct {
    // View current state
    int counter;
    bool button_pressed;
    // For thread safety
    FuriMutex* mutex;
} GuiDemoState;

typedef struct {
    GuiDemoState* state;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Gui* gui;
} GuiDemo;

// Screen is 128x64 px

static void draw_callback(Canvas* canvas, void* ctx) {
    GuiDemoState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);

    // Clear screen
    canvas_clear(canvas);

    // Draw greeting
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 12, "My First GUI!");

    // Draw counter value
    canvas_set_font(canvas, FontSecondary);
    char counter_str[32];
    snprintf(counter_str, sizeof(counter_str), "Counter: %d", state->counter);
    canvas_draw_str(canvas, 5, 30, counter_str);

    // Draw button state
    if(state->button_pressed) {
        canvas_draw_box(canvas, 50, 40, 28, 15);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str(canvas, 54, 50, "OK!");
    } else {
        canvas_draw_frame(canvas, 50, 40, 28, 15);
        canvas_draw_str(canvas, 54, 50, "OK!");
    }

    // Draw help
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 60, "Press OK to count");

    furi_mutex_release(state->mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void cleanup_callback(void* ctx) {
    GuiDemo* gui_demo = ctx;
    view_port_enabled_set(gui_demo->view_port, false);
    gui_remove_view_port(gui_demo->gui, gui_demo->view_port);
    furi_mutex_free(gui_demo->state->mutex);
    free(gui_demo->state);
    view_port_free(gui_demo->view_port);
    furi_message_queue_free(gui_demo->event_queue);
    free(gui_demo);
}

int32_t my_first_gui_app(void* p) {
    UNUSED(p);
    GuiDemo* gui_demo = malloc(sizeof(GuiDemo));

    // Allocate state
    gui_demo->state = malloc(sizeof(GuiDemoState));
    gui_demo->state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    gui_demo->state->counter = 0;
    gui_demo->state->button_pressed = false;

    // Set system callbacks
    gui_demo->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    gui_demo->view_port = view_port_alloc();
    view_port_draw_callback_set(gui_demo->view_port, draw_callback, gui_demo->state);
    view_port_input_callback_set(gui_demo->view_port, input_callback, gui_demo->event_queue);

    // Register viewport
    gui_demo->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui_demo->gui, gui_demo->view_port, GuiLayerFullscreen);

    // Main loop
    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(gui_demo->event_queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(gui_demo->state->mutex, FuriWaitForever);
            
            if(event.type == InputTypePress) {
                if(event.key == InputKeyOk) {
                    gui_demo->state->button_pressed = true;
                    gui_demo->state->counter++;
                } else if(event.key == InputKeyBack) {
                    running = false;
                }
            } else if(event.type == InputTypeRelease) {
                if(event.key == InputKeyOk) {
                    gui_demo->state->button_pressed = false;
                }
            }

            furi_mutex_release(gui_demo->state->mutex);
            view_port_update(gui_demo->view_port);
        }
    }

    cleanup_callback(gui_demo);
    furi_record_close(RECORD_GUI);
    return 0;
} 