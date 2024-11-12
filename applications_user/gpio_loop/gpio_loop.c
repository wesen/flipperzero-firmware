#include <furi.h>
#include <furi_hal_gpio.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>
#include "agitation_sequence.h"
#include "agitation_processes.h"
#include "agitation_process_interpreter.h"

typedef struct {
    FuriEventLoop* event_loop;
    ViewPort* view_port;
    Gui* gui;
    FuriEventLoopTimer* state_timer;

    // Process state
    AgitationProcessInterpreterState process_state;
    const AgitationProcessStatic* current_process;
    bool process_active;

    // Display info
    char status_text[64];
    char step_text[32];
    char movement_text[32];

    // Additional state tracking
    bool paused;
} GpioLoopApp;

// Define our GPIO pins (active low)
const GpioPin* const pin_cw = &gpio_ext_pa7;
const GpioPin* const pin_ccw = &gpio_ext_pa6;

static void motor_stop() {
    // Set both pins high (inactive) for active low
    furi_hal_gpio_write(pin_cw, true);
    furi_hal_gpio_write(pin_ccw, true);
}

static void motor_cw_callback(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
        furi_delay_us(1000); // 1ms delay for safety
        furi_hal_gpio_write(pin_cw, false); // Enable CW (active low)
    } else {
        furi_hal_gpio_write(pin_cw, true); // Disable CW
    }
}

static void motor_ccw_callback(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_cw, true); // Disable CW
        furi_delay_us(1000); // 1ms delay for safety
        furi_hal_gpio_write(pin_ccw, false); // Enable CCW (active low)
    } else {
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
    }
}

static void draw_callback(Canvas* canvas, void* context) {
    GpioLoopApp* app = context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    // Draw title
    canvas_draw_str(canvas, 2, 12, "C41 Process");

    // Draw current step info
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 24, app->step_text);

    // Draw status or user message
    if(app->process_state.waiting_for_user) {
        canvas_draw_str(canvas, 2, 36, app->process_state.user_message ? 
                       app->process_state.user_message : "Press OK to continue");
    } else {
        canvas_draw_str(canvas, 2, 36, app->status_text);
    }

    // Draw movement state if not waiting for user
    if(!app->process_state.waiting_for_user) {
        canvas_draw_str(canvas, 2, 48, app->movement_text);
    }

    // Draw pin states
    canvas_draw_str(canvas, 2, 60, "CW:");
    canvas_draw_str(canvas, 50, 60, !furi_hal_gpio_read(pin_cw) ? "ON" : "OFF");

    canvas_draw_str(canvas, 2, 70, "CCW:");
    canvas_draw_str(canvas, 50, 70, !furi_hal_gpio_read(pin_ccw) ? "ON" : "OFF");

    // Draw control hints
    if(app->process_active) {
        if(app->process_state.waiting_for_user) {
            elements_button_center(canvas, "Continue");
        } else if(app->paused) {
            elements_button_center(canvas, "Resume");
        } else {
            elements_button_center(canvas, "Pause");
        }
        elements_button_right(canvas, "Skip");
        elements_button_left(canvas, "Restart");
    } else {
        elements_button_center(canvas, "Start");
    }
}

