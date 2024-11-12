#include <string.h>
#include "agitation_interpreter.h"

// #define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#ifdef HOST
#include <stdio.h>
#endif

// New macro to access the current loop stack entry
#define CURRENT_LOOP(state) (state->loop_stack[state->loop_depth - 1])

// Add new field to loop stack
typedef struct {
    const AgitationMovementStatic* sequence;
    size_t sequence_length;
    uint32_t remaining_iterations;
    size_t start_index;
    uint32_t original_count;
    uint32_t elapsed_duration; // New field to track elapsed time
    uint32_t max_duration; // New field for max duration
} LoopStackEntry;

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
    uint32_t loop_count,
    uint32_t max_duration) {
    if(state->loop_depth >= MAX_LOOP_DEPTH) {
        DEBUG_PRINT("Max loop depth reached, cannot push loop context\n");
        return;
    }

    DEBUG_PRINT(
        "Pushing loop context, depth %d, max_duration %u, length %zu \n",
        state->loop_depth,
        max_duration,
        loop_length);

    // Push current loop context
    if(state->loop_depth > 0) {
        // save current state
        CURRENT_LOOP(state).sequence = state->current_sequence;
        CURRENT_LOOP(state).sequence_length = state->sequence_length;
        CURRENT_LOOP(state).start_index = state->current_index;
    }

    state->loop_depth++;

    // push new state on top
    CURRENT_LOOP(state).sequence = loop_sequence;
    CURRENT_LOOP(state).sequence_length = loop_length;
    CURRENT_LOOP(state).start_index = 0;
    CURRENT_LOOP(state).original_count = loop_count;
    CURRENT_LOOP(state).max_duration = max_duration;
    CURRENT_LOOP(state).elapsed_duration = 0;
    CURRENT_LOOP(state).stop = false;

    // Set remaining iterations based on count/duration logic
    if(loop_count == 0 && max_duration == 0) {
        CURRENT_LOOP(state).remaining_iterations = LOOP_CONTINUOUS;
    } else if(loop_count > 0) {
        CURRENT_LOOP(state).remaining_iterations = loop_count;
    } else {
        CURRENT_LOOP(state).remaining_iterations = LOOP_CONTINUOUS;
    }

    // Switch to loop sequence
    state->current_sequence = loop_sequence;
    state->sequence_length = loop_length;
    state->current_index = 0;

    DEBUG_PRINT(
        "Pushed loop context. New depth: %d, length %zu\n",
        state->loop_depth,
        state->sequence_length);
}

// New helper function to pop loop context
static bool pop_loop_context(AgitationInterpreterState* state) {
    if(state->loop_depth == 0) {
        DEBUG_PRINT("Cannot pop loop context, already at root\n");
        return false;
    }

    // Decrement loop depth
    state->loop_depth--;

    if(state->loop_depth == 0) {
        return false;
    }

    // pop until reaching the top level or not stopping
    while(state->loop_depth > 1 && state->loop_stack[state->loop_depth - 1].stop) {
        DEBUG_PRINT(
            "Popping loop context, depth %d, stop %d\n",
            state->loop_depth,
            state->loop_stack[state->loop_depth].stop);
        state->loop_depth--;
    }

    // Restore previous sequence
    state->current_sequence = CURRENT_LOOP(state).sequence;
    state->sequence_length = CURRENT_LOOP(state).sequence_length;
    state->current_index = CURRENT_LOOP(state).start_index + 1;

    DEBUG_PRINT(
        "Popped loop context. New depth: %d, current_index %zu, sequence_length %zu, stop %d\n",
        state->loop_depth,
        state->current_index,
        state->sequence_length,
        state->loop_stack[state->loop_depth].stop);

    return true;
}

static bool check_loop_end(AgitationInterpreterState* state) {
    if(state->current_index >= state->sequence_length) {
        return true;
    }
    if(state->loop_depth > 0) {
        return CURRENT_LOOP(state).stop;
    }
    return false;
}

