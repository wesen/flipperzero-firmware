#include "../film_developer/processes/c41_process.hpp"
#include "../film_developer/movement/movement_loader.hpp"
#include <iostream>

#ifdef HOST
// Mock implementation of MotorController for testing
class TestMotorController : public MotorController {
public:
    void clockwise(bool enable) override {
        (void)enable; // Suppress unused parameter warning
        DEBUG_PRINT("Motor CW: %s", enable ? "ON" : "OFF");
    }

    void counterClockwise(bool enable) override {
        (void)enable; // Suppress unused parameter warning
        DEBUG_PRINT("Motor CCW: %s", enable ? "ON" : "OFF");
    }

    void stop() override {
        DEBUG_PRINT("Motor: STOP");
    }
};
#endif

void print_c41_process() {
    DEBUG_PRINT("\nC41 Process Overview:");
    DEBUG_PRINT("Name: %s", C41_FULL_PROCESS_STATIC.process_name);
    DEBUG_PRINT("Film Type: %s", C41_FULL_PROCESS_STATIC.film_type);
    DEBUG_PRINT("Chemistry: %s", C41_FULL_PROCESS_STATIC.chemistry);
    DEBUG_PRINT("Temperature: %.1f°C", C41_FULL_PROCESS_STATIC.temperature);

    // Create factory and loader instances
    MovementFactory factory;
    MovementLoader loader(factory);

    DEBUG_PRINT("\nSteps:");
    for(size_t i = 0; i < C41_FULL_PROCESS_STATIC.steps_length; i++) {
        const auto& step = C41_FULL_PROCESS_STATIC.steps[i];
        DEBUG_PRINT("\nStep %zu: %s", i + 1, step.name);
        DEBUG_PRINT("Description: %s", step.description);
        DEBUG_PRINT("Temperature: %.1f°C", step.temperature);

        // Create array to store the sequence
        AgitationMovement* sequence[MovementLoader::MAX_SEQUENCE_LENGTH];

        // Load and print the movement sequence
        size_t length = loader.loadSequence(step.sequence, step.sequence_length, sequence);

        DEBUG_PRINT("Loaded main sequence length: %zu", length);

        DEBUG_PRINT("");
        DEBUG_PRINT("");
        DEBUG_PRINT("");

        DEBUG_PRINT("Movements:");
        for(size_t j = 0; j < length; j++) {
            if(sequence[j]) {
                sequence[j]->print();
            }
        }

        DEBUG_PRINT("---");
        DEBUG_PRINT("");
    }
}

int main() {
    print_c41_process();
    return 0;
}
