#include <string.h>
#include "agitation_interpreter.h"

// #define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

void agitation_interpreter_init(
    AgitationInterpreterState* state,
    const struct AgitationMovementStatic* sequence,
    size_t sequence_length,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool)) {
    // Reset all state
    memset(state, 0, sizeof(AgitationInterpreterState));

    // Set initial sequence
    state->current_sequence = sequence;
    state->sequence_length = sequence_length;
    state->current_index = 0;
    state->loop_depth = 0;

    // Set motor control callbacks
    state->motor_cw_callback = motor_cw;
    state->motor_ccw_callback = motor_ccw;
}

static void stop_current_movement(AgitationInterpreterState* state) {
    switch(state->current_movement) {
    case AgitationMovementTypeCW:
        state->motor_cw_callback(false);
        break;
    case AgitationMovementTypeCCW:
        state->motor_ccw_callback(false);
        break;
    default:
        break;
    }
    state->current_movement = AgitationMovementTypePause;
}

static void
    start_movement(AgitationInterpreterState* state, const AgitationMovementStatic* movement) {
    // Stop any existing movement
    stop_current_movement(state);

    // Set new movement and duration
    state->current_movement = movement->type;
    state->time_remaining = movement->duration;

    // Activate motor based on movement type
    switch(movement->type) {
    case AgitationMovementTypeCW:
        state->motor_cw_callback(true);
        break;
    case AgitationMovementTypeCCW:
        state->motor_ccw_callback(true);
        break;
    default:
        break;
    }
}

// New helper function to push loop context
static void push_loop_context(
    AgitationInterpreterState* state,
    const struct AgitationMovementStatic* loop_sequence,
    size_t loop_length,
    uint32_t loop_count) {
    if(state->loop_depth >= MAX_LOOP_DEPTH) {
        DEBUG_PRINT("Max loop depth reached, cannot push loop context\n");
        return;
    }

    // Push current loop context
    state->loop_stack[state->loop_depth].sequence = state->current_sequence;
    state->loop_stack[state->loop_depth].sequence_length = state->sequence_length;
    state->loop_stack[state->loop_depth].start_index = state->current_index;

    // Store original count and set remaining iterations
    state->loop_stack[state->loop_depth].original_count = loop_count;
    state->loop_stack[state->loop_depth].remaining_iterations =
        (loop_count == LOOP_CONTINUOUS) ? loop_count : loop_count;

    // Switch to loop sequence
    state->current_sequence = loop_sequence;
    state->sequence_length = loop_length;
    state->current_index = 0;
    state->loop_depth++;

    DEBUG_PRINT("Pushed loop context. New depth: %d\n", state->loop_depth);
}

// New helper function to pop loop context
static bool pop_loop_context(AgitationInterpreterState* state) {
    if(state->loop_depth == 0) {
        DEBUG_PRINT("Cannot pop loop context, already at root\n");
        return false;
    }

    // Decrement loop depth
    state->loop_depth--;

    DEBUG_PRINT("Popped loop context. New depth: %d\n", state->loop_depth);

    // Restore previous sequence
    state->current_sequence = state->loop_stack[state->loop_depth].sequence;
    state->sequence_length = state->loop_stack[state->loop_depth].sequence_length;
    state->current_index = state->loop_stack[state->loop_depth].start_index + 1;

    return true;
}

