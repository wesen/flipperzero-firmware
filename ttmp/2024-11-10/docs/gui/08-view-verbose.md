# Flipper Zero View System Documentation

## Overview

The View system represents the foundation of sophisticated UI components in the Flipper Zero. While ViewPort handles raw drawing and input, View introduces a higher abstraction layer that implements the Model-View pattern. This separation of concerns allows developers to focus on their application's data and logic (the Model) separately from its presentation (the View). The View system also provides thread-safe state management and standardized lifecycle events, making it easier to create complex, maintainable UIs.

## View Architecture

### Core Structure

The View structure is designed around the principle of separation of concerns. Each View maintains its own model (data), callbacks for various events, and orientation settings. This architecture allows Views to be self-contained modules that can be easily managed by higher-level components like ViewDispatcher.

```c
struct View {
    ViewDrawCallback draw_callback;
    ViewInputCallback input_callback;
    ViewCustomCallback custom_callback;

    ViewModelType model_type;
    ViewNavigationCallback previous_callback;
    ViewCallback enter_callback;
    ViewCallback exit_callback;
    ViewOrientation orientation;

    ViewUpdateCallback update_callback;
    void* update_callback_context;

    void* model;
    void* context;
};
```

### Model Types

The View system provides different model types to handle thread safety appropriately. ViewModelTypeLockFree is suitable for simple atomic operations, while ViewModelTypeLocking provides mutex protection for complex data structures. This design allows developers to choose the appropriate level of thread safety based on their needs.

```c
typedef enum {
    ViewModelTypeNone,      // No model allocated
    ViewModelTypeLockFree,  // Lock-free model for atomic operations
    ViewModelTypeLocking,   // Thread-safe model with mutex protection
} ViewModelType;
```

## Using View

### 1. Basic Setup

The basic setup demonstrates the fundamental pattern of View initialization. When creating a View, you first allocate the View itself, then define and allocate a model to hold your application's state. The model should contain all the data needed to render your UI and maintain your application's state. This separation ensures clean architecture and makes your code easier to maintain and test.

```c
// Allocate View
View* view = view_alloc();

// Define model structure
typedef struct {
    int32_t counter;
    char* text;
    bool active;
} MyViewModel;

// Allocate model
view_allocate_model(view, ViewModelTypeLocking, sizeof(MyViewModel));

// Set callbacks
view_set_draw_callback(view, my_draw_callback);
view_set_input_callback(view, my_input_callback);
```

### 2. Model Management

Proper model management is crucial for preventing memory leaks and ensuring clean application lifecycle. The model structure should be designed to handle all necessary data cleanup when the view is destroyed. This example shows how to properly manage complex data types within your model, including string allocation and cleanup.

```c
// Define model structure with proper memory management
typedef struct {
    FuriString* text;      // Dynamic string that needs proper cleanup
    uint32_t counter;      // Simple type that doesn't need cleanup
    bool active;           // Simple type that doesn't need cleanup
} MyViewModel;

// Initialize model with proper allocation
static MyViewModel* model_alloc() {
    MyViewModel* model = malloc(sizeof(MyViewModel));
    model->text = furi_string_alloc();  // Allocate string
    model->counter = 0;
    model->active = false;
    return model;
}

// Cleanup model to prevent memory leaks
static void model_free(MyViewModel* model) {
    furi_string_free(model->text);  // Free string
    free(model);
}
```

### 3. Thread-Safe Model Access

The View system provides a powerful macro `with_view_model` that ensures thread-safe access to your model. This pattern is essential for preventing race conditions when multiple parts of your application might try to access the model simultaneously. The last parameter determines whether the view should be updated after the model changes.

```c
// Using with_view_model macro for safe model access
static void update_model(View* view) {
    // The with_view_model macro handles locking and unlocking automatically
    with_view_model(
        view,
        MyViewModel * model,
        {
            // This code block executes with exclusive access to the model
            model->counter++;
            furi_string_printf(model->text, "Count: %ld", model->counter);
        },
        true);  // Request a view update after modification
}
```

### 4. Drawing Implementation

The draw callback is where you implement the visual representation of your model. This function should be pure rendering code - it should not modify the model or handle any logic. The model is provided as a parameter, and you use the Canvas API to render its current state. This separation ensures that your UI remains responsive and predictable.

```c
static void my_draw_callback(Canvas* canvas, void* model) {
    MyViewModel* my_model = model;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    // Draw model data - pure rendering, no logic
    canvas_draw_str(
        canvas,
        0,
        10,
        furi_string_get_cstr(my_model->text));

    if(my_model->active) {
        canvas_draw_box(canvas, 0, 20, 10, 10);
    }
}
```

### 5. Input Handling

Input handling in the View system follows an event-driven pattern. The input callback receives input events and should handle them by modifying the model as needed. The return value indicates whether the input was consumed (handled) by this view. This pattern allows for event bubbling when views are nested.

```c
static bool my_input_callback(InputEvent* event, void* context) {
    View* view = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        with_view_model(
            view,
            MyViewModel * model,
            {
                switch(event->key) {
                    case InputKeyUp:
                        model->counter++;
                        consumed = true;
                        break;
                    case InputKeyDown:
                        model->counter--;
                        consumed = true;
                        break;
                    case InputKeyOk:
                        model->active = !model->active;
                        consumed = true;
                        break;
                }
            },
            consumed);  // Only update if we consumed the event
    }

    return consumed;
}
```

