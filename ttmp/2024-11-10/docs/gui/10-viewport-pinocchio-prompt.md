<technical_document_outline>

1. Key Components Analysis:

- ViewPort is a fundamental GUI component in Flipper Zero
- Handles screen drawing and input management
- Supports different orientations and input mapping
- Thread-safe design with mutex protection
- Callback-based architecture for drawing and input
- Part of larger GUI system with layering capabilities

2. Target Audience:

- Firmware developers working on Flipper Zero applications
- Expected to have C programming knowledge
- Familiar with embedded systems concepts
- Understanding of basic GUI programming concepts

3. Important Focus Areas:

- ViewPort lifecycle management
- Drawing and input callback system
- Thread safety considerations
- Screen orientation handling
- Integration with GUI system
- Memory management

4. Document Structure:

- Overview and Architecture
- ViewPort Lifecycle
- Drawing System
- Input Handling
- Orientation Management
- Thread Safety
- Integration with GUI
- Best Practices
- Common Patterns

5. Specialized Terms:

- ViewPort
- Canvas
- InputEvent
- GUI Layer
- ViewPortOrientation
- Mutex
- Callback

6. Code Examples:

- Basic ViewPort creation and setup
- Drawing callback implementation
- Input handling
- Orientation changes
- GUI integration

7. Explanations for Code:

- Purpose of each component
- Memory management considerations
- Thread safety implications
- Common pitfalls to avoid

8. Key Conclusions:

- Best practices for ViewPort usage
- Performance considerations
- Integration patterns
- Common mistakes to avoid

9. Potential Challenges:

- Thread safety complexities
- Memory management
- Callback timing
- Orientation mapping understanding

10. Reader Questions:

- When to use ViewPort vs other GUI components
- How to handle complex drawing needs
- Input processing flow
- Memory management responsibilities

11. Real-world Applications:

- Simple display application
- Input handling example
- Screen orientation handling

12. Additional Research:

- ViewPort performance characteristics
- Memory usage patterns
- Callback timing details
  </technical_document_outline>

# Flipper Zero ViewPort System Developer Guide

## Introduction

The ViewPort system is a fundamental component of the Flipper Zero's GUI framework, providing a flexible and efficient way to manage screen drawing and input handling. It serves as the basic building block for creating interactive user interfaces, offering thread-safe operation and support for different screen orientations.

This technical document provides a comprehensive exploration of the ViewPort system, its architecture, and proper usage patterns for Flipper Zero application development.

## Architecture Overview

The ViewPort system is built around the `ViewPort` structure, which encapsulates:

- Drawing capabilities through callback mechanisms
- Input handling through event callbacks
- Screen orientation management
- Thread safety through mutex protection
- GUI layer integration

A ViewPort represents a drawable region that can receive input events. It operates within the larger GUI system, which manages multiple ViewPorts in different layers and handles their drawing order and input distribution.

## ViewPort Lifecycle

### Creation and Initialization

A ViewPort is created using `view_port_alloc()`, which sets up the basic structure with default values:

- Horizontal orientation
- Enabled state
- Recursive mutex for thread safety

Example of proper ViewPort initialization:

```c
ViewPort* view_port = view_port_alloc();
view_port_draw_callback_set(view_port, draw_callback, context);
view_port_input_callback_set(view_port, input_callback, context);
```

### Memory Management

The ViewPort system follows a clear ownership model:

1. The application is responsible for allocating and freeing ViewPorts
2. ViewPorts must be removed from the GUI before being freed
3. Callbacks must not outlive the ViewPort

### Cleanup and Deallocation

Proper cleanup sequence:

1. Disable the ViewPort
2. Remove from GUI
3. Free the ViewPort

```c
view_port_enabled_set(view_port, false);
gui_remove_view_port(gui, view_port);
view_port_free(view_port);
```

## Drawing System

The ViewPort drawing system is callback-based, allowing applications to define their rendering logic while the framework handles the timing and coordination of drawing operations.

### Drawing Callback

The drawing callback provides a Canvas object for rendering:

