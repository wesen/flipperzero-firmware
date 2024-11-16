# Tutorial: Using the Number Input Widget in Flipper Zero

## Overview

The Number Input widget provides a user interface for entering and editing numeric values. It's particularly useful for:

- Entering PIN codes
- Setting numeric values
- Inputting frequencies
- Configuring numeric parameters
- Creating number-based games

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/number_input.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Number Input Structure

```c
typedef struct {
    NumberInput* number_input;
    ViewDispatcher* view_dispatcher;
    // Add your application-specific state here
    uint32_t result;
} MyApp;

// Callback for when the number input is completed
static void number_input_callback(void* context) {
    MyApp* app = context;
    // Handle the entered number
    app->result = number_input_get_value(app->number_input);
    // Switch back to previous view or handle the result
}
```

## Step 3: Initialize the Number Input

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate the number input
app->number_input = number_input_alloc();

// Configure the number input
number_input_set_header_text(app->number_input, "Enter Number");

// Set input parameters
number_input_set_min_max(app->number_input, 0, 9999);  // Set range
number_input_set_step(app->number_input, 1);           // Set increment step
number_input_set_value(app->number_input, 0);          // Set initial value

// Set callback
number_input_set_callback(
    app->number_input,
    number_input_callback,
    app
);
```

## Step 4: Integration with ViewDispatcher

```c
View* view = number_input_get_view(app->number_input);
view_dispatcher_add_view(view_dispatcher, MyNumberInputView, view);
```

## Example Application

Here's a complete example that creates a frequency input:

```c
typedef struct {
    NumberInput* number_input;
    ViewDispatcher* view_dispatcher;
    uint32_t frequency;
} FrequencyApp;

static void frequency_input_callback(void* context) {
    FrequencyApp* app = context;
    app->frequency = number_input_get_value(app->number_input);
    FURI_LOG_I("Frequency", "Set to: %lu Hz", app->frequency);
    
    // Here you would typically handle the frequency value
    // For example, configure a radio module or save the setting
}

int32_t frequency_app(void* p) {
    UNUSED(p);
    FrequencyApp* app = malloc(sizeof(FrequencyApp));

    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate number input
    app->number_input = number_input_alloc();
    
    // Configure number input
    number_input_set_header_text(app->number_input, "Frequency (Hz)");
    number_input_set_min_max(app->number_input, 1, 999999);  // 1 Hz to 999.999 kHz
    number_input_set_step(app->number_input, 1000);          // 1 kHz steps
    number_input_set_value(app->number_input, 433920);       // Default to 433.92 MHz
    
    // Set callback
    number_input_set_callback(app->number_input, frequency_input_callback, app);
    
    // Get view and add to dispatcher
    View* view = number_input_get_view(app->number_input);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    number_input_free(app->number_input);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
    
    return 0;
}
```

## Key Features

- Configurable numeric range
- Adjustable step size
- Header text customization
- Callback support
- Value validation
- Up/Down navigation
- Direct number entry

## Common Customization Options

1. **Set Range:**
```c
number_input_set_min_max(number_input, min_value, max_value);
```

2. **Set Step Size:**
```c
number_input_set_step(number_input, step_value);
```

3. **Set Header Text:**
```c
number_input_set_header_text(number_input, "Enter Value");
```

4. **Set Initial Value:**
```c
number_input_set_value(number_input, initial_value);
```

## Best Practices

1. Set appropriate min/max ranges
2. Use logical step sizes
3. Provide clear header text
4. Handle all input callbacks
5. Validate input values
6. Clean up resources when done
7. Consider user input constraints

## Common Use Cases

1. **PIN Entry:**
```c
// Configure for 4-digit PIN
number_input_set_min_max(number_input, 0, 9999);
number_input_set_step(number_input, 1);
number_input_set_header_text(number_input, "Enter PIN");
```

2. **Frequency Input:**
```c
// Configure for frequency in kHz
number_input_set_min_max(number_input, 1, 999999);
number_input_set_step(number_input, 1000);
number_input_set_header_text(number_input, "Frequency (kHz)");
```

3. **Value Selector:**
```c
// Configure for selecting a value from 1-100
number_input_set_min_max(number_input, 1, 100);
number_input_set_step(number_input, 1);
number_input_set_header_text(number_input, "Select Value");
```

## Common Issues and Solutions

1. **Value out of range:**
   - Verify min/max settings
   - Check step size
   - Ensure initial value is within range

2. **Step size issues:**
   - Verify step size is appropriate for range
   - Check if step size divides evenly into range
   - Consider user input requirements

3. **Callback not firing:**
   - Verify callback is properly set
   - Check context pointer
   - Ensure view is properly added to dispatcher

## Exercises

1. Create a PIN entry system
2. Build a frequency selector
3. Implement a game score input
4. Create a temperature setting interface
5. Build a time input (hours/minutes)

## Advanced Usage

1. **Custom Value Formatting:**
```c
static void format_value(NumberInput* input, uint32_t value) {
    char str[32];
    snprintf(str, sizeof(str), "%lu.%lu MHz", value / 1000, value % 1000);
    number_input_set_header_text(input, str);
}
```

2. **Range Validation:**
```c
static bool validate_frequency(uint32_t freq) {
    // Check if frequency is within valid bands
    return (freq >= 433000 && freq <= 434000) ||
           (freq >= 868000 && freq <= 869000);
}
```

3. **Step Size Adjustment:**
```c
static void adjust_step_size(NumberInput* input, uint32_t value) {
    if(value < 1000) {
        number_input_set_step(input, 1);        // 1 Hz steps
    } else if(value < 10000) {
        number_input_set_step(input, 10);       // 10 Hz steps
    } else {
        number_input_set_step(input, 1000);     // 1 kHz steps
    }
}
```

## Tips for Better User Experience

1. Use appropriate step sizes for the value range
2. Provide clear feedback through header text
3. Consider adding unit labels
4. Implement value validation
5. Use sensible default values
6. Add range indicators in header
7. Consider user input speed vs precision
