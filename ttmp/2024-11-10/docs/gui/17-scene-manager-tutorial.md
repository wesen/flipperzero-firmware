# Building a Multi-Scene Flipper Zero Application: A Step-by-Step Guide

## Introduction

In this tutorial, we'll build a "Device Info" app that shows different system information across multiple screens. We'll start with the basics and gradually add more complex features. Each step introduces new concepts with detailed explanations.

## Step 1: Basic Single-Screen Application

Let's start with the simplest possible application using ViewDispatcher.

```c
// device_info.h
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* main_view;  // Basic view to start with
} DeviceInfoApp;

// device_info.c
typedef struct {
    int x;  // Simple state for demonstration
    int y;
} MainViewModel;

static void view_draw_callback(Canvas* canvas, void* model) {
    MainViewModel* state = model;  // Now receiving model directly
    canvas_draw_str(canvas, state->x, state->y, "Hello Flipper!");
}

int32_t device_info_app(void* p) {
    DeviceInfoApp* app = malloc(sizeof(DeviceInfoApp));

    // Initialize basic components
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->main_view = view_alloc();

    // Configure view with proper model allocation
    view_allocate_model(app->main_view, ViewModelTypeLocking, sizeof(MainViewModel));

    // Initialize model using with_view_model
    with_view_model(
        app->main_view,
        MainViewModel* model,
        {
            model->x = 10;
            model->y = 30;
        },
        true);

    view_set_draw_callback(app->main_view, view_draw_callback);

    // Set up ViewDispatcher
    view_dispatcher_add_view(app->view_dispatcher, 0, app->main_view);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    // Run the app
    view_dispatcher_run(app->view_dispatcher);

    // Cleanup (order is important!)
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    view_free_model(app->main_view);
    view_free(app->main_view);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
```

### Key Concepts Explained:

1. **View**: A basic UI component that handles drawing and input
2. **ViewDispatcher**: Manages views and coordinates their display
3. **Drawing Callback**: Function called to render the view's content
4. **View Context**: State data associated with a view

### Beginner Exercises:

1. Change the text position and content
2. Add a second line of text
3. Draw a simple shape (rectangle or circle)
4. Add a border around the text

## Step 2: Adding Input Handling

Let's make our app interactive by responding to button presses.

```c
static bool view_input_callback(InputEvent* event, void* context) {
    DeviceInfoApp* app = context;
    bool handled = false;

    if(event->type == InputTypeShort) {
        with_view_model(
            app->main_view,  // Get view from app context
            MainViewModel* model,
            {
                switch(event->key) {
                    case InputKeyUp:
                        model->y -= 2;
                        handled = true;
                        break;
                    case InputKeyDown:
                        model->y += 2;
                        handled = true;
                        break;
                }
            },
            handled);  // Only update if we handled the event
    }

    return handled;
}

// In main app:
view_set_input_callback(app->main_view, view_input_callback);
```

### Key Concepts Explained:

1. **Input Callback**: Function called when input events occur
2. **Input Types**: Different kinds of input events (short press, long press, etc.)
3. **Event Handling**: Processing input and updating state
4. **View Updates**: Marking events as handled

### Intermediate Exercises:

1. Add left/right movement
2. Implement different behaviors for long press
3. Add boundaries to prevent text from moving off screen
4. Create a simple animation when moving

## Step 3: Introducing Scene Manager

Now let's add a second screen and manage navigation between them.

```c
// Add to device_info.h
typedef enum {
    DeviceInfoSceneMain,
    DeviceInfoSceneDetails,
    DeviceInfoSceneNum  // Must be last
} DeviceInfoScene;

// New structure for scene handlers
extern const SceneManagerHandlers device_info_scene_handlers;

// Add to app struct:
SceneManager* scene_manager;
```

### Scene Handlers Configuration

Here's how to properly initialize scene handlers based on the example BLE beacon application.

1. First, create a scene configuration header (`scene_config.h`):

```c:scene_config.h
ADD_SCENE(device_info, main, Main)
ADD_SCENE(device_info, details, Details)
ADD_SCENE(device_info, system, System)
```

2. Create a scenes header (`scenes.h`):

