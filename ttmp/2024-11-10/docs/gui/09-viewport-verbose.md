# Flipper Zero ViewPort Deep Dive

## Fundamental Concepts

ViewPort is the most basic UI element in the Flipper Zero GUI system. It represents a drawable area on the screen with its own input handling capabilities. Think of it as a window or canvas that you can draw on and interact with. The ViewPort system is designed around three core principles:

1. Separation of rendering and input handling
2. Thread-safe operation in a multi-threaded environment
3. Automatic handling of different screen orientations

Let's explore each component in detail.

## Core Components

### 1. ViewPort Structure
The ViewPort structure encapsulates everything needed for a basic UI component. Understanding each field is crucial for proper implementation:

```c
struct ViewPort {
    Gui* gui;                          // Reference to GUI system
    FuriMutex* mutex;                  // Thread safety mechanism
    bool is_enabled;                   // Visibility control
    ViewPortOrientation orientation;    // Screen orientation setting
    uint8_t width;                     // ViewPort width
    uint8_t height;                    // ViewPort height
    ViewPortDrawCallback draw_callback; // Drawing function
    void* draw_callback_context;       // Context for drawing
    ViewPortInputCallback input_callback; // Input handler
    void* input_callback_context;      // Context for input
};
```

This structure follows the principle of encapsulation - each ViewPort maintains its own state and callbacks, allowing multiple ViewPorts to coexist independently.

### 2. Basic Setup and Initialization
The initialization process follows a specific pattern to ensure proper resource allocation and setup:

```c
// Allocate ViewPort
ViewPort* view_port = view_port_alloc();

// Configure callbacks
view_port_draw_callback_set(view_port, my_draw_callback, draw_context);
view_port_input_callback_set(view_port, my_input_callback, input_context);

// Connect to GUI system
Gui* gui = furi_record_open(RECORD_GUI);
gui_add_view_port(gui, view_port, GuiLayerFullscreen);
```

This initialization pattern ensures:
- All resources are properly allocated
- Callbacks are set up before any rendering or input can occur
- The ViewPort is properly integrated into the GUI system

### 3. Drawing Implementation
The drawing callback is where you implement your UI's visual appearance. This callback should be pure rendering code, avoiding any state modifications:

```c
static void my_draw_callback(Canvas* canvas, void* context) {
    // Cast context to your data type if needed
    MyAppState* state = context;
    
    // Clear canvas before drawing
    canvas_clear(canvas);
    
    // Set up drawing properties
    canvas_set_font(canvas, FontPrimary);
    
    // Draw your UI elements
    canvas_draw_str(canvas, 0, 10, "Hello Flipper!");
    
    // Access state safely
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_draw_str(canvas, state->x, state->y, "^");
    furi_mutex_release(state->mutex);
}
```

Key principles for drawing:
- Always clear the canvas first
- Use thread-safe state access
- Keep drawing code efficient
- Separate drawing from logic

### 4. Input Handling
Input handling in ViewPort follows an event-driven pattern. The input callback receives input events that you can process:

```c
static void my_input_callback(InputEvent* input_event, void* context) {
    // Get our message queue from context
    FuriMessageQueue* queue = context;
    
    // Queue input event for processing in main loop
    furi_message_queue_put(queue, input_event, FuriWaitForever);
}
```

Important input handling principles:
- Don't process input directly in the callback
- Queue events for processing in main loop
- Use appropriate context for event handling

### 5. Main Application Loop
The main loop is where you process queued events and update your application state:

```c
int32_t my_viewport_app() {
    // Initialize message queue
    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    // Initialize application state
    MyAppState* state = malloc(sizeof(MyAppState));
    state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    
    // Main event loop
    bool running = true;
    InputEvent event;
    
    while(running) {
        // Wait for and process events
        if(furi_message_queue_get(queue, &event, FuriWaitForever) == FuriStatusOk) {
            // Handle input events
            if(event.type == InputTypeShort) {
                switch(event.key) {
                    case InputKeyBack:
                        running = false;
                        break;
                    // Handle other keys...
                }
            }
            
            // Request redraw if needed
            view_port_update(view_port);
        }
    }
    
    // Cleanup
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_mutex_free(state->mutex);
    free(state);
    furi_message_queue_free(queue);
    
    return 0;
}
```

