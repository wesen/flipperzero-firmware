#pragma once

#include <inttypes.h>

// Custom events for the application
enum class CustomEvent : uint32_t {
    ToggleScene = 0,
};

struct ViewModel {
    int counter{0};
    const char* message{"Hello from Second View"};
};
