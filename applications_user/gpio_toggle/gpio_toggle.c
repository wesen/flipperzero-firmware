// gpio_toggle_app.h
typedef struct GpioToggleApp GpioToggleApp;

// gpio_toggle_app.c
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <furi_hal_gpio.h>

struct GpioToggleApp {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    bool pin_state;
};

static void draw_callback(Canvas* canvas, void* context) {
    GpioToggleApp* app = context;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GPIO Toggle Demo");
    canvas_draw_str(canvas, 2, 30, app->pin_state ? "Pin: HIGH" : "Pin: LOW");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 50, "Press OK to toggle");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    GpioToggleApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

int32_t gpio_toggle_app(void* p) {
    UNUSED(p);
    GpioToggleApp* app = malloc(sizeof(GpioToggleApp));

    // Initialize GPIO
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    app->pin_state = false;
    furi_hal_gpio_write(&gpio_ext_pa7, app->pin_state);

    // Create UI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Main loop
    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyOk) {
                app->pin_state = !app->pin_state;
                furi_hal_gpio_write(&gpio_ext_pa7, app->pin_state);
                view_port_update(app->view_port);
            } else if(event.key == InputKeyBack) {
                running = false;
            }
        }
    }

    // Cleanup
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}