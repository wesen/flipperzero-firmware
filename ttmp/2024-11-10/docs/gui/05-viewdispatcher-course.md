# Flipper Zero ViewDispatcher Course

## Course Overview
This course teaches developers how to effectively use the ViewDispatcher system in Flipper Zero applications. Through progressive lessons and hands-on exercises, you'll learn to manage multiple views, handle user input, and create polished user interfaces.

## Section 1: Fundamentals of ViewDispatcher

### Understanding ViewDispatcher Basics

The ViewDispatcher is a core component that manages multiple views in a Flipper Zero application. Think of it as a traffic controller for your app's screens - it determines which view is currently visible and routes user input to the appropriate handlers.

At its most basic, the ViewDispatcher requires three key components:
1. The ViewDispatcher instance itself
2. One or more View objects 
3. A connection to the GUI system

Here's how these pieces come together:

```c
// Create the dispatcher
ViewDispatcher* view_dispatcher = view_dispatcher_alloc();

// Create a view
View* my_view = view_alloc();

// Get GUI handle and connect
Gui* gui = furi_record_open(RECORD_GUI);
view_dispatcher_attach_to_gui(
    view_dispatcher, 
    gui, 
    ViewDispatcherTypeFullscreen
);
```

The ViewDispatcher uses a simple identifier system to track views. Each view is registered with a numeric ID that you define:

```c
// Define view IDs
enum {
    MyMainViewId,
    MySettingsViewId
};

// Register views
view_dispatcher_add_view(view_dispatcher, MyMainViewId, my_view);
view_dispatcher_add_view(view_dispatcher, MySettingsViewId, settings_view);
```

To switch between views, you use these IDs:

```c
// Switch to main view
view_dispatcher_switch_to_view(view_dispatcher, MyMainViewId);
```

The ViewDispatcher must be explicitly started to begin processing events:

```c
view_dispatcher_run(view_dispatcher);
```

### Event Queue System

A key concept is that the ViewDispatcher uses an event queue to handle input and custom events. This must be enabled:

```c
view_dispatcher_enable_queue(view_dispatcher);
```

Once enabled, you can send custom events to trigger view changes or other actions:

```c
view_dispatcher_send_custom_event(view_dispatcher, MyCustomEventId);
```

### Exercises for Section 1

#### Exercise 1.1: Basic Setup
Create a minimal application that:
- Allocates a ViewDispatcher
- Creates one View
- Registers the View with ID=0
- Connects to GUI
- Runs the dispatcher
- Properly cleans up resources

#### Exercise 1.2: Multiple Views
Extend Exercise 1.1 to:
- Create two Views
- Register them with different IDs
- Switch between them after a 2-second delay
- Properly clean up both views

#### Exercise 1.3: Custom Events
Create an application that:
- Has two views
- Sends a custom event every second
- Switches between views when receiving the custom event
- Uses proper event queue initialization

#### Exercise 1.4: View Cleanup
Create an application demonstrating proper cleanup:
- Register 3 views
- Switch between them
- Implement a proper shutdown sequence
- Verify no memory leaks using the debug console

## Section 2: View Management and Callbacks

### Understanding View Lifecycle

Views in the ViewDispatcher system have a defined lifecycle with specific callbacks:

```c
// Draw callback - called when view needs to render
static void view_draw_callback(Canvas* canvas, void* context) {
    canvas_draw_str(canvas, 0, 10, "Hello World");
}

// Input callback - called for button presses
static bool view_input_callback(InputEvent* event, void* context) {
    if(event->type == InputTypeShort) {
        return true; // Event handled
    }
    return false; // Event not handled
}

// Setup callbacks
view_set_draw_callback(view, view_draw_callback);
view_set_input_callback(view, view_input_callback);
```

Enter and exit callbacks are triggered when switching views:

```c
static void view_enter_callback(void* context) {
    // Setup when view becomes active
}

static void view_exit_callback(void* context) {
    // Cleanup when view becomes inactive
}

view_set_enter_callback(view, view_enter_callback);
view_set_exit_callback(view, view_exit_callback);
```

### Exercises for Section 2

#### Exercise 2.1: View Callbacks
Create an application with:
- One view with all callbacks implemented
- Draw callback that shows how long view has been active
- Input callback that handles all button types
- Enter/exit callbacks that log to debug console

#### Exercise 2.2: Context Usage
Create an application that:
- Defines a custom context struct with counter
- Passes context through to all callbacks
- Updates counter in input callback
- Displays counter in draw callback

#### Exercise 2.3: View Switching
Create an application with:
- Two views sharing same context
- Different drawing styles for each view
- Proper state handling during switches
- Enter/exit callbacks that maintain view state

#### Exercise 2.4: Input Handling
Create an application demonstrating:
- Complex input handling (combinations)
- Input state tracking between views
- Custom event generation from input
- Proper input event consumption

## Section 3: Advanced ViewDispatcher Features

### Navigation and Custom Events

The ViewDispatcher can handle navigation (back button) and custom events with special callbacks:

```c
bool navigation_callback(void* context) {
    // Return true to handle back button
    // Return false to exit application
    return true;
}

bool custom_callback(void* context, uint32_t event) {
    // Handle custom events
    switch(event) {
        case MyCustomEvent:
            // Do something
            return true;
    }
    return false;
}

view_dispatcher_set_navigation_event_callback(
    view_dispatcher,
    navigation_callback
);

view_dispatcher_set_custom_event_callback(
    view_dispatcher,
    custom_callback
);
```

### Exercises for Section 3

#### Exercise 3.1: Navigation System
Create an application with:
- Three views in a hierarchical structure
- Navigation callback implementing back behavior
- Custom transition animations
- State preservation between views

#### Exercise 3.2: Custom Event System
Create an application demonstrating:
- Multiple custom event types
- Event handling at view and dispatcher level
- Event queueing and processing
- Error handling for events

#### Exercise 3.3: Complex State Management
Create an application that:
- Maintains complex state across views
- Handles state corruption scenarios
- Implements save/restore functionality
- Uses custom events for state updates

#### Exercise 3.4: Integration Project
Create a complete application that:
- Uses all ViewDispatcher features
- Implements a coherent UI flow
- Handles all error cases
- Follows best practices for resource management

## Final Project

### ViewDispatcher Weather Station

Create a weather station application that demonstrates mastery of the ViewDispatcher system:

Requirements:
1. Main view showing current readings
2. Settings view for configuration
3. History view showing graphs
4. About view with information
5. Proper navigation between all views
6. Custom events for sensor updates
7. State management for settings
8. Clean shutdown and resource handling

This project will test your understanding of:
- View management
- Event handling
- State management
- Resource cleanup
- User interface design
- Error handling
- Performance optimization

Submit your project with:
- Full source code
- Documentation
- Test cases
- Performance analysis

This course structure provides a progressive learning path from basic concepts to advanced features, with practical exercises reinforcing each concept. Each section builds on the previous knowledge while introducing new concepts and challenges.