### 6. Navigation Handling
Navigation in the View system is built around the concept of a view hierarchy. When the user presses the back button, the navigation callback determines where the application should go next. This enables complex navigation flows while maintaining a consistent user experience. The callback can return special values (VIEW_NONE, VIEW_IGNORE) to handle edge cases or return a specific view ID to navigate to that view.

```c
// Navigation callback for handling "back" actions
static uint32_t my_navigation_callback(void* context) {
    // Return values determine navigation behavior:
    // - Specific view ID: navigate to that view
    // - VIEW_NONE: hide viewport
    // - VIEW_IGNORE: ignore navigation request
    return MyPreviousViewId;
}

// Set navigation callback
view_set_previous_callback(view, my_navigation_callback);
```

### 7. Lifecycle Management
View lifecycle management is crucial for proper resource management and state maintenance. The enter callback is called when a view becomes active, allowing you to initialize or refresh state. The exit callback is called when leaving a view, enabling cleanup or state saving. This pattern is similar to onCreate/onDestroy in other UI frameworks.

```c
// Enter callback - initialize or refresh view state
static void my_enter_callback(void* context) {
    View* view = context;
    with_view_model(
        view,
        MyViewModel * model,
        {
            // Reset or initialize state when view becomes active
            model->counter = 0;
            model->active = false;
        },
        true);  // Update display after initialization
}

// Exit callback - cleanup or save state
static void my_exit_callback(void* context) {
    View* view = context;
    with_view_model(
        view,
        MyViewModel * model,
        {
            // Save state or perform cleanup
            model->active = false;
        },
        false);  // No need to update display when exiting
}

// Register lifecycle callbacks
view_set_enter_callback(view, my_enter_callback);
view_set_exit_callback(view, my_exit_callback);
```

## Advanced Features

### 1. Custom Events
Custom events provide a mechanism for handling application-specific events beyond basic input. This pattern enables communication between different parts of your application while maintaining the View system's thread safety. Each custom event can carry an event code and be handled appropriately by the view.

```c
typedef enum {
    MyCustomEventRefresh,
    MyCustomEventTimeout,
    MyCustomEventData
} MyCustomEvent;

static bool my_custom_callback(uint32_t event, void* context) {
    View* view = context;
    bool handled = false;
    
    switch(event) {
        case MyCustomEventRefresh:
            with_view_model(
                view,
                MyViewModel * model,
                {
                    // Handle refresh event safely within model context
                    model->counter = 0;
                },
                true);
            handled = true;
            break;
    }
    
    return handled;
}

// Register custom event handler
view_set_custom_callback(view, my_custom_callback);
```

### 2. Update Callback
The update callback system provides a way to respond to model changes. When the model is modified with the update flag set to true, this callback is triggered. This pattern is useful for implementing animations, updating related views, or triggering side effects when the model changes.

```c
static void my_update_callback(View* view, void* context) {
    // Called whenever model is updated with update flag true
    MyApp* app = context;
    
    // Trigger view_port update or perform other necessary updates
    view_port_update(app->view_port);
}

// Register update callback system
view_set_update_callback(view, my_update_callback);
view_set_update_callback_context(view, app);
```

### 3. Icon Animation
Icon animation integration demonstrates how the View system can be extended with additional features. The animation system automatically triggers view updates when frames change, providing smooth animations without manual frame management.

```c
// Initialize and connect animation system
IconAnimation* animation = icon_animation_alloc();
view_tie_icon_animation(view, animation);

// Animation updates will automatically trigger view updates
```

## Best Practices

### 1. Model Management
Proper model management is essential for preventing memory leaks and maintaining application stability. This example demonstrates the correct pattern for allocating and cleaning up model resources throughout the view lifecycle.

```c
// Comprehensive model cleanup
static void view_free_model(MyViewModel* model) {
    furi_string_free(model->text);
    // Free any other dynamically allocated resources
}

void my_view_free(View* view) {
    with_view_model(
        view,
        MyViewModel * model,
        {
            view_free_model(model);
        },
        false);  // No need to update view during cleanup
    view_free(view);
}
```

### 2. Thread Safety
Thread safety is crucial in embedded systems where multiple threads might access the same resources. The View system's thread safety mechanisms help prevent race conditions and ensure data consistency.

```c
// Example of thread-safe state updates
static void update_state(View* view, int value) {
    with_view_model(
        view,
        MyViewModel * model,
        {
            // All model access is automatically protected
            model->counter = value;
            furi_string_printf(model->text, "Value: %d", value);
        },
        true);  // Trigger update after state change
}
```

### 3. Resource Management
Proper resource management ensures your application doesn't leak memory or resources. This example shows the correct cleanup sequence, ensuring all resources are properly freed in the correct order.

```c
// Complete cleanup sequence
void cleanup_view(View* view) {
    // First unregister callbacks to prevent any new updates
    view_set_draw_callback(view, NULL);
    view_set_input_callback(view, NULL);
    
    // Then clean up model resources
    with_view_model(
        view,
        MyViewModel * model,
        {
            view_free_model(model);
        },
        false);
        
    // Finally free the view itself
    view_free(view);
}
```

## Key Principles Summary

1. **Separation of Concerns**: Keep model data separate from view logic
2. **Thread Safety**: Always use provided thread-safe mechanisms for model access
3. **Resource Management**: Clean up resources in the correct order
4. **Event Handling**: Use appropriate callbacks for different types of events
5. **Lifecycle Management**: Properly handle view entry and exit
6. **State Management**: Maintain clean state transitions and updates

These principles, when followed correctly, enable the creation of robust, maintainable UI applications on the Flipper Zero platform.