# Flipper Zero ViewPort and UI System Documentation

## Overview

The Flipper Zero UI system is built around several key components that work together to create the user interface. At its core is the ViewPort, which provides the most basic UI functionality. The system follows a hierarchical structure:

1. **ViewPort** - The basic building block for rendering and input handling
2. **View** - A more sophisticated UI component built on top of ViewPort
3. **ViewDispatcher** - Manages multiple Views and handles navigation
4. **Modules** - Pre-built UI components (like menus, dialogs)
5. **SceneManager** - Handles complex UI flows and scene transitions

## ViewPort Deep Dive

### What is ViewPort?

ViewPort is the fundamental UI component in the Flipper Zero's GUI system. It provides:

- Basic drawing capabilities through a Canvas
- Input handling from the device's buttons
- Screen orientation management
- Enable/disable functionality
- Automatic input mapping based on orientation

### Key Components of ViewPort

#### 1. Structure

```c
struct ViewPort {
    Gui* gui;                          // Reference to GUI system
    FuriMutex* mutex;                  // Thread safety mutex
    bool is_enabled;                   // Visibility flag
    ViewPortOrientation orientation;    // Screen orientation
    uint8_t width;                     // ViewPort width
    uint8_t height;                    // ViewPort height
    ViewPortDrawCallback draw_callback; // Drawing function
    void* draw_callback_context;       // Drawing context
    ViewPortInputCallback input_callback; // Input handling function
    void* input_callback_context;      // Input context
};
```

#### 2. Callbacks

ViewPort uses two main callbacks:

```c
// Drawing callback
typedef void (*ViewPortDrawCallback)(Canvas* canvas, void* context);

// Input callback
typedef void (*ViewPortInputCallback)(InputEvent* event, void* context);
```

### Using ViewPort

#### 1. Initialization

```c
// Allocate ViewPort
ViewPort* view_port = view_port_alloc();

// Set callbacks
view_port_draw_callback_set(view_port, draw_callback, context);
view_port_input_callback_set(view_port, input_callback, context);

// Add to GUI
Gui* gui = furi_record_open(RECORD_GUI);
gui_add_view_port(gui, view_port, GuiLayerFullscreen);
```

#### 2. Drawing Callback Implementation

```c
static void draw_callback(Canvas* canvas, void* context) {
    // Clear canvas
    canvas_clear(canvas);

    // Set font
    canvas_set_font(canvas, FontPrimary);

    // Draw text
    canvas_draw_str(canvas, 0, 10, "Hello Flipper!");

    // Draw shapes
    canvas_draw_frame(canvas, 0, 20, 50, 30);
}
```

#### 3. Input Callback Implementation

```c
static void input_callback(InputEvent* input_event, void* context) {
    furi_assert(context);
    FuriMessageQueue* queue = context;

    // Queue input event for processing
    furi_message_queue_put(queue, input_event, FuriWaitForever);
}
```

#### 4. Main Loop

```c
bool running = true;
InputEvent event;

while(running) {
    if(furi_message_queue_get(queue, &event, FuriWaitForever) == FuriStatusOk) {
        // Handle input
        if(event.type == InputTypeShort) {
            switch(event.key) {
                case InputKeyBack:
                    running = false;
                    break;
                // Handle other keys...
            }
        }

        // Request redraw
        view_port_update(view_port);
    }
}
```

### ViewPort Orientation

ViewPort supports four orientations:

```c
typedef enum {
    ViewPortOrientationHorizontal,      // Normal
    ViewPortOrientationHorizontalFlip,  // Upside down
    ViewPortOrientationVertical,        // 90 degrees (USB at bottom)
    ViewPortOrientationVerticalFlip     // 270 degrees (USB at top)
} ViewPortOrientation;
```

To set orientation:

```c
view_port_set_orientation(view_port, ViewPortOrientationVertical);
```

The orientation affects both:

1. How the screen is rendered
2. How input directions are mapped

### Thread Safety

ViewPort uses a mutex for thread safety:

```c
// In ViewPort operations
furi_mutex_acquire(view_port->mutex, FuriWaitForever);
// Perform operations...
furi_mutex_release(view_port->mutex);
```

Best practices:

- Always use mutex when accessing shared state
- Keep critical sections as short as possible
- Use the provided context for state management

### Memory Management

Proper cleanup is essential:

```c
// Cleanup sequence
view_port_enabled_set(view_port, false);
gui_remove_view_port(gui, view_port);
view_port_free(view_port);
furi_record_close(RECORD_GUI);
```

### Best Practices

1. **State Management**

   ```c
   typedef struct {
       FuriMutex* mutex;
       // Your state variables
   } MyAppState;

   // Allocate state
   MyAppState* state = malloc(sizeof(MyAppState));
   state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
   ```

2. **Thread-Safe Updates**

   ```c
   static void update_state(MyAppState* state, int new_value) {
       furi_mutex_acquire(state->mutex, FuriWaitForever);
       // Update state
       furi_mutex_release(state->mutex);

       // Request redraw
       view_port_update(view_port);
   }
   ```

3. **Input Queue Processing**

   ```c
   // Define custom event type
   typedef enum {
       MyEventTypeInput,
       MyEventTypeTimer
   } MyEventType;

   typedef struct {
       MyEventType type;
       InputEvent input;  // For input events
   } MyEvent;

   // Create queue
   FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(MyEvent));
   ```

4. **Separation of Concerns**
   - Keep drawing logic in draw callback
   - Queue input events for processing in main loop
   - Use state struct for shared data
   - Implement clean shutdown sequence

### Common Pitfalls

1. **Mutex Deadlocks**

   - Don't hold mutex across callbacks
   - Use appropriate timeout values
   - Release mutex before calling external functions

2. **Memory Leaks**

   - Free all allocated resources
   - Follow allocation/deallocation pairs
   - Check for null pointers

3. **Thread Safety Issues**
   - Always protect shared state
   - Don't assume callback execution order
   - Use message queue for input processing

### Advanced Features

#### 1. Custom Drawing

```c
static void draw_custom_ui(Canvas* canvas, MyAppState* state) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, state->x, state->y, 10, 10);

    // XOR drawing for highlights
    canvas_set_color(canvas, ColorXOR);
    canvas_draw_frame(canvas, state->x-1, state->y-1, 12, 12);
}
```

#### 2. Animation Support

```c
static void timer_callback(FuriTimer* timer) {
    MyAppState* state = timer->context;
    state->animation_frame++;
    view_port_update(view_port);
}

// Create timer
FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, state);
furi_timer_start(timer, 1000); // 1 second interval
```

## Summary

ViewPort provides a robust foundation for building UIs on the Flipper Zero. Key points to remember:

- Use mutex for thread safety
- Process input via message queue
- Maintain clean separation of concerns
- Follow proper resource management
- Leverage orientation support for flexible UIs

The ViewPort system, while simple, is powerful enough to create complex UIs when combined with proper state management and event handling.