Main loop principles:
- Event-driven architecture
- Clean resource management
- Proper state handling
- Responsive UI updates

### 6. Thread Safety
Thread safety is crucial in the ViewPort system. Here's how to handle shared state:

```c
typedef struct {
    FuriMutex* mutex;
    int32_t x;
    int32_t y;
    // Other state variables...
} MyAppState;

// Updating state safely
static void update_state(MyAppState* state, int32_t new_x, int32_t new_y) {
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    state->x = new_x;
    state->y = new_y;
    furi_mutex_release(state->mutex);
}

// Reading state safely
static void read_state(MyAppState* state, int32_t* x, int32_t* y) {
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    *x = state->x;
    *y = state->y;
    furi_mutex_release(state->mutex);
}
```

Thread safety principles:
- Protect shared state with mutex
- Keep critical sections small
- Use appropriate mutex types
- Always release acquired mutexes

### 7. Orientation Handling
ViewPort automatically handles screen orientation changes:

```c
// Set ViewPort orientation
view_port_set_orientation(view_port, ViewPortOrientationVertical);
```

The ViewPort system automatically:
- Rotates the display output
- Remaps input directions
- Maintains consistent user experience

Orientation principles:
- Set orientation early in initialization
- Consider default orientation for your app
- Test all orientations thoroughly
- Handle orientation-specific layout adjustments

## Best Practices

### 1. State Management
```c
typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    ViewPort* view_port;
    // Application-specific state...
} MyApp;

static MyApp* my_app_alloc() {
    MyApp* app = malloc(sizeof(MyApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->view_port = view_port_alloc();
    return app;
}

static void my_app_free(MyApp* app) {
    furi_mutex_free(app->mutex);
    furi_message_queue_free(app->queue);
    view_port_free(app->view_port);
    free(app);
}
```

State management principles:
- Centralize state in a single structure
- Initialize all components at startup
- Clean up all resources at shutdown
- Use appropriate protection mechanisms

### 2. Event Processing
```c
typedef enum {
    MyEventTypeInput,
    MyEventTypeTimer,
    MyEventTypeCustom
} MyEventType;

typedef struct {
    MyEventType type;
    union {
        InputEvent input;
        uint32_t timer_ticks;
        // Custom event data...
    } data;
} MyEvent;

// Event processing loop
while(running) {
    MyEvent event;
    if(furi_message_queue_get(app->queue, &event, 100) == FuriStatusOk) {
        switch(event.type) {
            case MyEventTypeInput:
                process_input(app, &event.data.input);
                break;
            case MyEventTypeTimer:
                process_timer(app, event.data.timer_ticks);
                break;
            // Handle other events...
        }
    }
}
```

Event processing principles:
- Use typed events for clarity
- Handle all event types appropriately
- Consider event priorities
- Maintain responsiveness

### 3. Resource Management
```c
// Proper cleanup sequence
static void cleanup_resources(MyApp* app) {
    // Disable ViewPort first
    view_port_enabled_set(app->view_port, false);
    
    // Remove from GUI
    gui_remove_view_port(app->gui, app->view_port);
    
    // Free GUI record
    furi_record_close(RECORD_GUI);
    
    // Free ViewPort
    view_port_free(app->view_port);
    
    // Free other resources
    furi_mutex_free(app->mutex);
    furi_message_queue_free(app->queue);
    
    // Free app structure
    free(app);
}
```

Resource management principles:
- Clean up in reverse order of allocation
- Handle cleanup errors gracefully
- Verify all resources are freed
- Follow consistent cleanup patterns

## Summary

The ViewPort system provides a robust foundation for building UIs on the Flipper Zero. Key takeaways:

1. **Architectural Principles**
   - Separation of concerns
   - Event-driven design
   - Thread-safe operation
   - Resource management

2. **Implementation Guidelines**
   - Use appropriate protection mechanisms
   - Follow established patterns
   - Handle errors gracefully
   - Maintain clean code structure

3. **Best Practices**
   - Centralize state management
   - Process events efficiently
   - Clean up resources properly
   - Handle orientation changes

Understanding these concepts and principles will help you create robust and maintainable applications using the ViewPort system.