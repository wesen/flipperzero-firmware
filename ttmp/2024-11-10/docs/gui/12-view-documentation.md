# Flipper Zero View System Documentation

## Introduction

The View system is a fundamental building block of the Flipper Zero's graphical user interface framework. A View represents a self-contained UI component that manages its own drawing, input handling, and state management. Views are designed to be modular, reusable, and thread-safe, making them ideal for creating complex user interfaces while maintaining clean separation of concerns.

This document provides a comprehensive guide to understanding and implementing Views in the Flipper Zero firmware.

## View Architecture

A View consists of several key components working together:

1. Core View Structure - Manages callbacks and configuration
2. View Model - Handles data storage and state
3. Drawing System - Renders content to the display
4. Input Handling - Processes user interactions
5. Navigation - Manages view transitions

The View system is designed around a callback-based architecture, allowing applications to respond to various events while maintaining a clean separation between the UI infrastructure and application logic.

### View Structure

At its core, a View is represented by the following structure:

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

Each field serves a specific purpose in the View's operation:

- Callbacks (`draw_callback`, `input_callback`, etc.) - Function pointers for handling various events
- Model management (`model_type`, `model`) - Controls data storage and access patterns
- Navigation (`previous_callback`) - Handles view transition logic
- State management (`enter_callback`, `exit_callback`) - Manages view lifecycle
- Display control (`orientation`) - Configures view presentation

## Creating and Managing Views

### View Initialization

To create a new View, use the following pattern:

```c
View* view = view_alloc();
view_allocate_model(view, ViewModelTypeLocking, sizeof(MyModel));
view_set_draw_callback(view, my_draw_callback);
view_set_input_callback(view, my_input_callback);
```

The initialization process involves:

1. Allocating the View structure
2. Configuring the model type and size
3. Setting up necessary callbacks
4. Configuring any additional parameters

### View Models

Views support two types of model management:

1. Lock-Free Models (`ViewModelTypeLockFree`):

   - Suitable for simple data structures
   - No thread synchronization overhead
   - Used when partial updates are acceptable

2. Locking Models (`ViewModelTypeLocking`):
   - Protected by a mutex
   - Ensures thread-safe access
   - Required for complex data structures

Model access is managed through the `with_view_model` macro:

```c
with_view_model(
    view,
    MyModelType* model,
    {
        // Access and modify model data
    },
    true  // Update view after modification
);
```

This pattern ensures proper locking and unlocking of the model while providing a clean syntax for data access.

## View Callbacks

### Draw Callback

The draw callback is responsible for rendering the View's content:

```c
static void my_draw_callback(Canvas* canvas, void* model) {
    MyModelType* my_model = model;
    // Render content using canvas_* functions
}
```

Drawing operations should:

- Be efficient and minimize calculations
- Use appropriate canvas functions
- Consider the current orientation
- Handle different model states

### Input Callback

Input callbacks handle user interactions:

```c
static bool my_input_callback(InputEvent* event, void* context) {
    MyContext* my_context = context;
    bool handled = false;

    if(event->type == InputTypeShort) {
        // Handle input
        handled = true;
    }

    return handled;
}
```

Input handling should:

- Return true if the input was handled
- Consider different input types
- Update the model as needed
- Trigger view updates when required

## Navigation and Orientation

### View Navigation

Views can implement navigation logic through the previous callback:

```c
static uint32_t my_previous_callback(void* context) {
    // Return the view ID to navigate to, or:
    // VIEW_NONE - to hide
    // VIEW_IGNORE - to ignore navigation
    return MyPreviousViewId;
}
```

### Orientation Management

Views support four orientation modes:

- `ViewOrientationHorizontal` - Standard horizontal layout
- `ViewOrientationHorizontalFlip` - Flipped horizontal layout
- `ViewOrientationVertical` - Rotated for vertical display
- `ViewOrientationVerticalFlip` - Flipped vertical layout

Set the orientation using:

```c
view_set_orientation(view, ViewOrientationHorizontal);
```

## Best Practices

1. Model Management

   - Choose appropriate model type based on complexity
   - Keep models focused and minimal
   - Use proper locking strategies

2. Drawing Operations

   - Optimize drawing routines
   - Cache calculations where possible
   - Consider all orientation modes

3. Input Handling

   - Handle all relevant input types
   - Provide clear feedback
   - Maintain consistent behavior

4. Memory Management
   - Free views when no longer needed
   - Clean up models properly
   - Handle context cleanup

## Common Pitfalls

1. Model Access

   - Accessing models outside of `with_view_model`
   - Not handling thread safety properly
   - Forgetting to update the view

2. Callback Management

   - Not cleaning up callbacks
   - Incorrect context handling
   - Missing error handling

3. Resource Management
   - Memory leaks in models
   - Dangling pointers in callbacks
   - Incomplete cleanup

## Conclusion

The View system provides a robust foundation for building user interfaces on the Flipper Zero. By understanding its architecture and following best practices, developers can create efficient, maintainable, and thread-safe UI components.

Key takeaways:

- Views provide a complete UI component framework
- Model management ensures thread safety
- Callback system enables clean separation of concerns
- Proper resource management is critical

When implementing Views, focus on:

- Choosing appropriate model types
- Implementing efficient drawing routines
- Handling input comprehensively
- Managing resources properly