```c
void draw_callback(Canvas* canvas, void* context) {
    // Drawing operations here
    canvas_draw_str(canvas, 0, 10, "Hello World");
}
```

Key considerations for drawing:

- Drawing occurs on the GUI thread
- Operations should be efficient to maintain UI responsiveness
- The Canvas state is reset before each callback invocation
- Thread safety must be maintained when accessing shared data

### Canvas Operations

The Canvas provides various drawing primitives:

- Text rendering with multiple fonts
- Geometric shapes (lines, rectangles, circles)
- Bitmap and icon drawing
- Pixel-level operations

## Input Handling

The ViewPort input system processes user interactions through a callback mechanism that handles the directional pad and buttons.

### Input Callback

Input callbacks receive InputEvent structures containing:

- Key code (up, down, left, right, ok, back)
- Event type (press, release, repeat, short, long)
- Sequence information

```c
void input_callback(InputEvent* event, void* context) {
    if(event->type == InputTypeShort) {
        switch(event->key) {
            case InputKeyOk:
                // Handle OK button press
                break;
        }
    }
}
```

### Input Mapping

The ViewPort system automatically handles input mapping based on screen orientation:

- Directional inputs are remapped to maintain logical consistency
- Hardware inputs are translated to logical inputs
- Left-handed mode is supported through automatic remapping

## Orientation Management

ViewPorts support four orientations:

- Horizontal (default)
- Horizontal Flip
- Vertical
- Vertical Flip

Orientation affects both drawing and input:

- Canvas coordinates are automatically adjusted
- Input mapping is reconfigured
- Drawing callbacks receive properly oriented Canvas

```c
view_port_set_orientation(view_port, ViewPortOrientationVertical);
```

## Thread Safety

The ViewPort system is designed for thread-safe operation through several mechanisms:

### Mutex Protection

- Each ViewPort has a recursive mutex
- All public API calls are protected
- Drawing and input callbacks are synchronized

### Update Mechanism

- view_port_update() triggers redraws safely
- GUI thread handles drawing coordination
- Input events are processed sequentially

Critical considerations:

- Avoid holding locks during callbacks
- Minimize critical section duration
- Use appropriate synchronization for shared data

## Integration with GUI

ViewPorts integrate with the GUI system through layers:

```c
gui_add_view_port(gui, view_port, GuiLayerFullscreen);
```

Layer types:

- GuiLayerFullscreen: Full screen coverage
- GuiLayerWindow: Standard window with status bar
- GuiLayerStatusBarLeft/Right: Status bar segments

### Update Flow

1. Application calls view_port_update()
2. GUI system schedules redraw
3. Drawing callback is invoked on GUI thread
4. Canvas updates are committed to display

## Best Practices

1. Memory Management

   - Always pair allocation with proper cleanup
   - Remove from GUI before freeing
   - Clean up resources in reverse order of allocation

2. Performance

   - Keep drawing callbacks efficient
   - Minimize critical section duration
   - Cache complex calculations
   - Update only when necessary

3. Thread Safety

   - Use appropriate synchronization for shared data
   - Avoid blocking operations in callbacks
   - Consider message queues for event handling

4. Error Handling
   - Check return values from GUI operations
   - Handle memory allocation failures
   - Implement proper cleanup paths

## Common Patterns

### Basic Display Application

```c
typedef struct {
    ViewPort* view_port;
    Gui* gui;
} MyApp;

static void draw_callback(Canvas* canvas, void* context) {
    // Drawing code
}

static void input_callback(InputEvent* event, void* context) {
    // Input handling code
}

MyApp* my_app_alloc() {
    MyApp* app = malloc(sizeof(MyApp));
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    return app;
}
```

## Conclusion

The ViewPort system provides a robust foundation for building Flipper Zero applications with rich user interfaces. Its thread-safe design, flexible orientation support, and integration with the GUI system make it a powerful tool for developers.

Key takeaways:

- Follow proper lifecycle management
- Maintain thread safety
- Use appropriate synchronization
- Consider performance implications
- Follow established patterns

Understanding these concepts enables developers to create reliable and efficient Flipper Zero applications while avoiding common pitfalls and maintenance issues.
