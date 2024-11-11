# Building Your First Flipper Zero GUI Application

This tutorial will guide you through creating a GUI application for the Flipper Zero, explaining key concepts along the way. We'll build a simple temperature unit converter that demonstrates core GUI concepts, user interaction, and application architecture.

## Background Concepts

Before diving into the code, let's understand some key concepts:

### GUI Architecture

The Flipper Zero GUI system is built on several layers:

1. **ViewPort**: The lowest level unit, responsible for actual drawing operations and input handling
2. **View**: A higher-level abstraction that contains application logic and state
3. **View Management**: Systems like ViewHolder or ViewDispatcher that handle displaying and switching views

### Application Lifecycle

A typical GUI application follows this pattern:
1. Initialize resources and GUI components
2. Set up views and callbacks
3. Run the main event loop
4. Clean up on exit

### Memory Management

Flipper Zero applications must carefully manage memory:
- All allocated resources must be freed when no longer needed
- Follow the pattern where the component that allocates a resource is responsible for freeing it

## Tutorial: Temperature Converter

We'll build a temperature converter that:
- Shows the current temperature in both Celsius and Fahrenheit
- Allows adjusting the temperature with up/down buttons
- Demonstrates proper GUI application structure

### Step 1: Basic Application Structure

Let's start with the basic application structure:

```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    
    // Application state
    float celsius;
} TempConverterApp;

```

First, let's understand what each part does:

- `Gui*`: Main GUI system interface
- `ViewPort*`: Our drawing canvas and input handler
- `FuriMessageQueue*`: Queue for handling input events
- `celsius`: Our application state (temperature in Celsius)

### Step 2: Drawing Function

Next, we'll create the draw callback that renders our interface:

```c
static void temp_converter_draw_callback(Canvas* canvas, void* ctx) {
    TempConverterApp* app = ctx;
    float fahrenheit = (app->celsius * 9/5.0f) + 32;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Draw title
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "Temperature Converter");
    
    // Draw temperatures
    char temp_str[32];
    canvas_set_font(canvas, FontSecondary);
    
    snprintf(temp_str, sizeof(temp_str), "Celsius: %.1f°C", app->celsius);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, temp_str);
    
    snprintf(temp_str, sizeof(temp_str), "Fahrenheit: %.1f°F", fahrenheit);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignTop, temp_str);
    
    // Draw instructions
    canvas_draw_str_aligned(
        canvas, 64, 55, AlignCenter, AlignTop, 
        "Up/Down to adjust\nBack to exit");
}
```

Let's break down the drawing concepts:

- `Canvas*`: The drawing surface
- `canvas_clear()`: Clears the screen
- `canvas_set_font()`: Changes text font
- `canvas_draw_str_aligned()`: Draws text with alignment options

The coordinate system is:
- Origin (0,0) at top-left
- X increases right
- Y increases down
- Screen is 128x64 pixels

### Step 3: Input Handling

Now let's handle user input:

```c
static void temp_converter_input_callback(InputEvent* input_event, void* ctx) {
    TempConverterApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

static void temp_converter_process_input(TempConverterApp* app, InputEvent* event) {
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyUp:
            app->celsius += 0.5f;
            break;
        case InputKeyDown:
            app->celsius -= 0.5f;
            break;
        default:
            break;
        }
    }
}
```

Key input concepts:
- Input events are queued for processing
- Events have type (short press, long press, repeat)
- Input callbacks should be quick - queue events for later processing

### Step 4: Application Management

Now let's implement application lifecycle management:

```c
static TempConverterApp* temp_converter_app_alloc() {
    TempConverterApp* app = malloc(sizeof(TempConverterApp));
    
    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    // Set callbacks
    view_port_draw_callback_set(app->view_port, temp_converter_draw_callback, app);
    view_port_input_callback_set(app->view_port, temp_converter_input_callback, app);
    
    // Add view port to GUI
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Initialize state
    app->celsius = 20.0f;  // Start at room temperature
    
    return app;
}

static void temp_converter_app_free(TempConverterApp* app) {
    // Remove and free GUI elements
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    
    // Close GUI
    furi_record_close(RECORD_GUI);
    
    // Free application
    free(app);
}
```

