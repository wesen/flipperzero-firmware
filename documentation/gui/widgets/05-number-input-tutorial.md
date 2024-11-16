# Tutorial: Using the Number Input Widget in Flipper Zero

## Overview

The Number Input widget provides a user interface for entering and editing integer values. It's particularly useful for:

- Setting numeric parameters
- Entering PIN codes or frequencies
- Configuring time or date values
- Input validation with min/max bounds

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment
- Understanding of basic GUI concepts

## Key Features

1. Integer value input with min/max bounds
2. Callback on value change
3. Integration with ViewDispatcher
4. Support for negative numbers
5. Built-in validation

## Basic Implementation

### Header Inclusion

```c
#include <gui/modules/number_input.h>
```

### Structure Definition

```c
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NumberInput* number_input;
    int32_t result;
} MyApp;

// View IDs for ViewDispatcher
typedef enum {
    MyAppViewNumberInput,
    // ... other views
} MyAppView;
```

### Callback Definition

```c
static void number_input_callback(void* context, int32_t number) {
    MyApp* app = context;
    app->result = number;
    // Handle the entered number
    // For example, switch back to previous view:
    view_dispatcher_switch_to_view(app->view_dispatcher, PreviousViewId);
}
```

### Initialization

```c
MyApp* app = malloc(sizeof(MyApp));

// Initialize GUI and ViewDispatcher
app->gui = furi_record_open(RECORD_GUI);
app->view_dispatcher = view_dispatcher_alloc();
view_dispatcher_enable_queue(app->view_dispatcher);
view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

// Initialize Number Input
app->number_input = number_input_alloc();

// Add view to ViewDispatcher
view_dispatcher_add_view(
    app->view_dispatcher,
    MyAppViewNumberInput,
    number_input_get_view(app->number_input));

// Configure Number Input
number_input_set_result_callback(
    app->number_input,
    number_input_callback,  // Callback function
    app,                   // Context
    5,                     // Initial value
    -100,                  // Minimum value
    100,                   // Maximum value
    "Enter Number",        // Header text
    "Value");             // Unit text
```

### Cleanup

```c
void my_app_free(MyApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, MyAppViewNumberInput);
    number_input_free(app->number_input);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
}
```

## Example Application

Here's a complete example that creates a number input for setting a frequency:

```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/number_input.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NumberInput* number_input;
    int32_t frequency;
} FrequencyApp;

typedef enum {
    FrequencyViewNumberInput,
} FrequencyView;

static void frequency_input_callback(void* context, int32_t number) {
    FrequencyApp* app = context;
    app->frequency = number;
    FURI_LOG_I("Frequency", "Set to: %ld Hz", app->frequency);
}

static bool frequency_input_back_event(void* context) {
    UNUSED(context);
    return false;  // Allow default back behavior
}

FrequencyApp* frequency_app_alloc() {
    FrequencyApp* app = malloc(sizeof(FrequencyApp));
    
    // GUI
    app->gui = furi_record_open(RECORD_GUI);
    
    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Number Input
    app->number_input = number_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FrequencyViewNumberInput,
        number_input_get_view(app->number_input));
    
    // Configure Number Input
    number_input_set_result_callback(
        app->number_input,
        frequency_input_callback,
        app,
        433920000,  // Default 433.92 MHz
        300000000,  // Min 300 MHz
        900000000,  // Max 900 MHz
        "Enter Frequency",
        "Hz");
    
    view_set_previous_callback(
        number_input_get_view(app->number_input),
        frequency_input_back_event);
    
    return app;
}

void frequency_app_free(FrequencyApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, FrequencyViewNumberInput);
    number_input_free(app->number_input);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t frequency_app(void* p) {
    UNUSED(p);
    FrequencyApp* app = frequency_app_alloc();
    
    view_dispatcher_switch_to_view(app->view_dispatcher, FrequencyViewNumberInput);
    view_dispatcher_run(app->view_dispatcher);
    
    frequency_app_free(app);
    return 0;
}
```

## Best Practices

1. Always set min/max values to prevent invalid input
2. Provide clear header and unit text for context
3. Handle the back event appropriately
4. Free resources in the correct order
5. Use meaningful variable names and comments

## Common Use Cases

1. **Frequency Input**: Setting radio frequencies with Hz units
2. **Time Settings**: Entering minutes or seconds for timers
3. **PIN Entry**: Secure number input with masked display
4. **Configuration**: Setting parameters with numeric bounds

## Advanced Features

1. **Custom Validation**: Add additional validation in the callback
2. **Dynamic Bounds**: Change min/max values based on context
3. **Unit Display**: Show appropriate units for the value
4. **State Management**: Save/restore input state

## Troubleshooting

1. **Input Not Showing**: Ensure view is added to ViewDispatcher
2. **Callback Not Firing**: Check callback registration
3. **Value Out of Bounds**: Verify min/max settings
4. **Memory Leaks**: Confirm proper cleanup sequence
