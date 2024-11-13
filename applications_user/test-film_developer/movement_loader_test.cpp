#include "../film_developer/movement/movement_loader.hpp"
#include "../film_developer/processes/c41_process.hpp"
#include "mock_controller.hpp"
#include <cassert>

void test_load_c41_sequence() {
    DEBUG_PRINT("\nTesting C41 sequence loading and execution...");

    // Reset the movement factory
    MovementFactory::reset();

    // Load the C41 color developer sequence
    auto [sequence, length] =
        MovementLoader::loadSequence(C41_COLOR_DEVELOPER, C41_COLOR_DEVELOPER_LENGTH);

    // Verify sequence was loaded
    assert(sequence != nullptr);
    assert(length == C41_COLOR_DEVELOPER_LENGTH);
    DEBUG_PRINT("Sequence loaded successfully, length: %zu", length);

    // Create mock controller for testing
    MockController motor;
    assert(motor.isStopped());

    // Execute first movement (should be a loop)
    assert(sequence[0]->getType() == AgitationMovement::Type::Loop);
    DEBUG_PRINT("\nExecuting sequence:");

    // Execute sequence
    uint32_t tick_count = 0;
    while(!sequence[0]->isComplete() && tick_count < 1000) { // Safety limit
        sequence[0]->execute(motor);
        tick_count++;

        // Verify motor state consistency
        assert(!(motor.isClockwise() && motor.isCounterClockwise()));

        // In real code we'd wait 1 second between ticks
        if(tick_count % 10 == 0) {
            DEBUG_PRINT(
                "Tick %u: %s",
                tick_count,
                motor.isClockwise()        ? "CW" :
                motor.isCounterClockwise() ? "CCW" :
                                             "STOPPED");
        }
    }

    DEBUG_PRINT("Sequence completed after %u ticks", tick_count);
    assert(motor.isStopped());

    // Clean up
    MovementFactory::reset();
    DEBUG_PRINT("Test completed successfully\n");
}

int main() {
    test_load_c41_sequence();
    return 0;
}
