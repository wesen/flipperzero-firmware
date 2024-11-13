#include "../film_developer/processes/c41_process.hpp"
#include "../film_developer/movement/movement_loader.hpp"
#include "../film_developer/agitation_process_interpreter.hpp"
#include "mock_controller.hpp"
#include <iostream>
#include <string>

// Clear screen function - platform independent
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_step_info(const AgitationStepStatic* step) {
    if (!step) return;
    
    DEBUG_PRINT("Step: %s", step->name);
    DEBUG_PRINT("Description: %s", step->description);
    DEBUG_PRINT("Temperature: %.1f°C", step->temperature);
    DEBUG_PRINT("Sequence Length: %zu movements", step->sequence_length);
}

void print_process_state(const AgitationProcessInterpreter& interpreter) {
    DEBUG_PRINT("Process State: %s", 
        interpreter.getState() == AgitationProcessState::Running ? "Running" :
        interpreter.getState() == AgitationProcessState::Idle ? "Idle" :
        interpreter.getState() == AgitationProcessState::Complete ? "Complete" :
        "Error");
    
    DEBUG_PRINT("Step Index: %lu/%lu", interpreter.getCurrentStepIndex() + 1, interpreter.getCurrentProcess()->steps_length);
    
    if (interpreter.isWaitingForUser()) {
        DEBUG_PRINT("Waiting for User: %s", interpreter.getUserMessage());
    }
}

void print_movement_timing(const AgitationProcessInterpreter& interpreter) {
    uint32_t elapsed = interpreter.getCurrentMovementTimeElapsed();
    uint32_t remaining = interpreter.getCurrentMovementTimeRemaining();
    uint32_t total = interpreter.getCurrentMovementDuration();
    
    if (total > 0) {
        DEBUG_PRINT("Movement Timing:");
        DEBUG_PRINT("  Total Duration: %u ticks", total);
        DEBUG_PRINT("  Elapsed Time:   %u ticks", elapsed);
        DEBUG_PRINT("  Remaining Time: %u ticks", remaining);
        DEBUG_PRINT("  Progress:       %.1f%%", 
            (elapsed * 100.0f) / total);
    }
}

void run_c41_color_developer() {
    DEBUG_PRINT("\nRunning C41 Color Developer Step:");

    // Create test motor controller
    MockController motor;

    // Create and initialize process interpreter
    AgitationProcessInterpreter interpreter;
    interpreter.init(&C41_FULL_PROCESS_STATIC, &motor);

    // Skip pre-wash to start at color developer
    // interpreter.skipToNextStep();

    std::string input;
    int tick_count = 0;

    while (true) {
        clearScreen();
        
        DEBUG_PRINT("\n=== Tick %d ===", tick_count + 1);
        
        // Print process state information
        print_process_state(interpreter);
        
        // Print current step info
        if (const auto* step = interpreter.getCurrentStep()) {
            print_step_info(step);
        }
        
        // Print current movement info if available
        if (const auto* movement = interpreter.getCurrentMovement()) {
            DEBUG_PRINT("\nCurrent Movement:");
            movement->print();
            print_movement_timing(interpreter);
        }
        
        bool active = interpreter.tick();

        if(interpreter.isWaitingForUser()) {
            DEBUG_PRINT("\nUser Input Required:");
            DEBUG_PRINT("Message: %s", interpreter.getUserMessage());
            interpreter.confirm();
        }

        if(!active) {
            DEBUG_PRINT("\nProcess Status: %s", 
                interpreter.getState() == AgitationProcessState::Complete ? 
                "Completed Successfully" : "Error Occurred");
            break;
        }

        DEBUG_PRINT("\n---");
        DEBUG_PRINT("Press Enter to continue (q to quit)...");
        
        std::getline(std::cin, input);
        if (input == "q" || input == "Q") {
            DEBUG_PRINT("Quitting...");
            break;
        }
        
        tick_count++;
    }
}

int main() {
    run_c41_color_developer();
    return 0;
}
