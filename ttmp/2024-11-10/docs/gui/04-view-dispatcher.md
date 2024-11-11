Here's a detailed documentation of how the ViewDispatcher works in the Flipper Zero GUI service:

# ViewDispatcher Documentation

## Overview

The ViewDispatcher is a key component in the Flipper Zero GUI system that manages multiple Views and handles switching between them. It acts as a coordinator that:

1. Maintains a registry of Views identified by unique IDs
2. Manages the current active View
3. Handles input events and routes them to the active View
4. Processes custom events and navigation (back button) events
5. Manages view transitions and lifecycle events

## Core Concepts

### Views and View IDs

- Views must be registered with the ViewDispatcher using unique numeric IDs
- Each View represents a distinct screen/interface in the application
- Multiple Views can be registered but only one is active at a time
- Views cannot be simultaneously registered with multiple ViewDispatchers

```c
typedef enum {
    MyFirstViewId,
    MySecondViewId,
    // ... other view IDs
} ViewId;

// Register a view
view_dispatcher_add_view(view_dispatcher, MyFirstViewId, my_view);
```

### Event Handling

The ViewDispatcher processes three main types of events:

1. Input Events

- Handles d-pad and button inputs
- Routes events to the current View's input callback
- Manages input state tracking for press/release pairs

2. Custom Events

- Application-defined events sent via `view_dispatcher_send_custom_event()`
- First passed to current View's custom callback
- If unhandled, passed to ViewDispatcher's custom event callback

3. Navigation Events

- Triggered by Back button if not handled by current View
- Can be used to implement view navigation/transitions
- Navigation callback determines if application should exit

## Initialization and Setup

### Creating a ViewDispatcher

```c
// Allocate
ViewDispatcher* view_dispatcher = view_dispatcher_alloc();

// Enable event queue (required for handling events)
view_dispatcher_enable_queue(view_dispatcher);

// Set event callbacks
view_dispatcher_set_event_callback_context(view_dispatcher, context);
view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);
view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);

// Attach to GUI
Gui* gui = furi_record_open(RECORD_GUI);
view_dispatcher_attach_to_gui(
    view_dispatcher,
    gui,
    ViewDispatcherTypeFullscreen  // or Window/Desktop
);
```

### Adding Views

```c
// Create and configure view
View* view = view_alloc();
view_set_draw_callback(view, draw_callback);
view_set_input_callback(view, input_callback);

// Add to dispatcher
view_dispatcher_add_view(view_dispatcher, VIEW_ID, view);

// Switch to view
view_dispatcher_switch_to_view(view_dispatcher, VIEW_ID);
```

## Event Flow

### Input Event Flow

1. GUI system captures input
2. ViewDispatcher receives input via registered ViewPort
3. Input queued to ViewDispatcher's input queue
4. ViewDispatcher processes queued input:
   - Tracks ongoing input state
   - Routes to current View's input callback
   - If unhandled and Back button, triggers navigation

### Custom Event Flow

1. Application sends custom event:

```c
view_dispatcher_send_custom_event(view_dispatcher, MY_EVENT);
```

2. Event queued to ViewDispatcher's event queue
3. ViewDispatcher processes event:
   - First sent to current View's custom callback
   - If unhandled, sent to ViewDispatcher's custom callback

### Navigation Event Flow

1. Back button pressed
2. Current View's input callback can handle it
3. If unhandled:
   - View's navigation callback called
   - Can return new view ID to switch to
4. If still unhandled:
   - ViewDispatcher's navigation callback called
   - Can return false to exit application

## View Lifecycle

### View Transitions

When switching views:

1. Current view's exit callback called
2. View orientation updated if needed
3. New view's enter callback called
4. ViewPort enabled/updated to trigger redraw

```c
void view_dispatcher_set_current_view(ViewDispatcher* dispatcher, View* view) {
    // Exit current view
    if(dispatcher->current_view) {
        view_exit(dispatcher->current_view);
    }

    // Set new view
    dispatcher->current_view = view;

    // Enter new view
    if(view) {
        view_enter(view);
        view_port_enabled_set(dispatcher->view_port, true);
        view_port_update(dispatcher->view_port);
    }
}
```

### Cleanup

When shutting down:

1. Remove all views
2. Free ViewDispatcher
3. Close GUI

```c
// Remove views
view_dispatcher_remove_view(view_dispatcher, VIEW_ID);
view_free(view);

// Cleanup dispatcher
view_dispatcher_free(view_dispatcher);
furi_record_close(RECORD_GUI);
```

## Best Practices

1. **Event Handling**

- Always handle or explicitly ignore input events
- Use custom events for View-to-View communication
- Keep navigation logic in navigation callbacks

2. **View Management**

- Free Views after removing from ViewDispatcher
- Don't share Views between ViewDispatchers
- Clean up resources in View exit callbacks

3. **Threading**

- ViewDispatcher callbacks run in GUI thread
- Don't block in callbacks
- Use message queues for thread communication

4. **Error Handling**

- Check return values from ViewDispatcher operations
- Maintain valid View state during transitions
- Clean up resources on error paths

## Example Use Case

Here's a basic example showing common ViewDispatcher usage:

```c
typedef struct {
    ViewDispatcher* view_dispatcher;
    View* main_view;
    View* settings_view;
} MyApp;

bool my_navigation_callback(void* context) {
    MyApp* app = context;
    return false; // Exit app on back
}

void my_custom_callback(void* context, uint32_t event) {
    MyApp* app = context;
    switch(event) {
        case ShowSettings:
            view_dispatcher_switch_to_view(
                app->view_dispatcher,
                SettingsViewId
            );
            break;
    }
}

MyApp* app_alloc() {
    MyApp* app = malloc(sizeof(MyApp));

    // Create dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(
        app->view_dispatcher,
        app
    );
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher,
        my_navigation_callback
    );
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher,
        my_custom_callback
    );

    // Add views
    app->main_view = view_alloc();
    app->settings_view = view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        MainViewId,
        app->main_view
    );
    view_dispatcher_add_view(
        app->view_dispatcher,
        SettingsViewId,
        app->settings_view
    );

    return app;
}
```

This documentation should help developers understand how the ViewDispatcher works and how to effectively use it in Flipper Zero applications.