Important concepts here:
- Resources are allocated in reverse order they're freed
- GUI elements must be properly disconnected before freeing
- Use of RECORD_GUI to access the GUI subsystem

### Step 5: Main Application Loop

Finally, let's implement the main application loop:

```c
int32_t temperature_converter_app(void* p) {
    UNUSED(p);
    TempConverterApp* app = temp_converter_app_alloc();

    InputEvent event;
    bool running = true;
    
    while(running) {
        // Wait for input event
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            // Exit on back button
            if(event.key == InputKeyBack) {
                running = false;
            } else {
                // Process other inputs
                temp_converter_process_input(app, &event);
            }
        }
        
        // Update display
        view_port_update(app->view_port);
    }

    temp_converter_app_free(app);
    return 0;
}
```

Main loop concepts:
- Waits for input events with timeout
- Processes events and updates state
- Requests screen updates
- Handles application exit

### Step 6: Application Registration

To make our application available on the Flipper Zero, we need to create an `application.fam` file:

```python
App(
    appid="temp_converter",
    name="Temperature Converter",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="temperature_converter_app",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_category="Tools",
)
```

## Complete Application

Here's the complete application code:

```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    float celsius;
} TempConverterApp;

static void temp_converter_draw_callback(Canvas* canvas, void* ctx) {
    TempConverterApp* app = ctx;
    float fahrenheit = (app->celsius * 9/5.0f) + 32;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "Temperature Converter");
    
    char temp_str[32];
    canvas_set_font(canvas, FontSecondary);
    
    snprintf(temp_str, sizeof(temp_str), "Celsius: %.1f°C", app->celsius);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, temp_str);
    
    snprintf(temp_str, sizeof(temp_str), "Fahrenheit: %.1f°F", fahrenheit);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignTop, temp_str);
    
    canvas_draw_str_aligned(
        canvas, 64, 55, AlignCenter, AlignTop, 
        "Up/Down to adjust\nBack to exit");
}

static void temp_converter_input_callback(InputEvent* input_event, void* ctx) {
    TempConverterApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

static void temp_converter_process_input(TempConverterApp* app, InputEvent* event) {
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyUp:
            app->celsius += 0.5f;
            break;
        case InputKeyDown:
            app->celsius -= 0.5f;
            break;
        default:
            break;
        }
    }
}

static TempConverterApp* temp_converter_app_alloc() {
    TempConverterApp* app = malloc(sizeof(TempConverterApp));
    
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    view_port_draw_callback_set(app->view_port, temp_converter_draw_callback, app);
    view_port_input_callback_set(app->view_port, temp_converter_input_callback, app);
    
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    app->celsius = 20.0f;
    
    return app;
}

static void temp_converter_app_free(TempConverterApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t temperature_converter_app(void* p) {
    UNUSED(p);
    TempConverterApp* app = temp_converter_app_alloc();

    InputEvent event;
    bool running = true;
    
    while(running) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.key == InputKeyBack) {
                running = false;
            } else {
                temp_converter_process_input(app, &event);
            }
        }
        view_port_update(app->view_port);
    }

    temp_converter_app_free(app);
    return 0;
}
```

## Next Steps

To enhance this application, you could:

1. Add input validation and temperature limits
2. Save/restore the last used temperature
3. Add animation effects
4. Use ViewDispatcher to add multiple screens
5. Add a settings menu

## Key Takeaways

- GUI applications need proper resource management
- Input handling should be queue-based for responsiveness
- Drawing operations happen through the Canvas API
- ViewPorts handle both drawing and input
- Applications should clean up all resources on exit

This tutorial covered the basics of GUI application development for the Flipper Zero. The temperature converter demonstrates fundamental concepts while remaining simple enough to understand. As you build more complex applications, consider using higher-level abstractions like ViewDispatcher and SceneManager to handle more sophisticated user interfaces.