static void timer_callback(void* context) {
    GpioLoopApp* app = context;

    if(app->process_active && !app->paused) {
        // Tick the process interpreter
        bool still_active = agitation_process_interpreter_tick(&app->process_state);

        // Update status texts
        const AgitationStepStatic* current_step =
            &app->current_process->steps[app->process_state.current_step_index];

        snprintf(app->step_text, sizeof(app->step_text), "Step: %s", current_step->name);

        // Get current loop info if in a loop
        if(app->process_state.movement_interpreter.loop_depth > 0) {
            // Show elapsed time in current loop
            uint32_t elapsed =
                app->process_state.movement_interpreter
                    .loop_stack[app->process_state.movement_interpreter.loop_depth - 1]
                    .elapsed_duration;
            uint32_t max_duration =
                app->process_state.movement_interpreter
                    .loop_stack[app->process_state.movement_interpreter.loop_depth - 1]
                    .max_duration;

            if(max_duration > 0) {
                snprintf(
                    app->status_text,
                    sizeof(app->status_text),
                    "%s Time: %lus/%lus",
                    app->paused ? "[PAUSED]" : "",
                    elapsed,
                    max_duration);
            } else {
                snprintf(
                    app->status_text,
                    sizeof(app->status_text),
                    "%s Time: %lus",
                    app->paused ? "[PAUSED]" : "",
                    elapsed);
            }
        } else {
            // Show remaining time for current movement
            snprintf(
                app->status_text,
                sizeof(app->status_text),
                "%s Time left: %lus",
                app->paused ? "[PAUSED]" : "",
                app->process_state.movement_interpreter.time_remaining);
        }

        // Update movement text based on current state (remember active low)
        const char* movement_str = "Idle";
        if(!furi_hal_gpio_read(pin_cw)) {
            movement_str = "Clockwise";
        } else if(!furi_hal_gpio_read(pin_ccw)) {
            movement_str = "Counter-CW";
        }
        snprintf(app->movement_text, sizeof(app->movement_text), "Movement: %s", movement_str);

        app->process_active = still_active;
        if(!still_active) {
            motor_stop(); // Ensure motors are stopped when process ends
        }
    }

    view_port_update(app->view_port);
}

static void input_callback(InputEvent* input_event, void* context) {
    GpioLoopApp* app = context;

    if(input_event->type == InputTypeShort) {
        if(input_event->key == InputKeyOk) {
            if(!app->process_active) {
                // Start new process
                agitation_process_interpreter_init(
                    &app->process_state,
                    &C41_FULL_PROCESS_STATIC,
                    motor_cw_callback,
                    motor_ccw_callback);
                app->process_active = true;
                app->paused = false;
            } else if(app->process_state.waiting_for_user) {
                // Handle user confirmation
                agitation_process_interpreter_confirm(&app->process_state);
            } else {
                // Toggle pause
                app->paused = !app->paused;
                if(app->paused) {
                    motor_stop();
                }
            }
        } else if(app->process_active && input_event->key == InputKeyRight) {
            // Skip to next step (only if not waiting for user)
            if(!app->process_state.waiting_for_user) {
                motor_stop();
                app->process_state.current_step_index++;
                if(app->process_state.current_step_index >= app->current_process->steps_length) {
                    app->process_active = false;
                }
            }
        } else if(app->process_active && input_event->key == InputKeyLeft) {
            // Restart current step
            motor_stop();
            agitation_process_interpreter_init(
                &app->process_state, app->current_process, motor_cw_callback, motor_ccw_callback);
            app->process_state.current_step_index =
                app->process_state.current_step_index; // Stay on current step
        }
    } else if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        if(app->process_active) {
            // Stop process
            app->process_active = false;
            app->paused = false;
            motor_stop();
        } else {
            furi_event_loop_stop(app->event_loop);
        }
    }
}

static void gpio_init() {
    furi_hal_gpio_init_simple(pin_cw, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeOutputPushPull);
    motor_stop(); // Initialize both pins to inactive state
}

static void gpio_deinit() {
    motor_stop(); // Ensure motors are stopped
    furi_hal_gpio_init_simple(pin_cw, GpioModeAnalog);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeAnalog);
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
        app->event_loop, timer_callback, FuriEventLoopTimerTypePeriodic, app);
    furi_event_loop_timer_start(app->state_timer, 1000); // 1 second intervals

    // Set initial state
    app->current_process = &C41_FULL_PROCESS_STATIC;
    app->process_active = false;
    app->paused = false;
    snprintf(app->status_text, sizeof(app->status_text), "Press OK to start");
    snprintf(app->step_text, sizeof(app->step_text), "Ready");
    snprintf(app->movement_text, sizeof(app->movement_text), "Movement: Idle");

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