bool agitation_interpreter_tick(AgitationInterpreterState* state) {
    // Check for loop end at the start of the tick
    if(state->current_index >= state->sequence_length) {
        // Check if we're in a loop
        if(state->loop_depth > 0) {
            // Decrement loop iterations only if not continuous
            if(state->loop_stack[state->loop_depth - 1].original_count != LOOP_CONTINUOUS) {
                state->loop_stack[state->loop_depth - 1].remaining_iterations--;
            }

            DEBUG_PRINT(
                "End of sequence, loop depth %d, remaining iterations %u\n",
                state->loop_depth,
                state->loop_stack[state->loop_depth - 1].remaining_iterations);

            // Check if we should continue looping
            if(state->loop_stack[state->loop_depth - 1].original_count == LOOP_CONTINUOUS ||
               state->loop_stack[state->loop_depth - 1].remaining_iterations > 0) {
                // Reset to start of loop sequence
                state->current_index = 0;
                DEBUG_PRINT("Continuing loop, reset to start\n");
            } else {
                // Pop loop context
                if(!pop_loop_context(state)) {
                    DEBUG_PRINT("No more sequences, stopping movement\n");
                    stop_current_movement(state);
                    return false;
                }
            }
        } else {
            DEBUG_PRINT("No more sequences, stopping movement\n");
            stop_current_movement(state);
            return false;
        }
    }

    // Decrement time remaining for current movement
    if(state->time_remaining > 0) {
        state->time_remaining--;
        DEBUG_PRINT("Tick: Time remaining %u\n", state->time_remaining);
        return true;
    }

    DEBUG_PRINT("\n\n-----------------------------------\n\n");
    DEBUG_PRINT(
        "Tick: Processing sequence, index %zu, length %zu, loop depth %d\n",
        state->current_index,
        state->sequence_length,
        state->loop_depth);

    // Move to next step
    while(state->current_index < state->sequence_length) {
        const AgitationMovementStatic* current_movement =
            &state->current_sequence[state->current_index];

        DEBUG_PRINT(
            "Processing movement at index %zu, type %d\n",
            state->current_index,
            current_movement->type);

        switch(current_movement->type) {
        case AgitationMovementTypeCW:
        case AgitationMovementTypeCCW:
            DEBUG_PRINT("Starting motor movement\n");
            start_movement(state, current_movement);
            state->current_index++;
            return true;

        case AgitationMovementTypePause:
            DEBUG_PRINT("Pause movement\n");
            stop_current_movement(state);
            start_movement(state, current_movement);
            state->current_index++;
            return true;

        case AgitationMovementTypeLoop:
            DEBUG_PRINT("Entering loop processing\n");

            // Use the new push_loop_context helper
            push_loop_context(
                state,
                current_movement->loop.sequence,
                current_movement->loop.sequence_length,
                current_movement->loop.count);
            break;

        default:
            DEBUG_PRINT("Unknown movement type, skipping\n");
            // Unknown movement type, move to next
            state->current_index++;
            break;
        }

        // If we've reached the end of a sequence
        if(state->current_index >= state->sequence_length) {
            // Check if we're in a loop
            if(state->loop_depth > 0) {
                // Decrement loop iterations only if not continuous
                if(state->loop_stack[state->loop_depth - 1].original_count != LOOP_CONTINUOUS) {
                    state->loop_stack[state->loop_depth - 1].remaining_iterations--;
                }

                DEBUG_PRINT(
                    "End of sequence, loop depth %d, remaining iterations %u\n",
                    state->loop_depth,
                    state->loop_stack[state->loop_depth - 1].remaining_iterations);

                // Check if we should continue looping
                if(state->loop_stack[state->loop_depth - 1].original_count == LOOP_CONTINUOUS ||
                   state->loop_stack[state->loop_depth - 1].remaining_iterations > 0) {
                    // Reset to start of loop sequence
                    state->current_index = 0;
                    DEBUG_PRINT("Continuing loop, reset to start\n");
                } else {
                    // Pop loop context
                    if(!pop_loop_context(state)) {
                        DEBUG_PRINT("No more sequences, stopping movement\n");
                        stop_current_movement(state);
                        return false;
                    }
                }
            } else {
                DEBUG_PRINT("No more sequences, stopping movement\n");
                // No more loops, we're done
                stop_current_movement(state);
                return false;
            }
        }
    }

    return false;
}

void agitation_interpreter_reset(AgitationInterpreterState* state) {
    agitation_interpreter_init(
        state,
        state->current_sequence,
        state->sequence_length,
        state->motor_cw_callback,
        state->motor_ccw_callback);
}
