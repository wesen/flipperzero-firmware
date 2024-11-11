#include <furi.h>
#include <furi_hal_gpio.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>

typedef enum {
    GpioStatePin1On,
    GpioStatePin1Wait,
    GpioStatePin2On,
    GpioStatePin2Wait,
} GpioState;

typedef struct {
    FuriEventLoop* event_loop;
    ViewPort* view_port;
    Gui* gui;
    FuriEventLoopTimer* state_timer;
    GpioState current_state;
    uint32_t time_remaining;
} GpioLoopApp;

// Define our GPIO pins
const GpioPin* const pin1 = &gpio_ext_pa7;
const GpioPin* const pin2 = &gpio_ext_pa6;

static void draw_callback(Canvas* canvas, void* context) {
    GpioLoopApp* app = context;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Draw title
    canvas_draw_str(canvas, 2, 12, "GPIO Loop Demo");
    
    // Draw status
    canvas_set_font(canvas, FontSecondary);
    char status[32];
    
    switch(app->current_state) {
        case GpioStatePin1On:
            snprintf(status, sizeof(status), "Pin 1: ON (%lds)", app->time_remaining);
            break;
        case GpioStatePin1Wait:
            snprintf(status, sizeof(status), "Wait 1->2 (%lds)", app->time_remaining);
            break;
        case GpioStatePin2On:
            snprintf(status, sizeof(status), "Pin 2: ON (%lds)", app->time_remaining);
            break;
        case GpioStatePin2Wait:
            snprintf(status, sizeof(status), "Wait 2->1 (%lds)", app->time_remaining);
            break;
    }
    
    canvas_draw_str(canvas, 2, 30, status);
    
    // Draw pin states
    canvas_draw_str(canvas, 2, 45, "Pin 1:");
    canvas_draw_str(canvas, 2, 55, "Pin 2:");
    
    canvas_draw_str(
        canvas,
        40,
        45,
        furi_hal_gpio_read(pin1) ? "HIGH" : "LOW");
    canvas_draw_str(
        canvas,
        40,
        55,
        furi_hal_gpio_read(pin2) ? "HIGH" : "LOW");
}

static void timer_callback(void* context) {
    GpioLoopApp* app = context;
    
    if(app->time_remaining > 0) {
        app->time_remaining--;
        view_port_update(app->view_port);
        return;
    }
    
    // Time's up, move to next state
    switch(app->current_state) {
        case GpioStatePin1On:
            furi_hal_gpio_write(pin1, false);
            app->current_state = GpioStatePin1Wait;
            app->time_remaining = 5;
            break;
            
        case GpioStatePin1Wait:
            furi_hal_gpio_write(pin2, true);
            app->current_state = GpioStatePin2On;
            app->time_remaining = 7;
            break;
            
        case GpioStatePin2On:
            furi_hal_gpio_write(pin2, false);
            app->current_state = GpioStatePin2Wait;
            app->time_remaining = 12;
            break;
            
        case GpioStatePin2Wait:
            furi_hal_gpio_write(pin1, true);
            app->current_state = GpioStatePin1On;
            app->time_remaining = 10;
            break;
    }
    
    view_port_update(app->view_port);
}

static void input_callback(InputEvent* input_event, void* context) {
    GpioLoopApp* app = context;
    if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        furi_event_loop_stop(app->event_loop);
    }
}

static void gpio_init() {
    furi_hal_gpio_init_simple(pin1, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(pin2, GpioModeOutputPushPull);
    furi_hal_gpio_write(pin1, false);
    furi_hal_gpio_write(pin2, false);
}

static void gpio_deinit() {
    furi_hal_gpio_write(pin1, false);
    furi_hal_gpio_write(pin2, false);
    furi_hal_gpio_init_simple(pin1, GpioModeAnalog);
    furi_hal_gpio_init_simple(pin2, GpioModeAnalog);
}

int32_t gpio_loop_app(void* p) {
    UNUSED(p);
    GpioLoopApp* app = malloc(sizeof(GpioLoopApp));

    // Initialize GPIO
    gpio_init();
    
    // Create event loop
    app->event_loop = furi_event_loop_alloc();
    
    // Create GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Create timer
    app->state_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app);
    furi_event_loop_timer_start(app->state_timer, 1000); // 1 second intervals
    
    // Set initial state
    app->current_state = GpioStatePin1On;
    app->time_remaining = 10;
    furi_hal_gpio_write(pin1, true);
    
    // Run event loop
    furi_event_loop_run(app->event_loop);
    
    // Cleanup
    furi_event_loop_timer_free(app->state_timer);
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_event_loop_free(app->event_loop);
    gpio_deinit();
    free(app);
    
    return 0;
} 