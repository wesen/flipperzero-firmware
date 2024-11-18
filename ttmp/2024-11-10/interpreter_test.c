#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Include the interpreter headers
#include "../../gpio_loop/agitation_interpreter.h"
#include "../../gpio_loop/agitation_process_interpreter.h"
#include "../../gpio_loop/agitation_processes.h"

// Mock motor and logging state
typedef struct {
    bool cw_active;
    bool ccw_active;
    int cw_count;
    int ccw_count;
    int pause_count;
} MockMotorState;

static MockMotorState g_motor_state = {0};

// Mock motor control callbacks
void mock_motor_cw(bool enable) {
    g_motor_state.cw_active = enable;
    if(enable) g_motor_state.cw_count++;
    printf("Motor CW: %s\n", enable ? "ON" : "OFF");
}

void mock_motor_ccw(bool enable) {
    g_motor_state.ccw_active = enable;
    if(enable) g_motor_state.ccw_count++;
    printf("Motor CCW: %s\n", enable ? "ON" : "OFF");
}

// Reset mock states
void reset_mock_states() {
    memset(&g_motor_state, 0, sizeof(MockMotorState));
}

// Test C41 full process using process interpreter
void test_c41_full_process() {
    printf("\n--- Running test_c41_full_process ---\n");

    AgitationProcessInterpreterState process_state;
    reset_mock_states();

    // Initialize process interpreter with C41 full process
    agitation_process_interpreter_init(
        &process_state, &C41_FULL_PROCESS_STATIC, mock_motor_cw, mock_motor_ccw);

    // Run process with max iterations to prevent infinite loop
    int max_iterations = 1000;
    while(agitation_process_interpreter_tick(&process_state) && max_iterations-- > 0) {
        // Just let it run
    }

    // Validate expectations
    printf("\nProcess Completed. Motor Counts:\n");
    printf("CW count: %d, CCW count: %d\n", g_motor_state.cw_count, g_motor_state.ccw_count);

    // Basic assertions
    assert(g_motor_state.cw_count > 0);
    assert(g_motor_state.ccw_count > 0);
    assert(process_state.process_state == AgitationProcessStateComplete);
}

// Test B&W standard development process
void test_bw_standard_dev_process() {
    printf("\n--- Running test_bw_standard_dev_process ---\n");

    AgitationProcessInterpreterState process_state;
    reset_mock_states();

    // Initialize process interpreter with B&W standard development process
    agitation_process_interpreter_init(
        &process_state, &BW_STANDARD_DEV_STATIC, mock_motor_cw, mock_motor_ccw);

    // Run process with max iterations to prevent infinite loop
    int max_iterations = 1000;
    while(agitation_process_interpreter_tick(&process_state) && max_iterations-- > 0) {
        // Just let it run
    }

    // Validate expectations
    printf("\nProcess Completed. Motor Counts:\n");
    printf("CW count: %d, CCW count: %d\n", g_motor_state.cw_count, g_motor_state.ccw_count);

    // Basic assertions
    assert(g_motor_state.cw_count > 0);
    assert(g_motor_state.ccw_count > 0);
    assert(process_state.process_state == AgitationProcessStateComplete);
}

int main() {
    // Run process tests
    test_c41_full_process();
    // test_bw_standard_dev_process();

    printf("\nAll process tests passed successfully!\n");
    return 0;
}