```c:scenes.h
#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) DeviceInfoScene##id,
typedef enum {
#include "scene_config.h"
    DeviceInfoSceneNum,
} DeviceInfoScene;
#undef ADD_SCENE

extern const SceneManagerHandlers device_info_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "scene_config.h"
#undef ADD_SCENE
```

3. Create a scenes implementation file (`scenes.c`):

```c:scenes.c
#include "scenes.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const device_info_on_enter_handlers[])(void*) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const device_info_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const device_info_on_exit_handlers[])(void* context) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers device_info_scene_handlers = {
    .on_enter_handlers = device_info_on_enter_handlers,
    .on_event_handlers = device_info_on_event_handlers,
    .on_exit_handlers = device_info_on_exit_handlers,
    .scene_num = DeviceInfoSceneNum,
};
```

4. In your main application initialization, set up the scene manager:

```c
// In your app allocation function
app->scene_manager = scene_manager_alloc(&device_info_scene_handlers, app);
```

The key benefits of this approach are:

1. **Automatic Handler Generation**: The macros automatically generate all necessary handler arrays
2. **Type Safety**: Scene IDs are generated as an enum
3. **Maintainability**: Adding a new scene only requires adding one line to `scene_config.h`
4. **Consistency**: All scene handlers follow the same pattern

### Key Concepts Explained:

1. **Scenes**: Represent different screens or states in the app
2. **Scene Manager**: Handles transitions between scenes
3. **Scene Stack**: Maintains navigation history
4. **Scene Handlers**: Functions that handle scene lifecycle events

### Advanced Exercises:

1. Add a third scene
2. Implement a menu system
3. Add transition effects between scenes
4. Save and restore scene state

## Step 4: Scene Implementation

Let's implement the basic scene handlers.

```c
// device_info_scene_main.c
void device_info_scene_main_on_enter(void* context) {
    DeviceInfoApp* app = context;
    // Setup for main scene
}

bool device_info_scene_main_on_event(void* context, SceneManagerEvent event) {
    DeviceInfoApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        // Handle custom events
        consumed = true;
    }

    return consumed;
}

void device_info_scene_main_on_exit(void* context) {
    DeviceInfoApp* app = context;
    // Cleanup main scene
}
```

### Key Concepts Explained:

1. **Scene Lifecycle**: Enter, event handling, and exit
2. **Scene Events**: Custom events vs system events
3. **Scene Context**: Accessing app state in scenes
4. **Event Consumption**: Proper event handling patterns

### Expert Exercises:

1. Implement scene state preservation
2. Add complex navigation patterns
3. Create a scene with multiple view types
4. Implement data sharing between scenes

## Step 5: ViewDispatcher Integration with Scenes

Now let's connect ViewDispatcher and Scene Manager.

```c
static bool device_info_navigation_event_callback(void* context) {
    DeviceInfoApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

// In main app initialization:
view_dispatcher_set_navigation_event_callback(
    app->view_dispatcher,
    device_info_navigation_event_callback
);
```

### Key Concepts Explained:

1. **Navigation Events**: System-wide back button handling
2. **Event Flow**: How events move through the system
3. **Navigation Patterns**: Managing scene transitions
4. **State Management**: Coordinating between views and scenes

### Master Exercises:

1. Implement complex navigation flows
2. Add modal dialogs
3. Create a plugin system
4. Add state persistence

## Step 6: Custom Events and Communication

Finally, let's implement communication between components.

```c
enum DeviceInfoCustomEvents {
    DeviceInfoCustomEventRefresh,
    DeviceInfoCustomEventSave,
    // Add more events as needed
};

static void send_custom_event(DeviceInfoApp* app, uint32_t event) {
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}
```

### Key Concepts Explained:

1. **Custom Events**: Application-specific events
2. **Event Routing**: How events flow through the system
3. **Component Communication**: Patterns for inter-component messaging
4. **State Synchronization**: Keeping views and scenes in sync

### Challenge Exercises:

1. Create a pub/sub event system
2. Implement undo/redo functionality
3. Add background tasks with progress updates
4. Create a complex state machine

Throughout the tutorial, focus on:

- Understanding each component's role
- Proper resource management
- Clean code organization
- Error handling
- Memory management
- User experience

Remember to:

- Test each stage thoroughly
- Handle edge cases
- Consider resource constraints
- Follow Flipper Zero UI guidelines

This step-by-step approach lets you build understanding progressively while creating a fully functional application.