static bool handle_loop_end(AgitationInterpreterState* state) {
    // Check for loop end at the start of the tick
    if(check_loop_end(state)) {
        // Check if we're in a loop
        if(state->loop_depth > 0) {
            // Decrement loop iterations only if not continuous
            if(CURRENT_LOOP(state).original_count != LOOP_CONTINUOUS) {
                CURRENT_LOOP(state).remaining_iterations--;
            }

            DEBUG_PRINT(
                "End of sequence, loop depth %d, remaining iterations %u, stopping %d, max_duration %u, elapsed_duration %u\n",
                state->loop_depth,
                CURRENT_LOOP(state).remaining_iterations,
                CURRENT_LOOP(state).stop,
                CURRENT_LOOP(state).max_duration,
                CURRENT_LOOP(state).elapsed_duration);

            // Check if we should continue looping
            if((CURRENT_LOOP(state).original_count == LOOP_CONTINUOUS ||
                CURRENT_LOOP(state).remaining_iterations > 0) &&
               !CURRENT_LOOP(state).stop) {
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
                DEBUG_PRINT("Popped loop context, continuing\n");
            }
        } else {
            DEBUG_PRINT("No more sequences, stopping movement\n");
            stop_current_movement(state);
            return false;
        }
    }

    return true;
}

bool agitation_interpreter_tick(AgitationInterpreterState* state) {
    DEBUG_PRINT("\n\n-----------------------------------\n\n");
    DEBUG_PRINT(
        "Tick: Processing sequence, index %zu, length %zu, loop depth %d\n",
        state->current_index,
        state->sequence_length,
        state->loop_depth);

    if(!handle_loop_end(state)) {
        DEBUG_PRINT("Loop end handled, stopping movement\n");
        return false;
    }

    // Update duration tracking for active loops
    if(state->loop_depth > 0) {
        bool interrupt_loops = false;
        for(int i = 0; i < state->loop_depth; i++) {
            DEBUG_PRINT(
                "Increment loop %d elapsed duration %u, max_duration %u\n",
                i,
                state->loop_stack[i].elapsed_duration,
                state->loop_stack[i].max_duration);
            state->loop_stack[i].elapsed_duration++;
            if(interrupt_loops) {
                DEBUG_PRINT(
                    "Interrupting loop, depth %d, stop %d\n", i, state->loop_stack[i].stop);
                state->loop_stack[i].stop = true;
            }

            // Check if max duration reached
            if(state->loop_stack[i].max_duration > 0 &&
               state->loop_stack[i].elapsed_duration >= state->loop_stack[i].max_duration) {
                interrupt_loops = true;
                DEBUG_PRINT(
                    "Max duration reached, interrupting loop, depth %d, stop %d\n",
                    i,
                    state->loop_stack[i].stop);
                // Force loop to end
                state->loop_stack[i].stop = true;
            }
        }

        if(interrupt_loops) {
            DEBUG_PRINT("Interrupting loops\n");
            if(!handle_loop_end(state)) {
                return false;
            }
            return true;
        }
    }

    // Decrement time remaining for current movement
    if(state->time_remaining > 0) {
        state->time_remaining--;
        DEBUG_PRINT("Tick: Time remaining %u\n", state->time_remaining);
        return true;
    }

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
            DEBUG_PRINT("Entering loop processing, \n");
            push_loop_context(
                state,
                current_movement->loop.sequence,
                current_movement->loop.sequence_length,
                current_movement->loop.count,
                current_movement->loop.max_duration);
            break;

        case AgitationMovementTypeWaitUser:
            // Wait for user continues to be handled by the UI layer
            // Just stay on this movement until UI signals to continue
            return true;

        default:
            DEBUG_PRINT("Unknown movement type, skipping\n");
            state->current_index++;
            break;
        }
    }

    if(!handle_loop_end(state)) {
        DEBUG_PRINT("Loop end handled, stopping movement\n");
        return false;
    }

    return true;
}

void agitation_interpreter_reset(AgitationInterpreterState* state) {
    agitation_interpreter_init(
        state,
        state->current_sequence,
        state->sequence_length,
        state->motor_cw_callback,
        state->motor_ccw_callback);
}
