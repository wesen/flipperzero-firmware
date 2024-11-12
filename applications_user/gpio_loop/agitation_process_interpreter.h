#pragma once

#include "agitation_sequence.h"
#include "agitation_interpreter.h"

typedef enum {
    AgitationProcessStateIdle,
    AgitationProcessStateRunning,
    AgitationProcessStateComplete,
    AgitationProcessStateError
} AgitationProcessState;

typedef struct {
    // The full process to interpret
    const AgitationProcessStatic* process;

    // Current step being executed
    size_t current_step_index;

    // Movement interpreter for current step
    AgitationInterpreterState movement_interpreter;

    // Process-level state tracking
    AgitationProcessState process_state;

    // Temperature tracking (informational only)
    float current_temperature;
    float target_temperature;

    // Callbacks for motor control
    void (*motor_cw_callback)(bool enable);
    void (*motor_ccw_callback)(bool enable);

    // User interaction state
    bool waiting_for_user;
    const char* user_message;  // Points to current wait message if waiting_for_user is true
} AgitationProcessInterpreterState;

// Initialize process interpreter
void agitation_process_interpreter_init(
    AgitationProcessInterpreterState* state,
    const AgitationProcessStatic* process,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool)
);

// Advance process interpreter by one tick
bool agitation_process_interpreter_tick(
    AgitationProcessInterpreterState* state
);

// Reset process interpreter
void agitation_process_interpreter_reset(
    AgitationProcessInterpreterState* state
);

// Add new function to handle user confirmation
void agitation_process_interpreter_confirm(AgitationProcessInterpreterState* state); 