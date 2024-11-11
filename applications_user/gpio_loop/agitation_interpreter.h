#pragma once

#include "agitation_sequence.h"

#define MAX_LOOP_DEPTH 3
#define LOOP_CONTINUOUS 0

typedef struct AgitationInterpreterState {
    // Current execution context
    const AgitationMovementStatic* current_sequence;
    size_t sequence_length;
    size_t current_index;

    // Loop tracking
    struct {
        const AgitationMovementStatic* sequence;
        size_t sequence_length;
        uint32_t remaining_iterations;
        size_t start_index;
        uint32_t original_count;
    } loop_stack[MAX_LOOP_DEPTH];
    uint8_t loop_depth;

    // Current movement details
    AgitationMovementType current_movement;
    uint32_t time_remaining;

    // Callback for motor control
    void (*motor_cw_callback)(bool enable);
    void (*motor_ccw_callback)(bool enable);
} AgitationInterpreterState;

// Initialize the interpreter state
void agitation_interpreter_init(
    AgitationInterpreterState* state,
    const struct AgitationMovementStatic* sequence,
    size_t sequence_length,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool));

// Step the interpreter forward by 1 second
bool agitation_interpreter_tick(AgitationInterpreterState* state);

// Reset the interpreter to its initial state
void agitation_interpreter_reset(AgitationInterpreterState* state);
