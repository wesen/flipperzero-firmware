# Tutorial: Using the Text Input Widget in Flipper Zero

## Overview

The Text Input widget provides a user interface for entering and editing text. It's particularly useful for:

- Entering names and labels
- Creating text-based configurations
- Building text editors
- Inputting passwords or usernames
- Creating text-based games

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/text_input.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Text Input Structure

```c
typedef struct {
    TextInput* text_input;
    ViewDispatcher* view_dispatcher;
    // Buffer to store the input text
    char* text_buffer;
    uint8_t text_buffer_size;
} MyApp;

// Callback for when text input is completed
static void text_input_callback(void* context) {
    MyApp* app = context;
    // Handle the entered text (stored in app->text_buffer)
    // Switch back to previous view or handle the result
}
```

## Step 3: Initialize the Text Input

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate text buffer
app->text_buffer_size = 64;
app->text_buffer = malloc(app->text_buffer_size);
memset(app->text_buffer, 0, app->text_buffer_size);

// Allocate the text input
app->text_input = text_input_alloc();

// Configure the text input
text_input_set_header_text(app->text_input, "Enter Text");

// Set input parameters
text_input_set_result_callback(
    app->text_input,
    text_input_callback,     // Callback
    app,                     // Context
    app->text_buffer,        // Buffer
    app->text_buffer_size,   // Buffer size
    true                     // Clear default text
);
```

## Step 4: Integration with ViewDispatcher

```c
View* view = text_input_get_view(app->text_input);
view_dispatcher_add_view(view_dispatcher, MyTextInputView, view);
```

## Example Application

Here's a complete example that creates a name input system:

```c
typedef struct {
    TextInput* text_input;
    ViewDispatcher* view_dispatcher;
    char* name_buffer;
    uint8_t name_buffer_size;
} NameInputApp;

static void name_input_callback(void* context) {
    NameInputApp* app = context;
    FURI_LOG_I("Name", "Entered: %s", app->name_buffer);
    
    // Here you would typically handle the entered name
    // For example, save it to storage or use it in the app
    
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);  // Switch back to main view
}

int32_t name_input_app(void* p) {
    UNUSED(p);
    NameInputApp* app = malloc(sizeof(NameInputApp));

    // Allocate name buffer
    app->name_buffer_size = 64;
    app->name_buffer = malloc(app->name_buffer_size);
    memset(app->name_buffer, 0, app->name_buffer_size);
    
    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate text input
    app->text_input = text_input_alloc();
    
    // Configure text input
    text_input_set_header_text(app->text_input, "Enter Name");
    
    // Set allowed input characters (optional)
    text_input_set_validator(
        app->text_input,
        // Allow letters, numbers, and some special chars
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -_"
    );
    
    // Set result callback
    text_input_set_result_callback(
        app->text_input,
        name_input_callback,
        app,
        app->name_buffer,
        app->name_buffer_size,
        true  // Clear default text
    );
    
    // Get view and add to dispatcher
    View* view = text_input_get_view(app->text_input);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    text_input_free(app->text_input);
    view_dispatcher_free(app->view_dispatcher);
    free(app->name_buffer);
    free(app);
    
    return 0;
}
```

## Key Features

- Customizable character set
- Header text customization
- Input validation
- Callback support
- Text buffer management
- Clear text option
- Multi-character support

## Common Customization Options

1. **Set Header Text:**
```c
text_input_set_header_text(text_input, "Enter Text");
```

2. **Set Allowed Characters:**
```c
text_input_set_validator(text_input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
```

3. **Set Result Callback:**
```c
text_input_set_result_callback(
    text_input,
    callback,
    context,
    buffer,
    buffer_size,
    clear_default
);
```

## Best Practices

1. Always allocate sufficient buffer size
2. Set appropriate character validators
3. Provide clear header text
4. Handle all input callbacks
5. Clean up resources when done
6. Validate input length
7. Consider user input constraints

## Common Use Cases

1. **Name Input:**
```c
// Configure for name entry
text_input_set_header_text(text_input, "Enter Name");
text_input_set_validator(text_input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ");
```

2. **Password Input:**
```c
// Configure for password entry
text_input_set_header_text(text_input, "Enter Password");
text_input_set_validator(text_input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*");
```

3. **Filename Input:**
```c
// Configure for filename entry
text_input_set_header_text(text_input, "Save As");
text_input_set_validator(text_input, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.");
```

## Common Issues and Solutions

1. **Text not being saved:**
   - Verify buffer size is sufficient
   - Check callback is properly set
   - Ensure buffer is properly allocated

2. **Invalid characters:**
   - Check validator string
   - Verify character set is appropriate
   - Consider input requirements

3. **Callback issues:**
   - Verify callback function is set
   - Check context pointer
   - Ensure view is properly added to dispatcher

## Exercises

1. Create a username input system
2. Build a password entry form
3. Implement a text-based game input
4. Create a filename input dialog
5. Build a multi-field text form

## Advanced Usage

1. **Custom Character Sets:**
```c
// Define custom character sets for different input types
static const char* ALPHA_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const char* NUM_CHARS = "0123456789";
static const char* SPECIAL_CHARS = "!@#$%^&*()-_+=";

// Combine sets based on needs
void set_input_mode(TextInput* input, InputMode mode) {
    char validator[128];
    memset(validator, 0, sizeof(validator));
    
    switch(mode) {
        case InputModeAlpha:
            strncpy(validator, ALPHA_CHARS, sizeof(validator)-1);
            break;
        case InputModeAlphaNum:
            snprintf(validator, sizeof(validator), "%s%s", ALPHA_CHARS, NUM_CHARS);
            break;
        case InputModeAll:
            snprintf(validator, sizeof(validator), "%s%s%s", 
                    ALPHA_CHARS, NUM_CHARS, SPECIAL_CHARS);
            break;
    }
    
    text_input_set_validator(input, validator);
}
```

2. **Input Validation:**
```c
static bool validate_password(const char* password) {
    bool has_upper = false;
    bool has_lower = false;
    bool has_number = false;
    
    for(size_t i = 0; password[i] != '\0'; i++) {
        if(isupper(password[i])) has_upper = true;
        if(islower(password[i])) has_lower = true;
        if(isdigit(password[i])) has_number = true;
    }
    
    return has_upper && has_lower && has_number;
}
```

3. **Dynamic Header Text:**
```c
static void update_header(TextInput* input, const char* buffer) {
    char header[32];
    snprintf(header, sizeof(header), "Enter Text (%lu/64)", strlen(buffer));
    text_input_set_header_text(input, header);
}
```

## Tips for Better User Experience

1. Use clear and concise header text
2. Provide appropriate character sets
3. Show input constraints in header
4. Implement input validation
5. Consider buffer size limits
6. Use meaningful default text when appropriate
7. Provide visual feedback for invalid input
