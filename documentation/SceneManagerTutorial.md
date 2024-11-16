# Scene Manager Tutorial for Flipper Zero Applications

This tutorial explains how to use the Scene Manager in Flipper Zero applications, using practical examples from the SubGhz Test application.

## What is Scene Manager?

Scene Manager is a component of the Flipper Zero GUI framework that helps manage different screens (scenes) in your application. It handles:
- Scene transitions
- Scene state management
- Event handling for each scene
- Scene lifecycle (enter/exit)

## Basic Concepts

### 1. Scene Structure

Each scene in your application typically consists of three main handlers:

```c
// Called when entering the scene
void on_enter_handler(void* context);

// Called to handle events in the scene
bool on_event_handler(void* context, SceneManagerEvent event);

// Called when exiting the scene
void on_exit_handler(void* context);
```

### 2. Scene Manager Setup

To use Scene Manager, you need to:

1. Define your scenes enum
2. Create handlers for each scene
3. Initialize Scene Manager with handlers
4. Set up scene transitions

## Implementation Guide

### 1. Define Scene IDs

First, define an enum with all your scenes:

```c
// yourapp_i.h
typedef enum {
    YourAppSceneStart,    // Initial scene
    YourAppSceneMenu,     // Menu scene
    YourAppSceneConfig,   // Configuration scene
    // ... other scenes ...
    YourAppSceneNum      // Must be last - total number of scenes
} YourAppScene;
```

### 2. Create Scene Handlers

For each scene, create the three handler functions:

```c
// scenes/yourapp_scene_start.c
void yourapp_scene_start_on_enter(void* context) {
    YourApp* app = context;
    // Initialize your scene here
}

bool yourapp_scene_start_on_event(void* context, SceneManagerEvent event) {
    YourApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case YourCustomEvent:
            // Handle custom event
            scene_manager_next_scene(app->scene_manager, YourAppSceneMenu);
            consumed = true;
            break;
        }
    }

    return consumed;
}

void yourapp_scene_start_on_exit(void* context) {
    YourApp* app = context;
    // Clean up scene here
}
```

### 3. Initialize Scene Manager

In your application's initialization:

```c
// yourapp.c
static const SceneManagerHandlers app_scene_handlers = {
    .on_enter_handlers = {
        [YourAppSceneStart] = yourapp_scene_start_on_enter,
        [YourAppSceneMenu] = yourapp_scene_menu_on_enter,
        // ... other scenes ...
    },
    .on_event_handlers = {
        [YourAppSceneStart] = yourapp_scene_start_on_event,
        [YourAppSceneMenu] = yourapp_scene_menu_on_event,
        // ... other scenes ...
    },
    .on_exit_handlers = {
        [YourAppSceneStart] = yourapp_scene_start_on_exit,
        [YourAppSceneMenu] = yourapp_scene_menu_on_exit,
        // ... other scenes ...
    },
    .scene_num = YourAppSceneNum,
};

YourApp* yourapp_alloc() {
    YourApp* app = malloc(sizeof(YourApp));
    
    // Initialize scene manager
    app->scene_manager = scene_manager_alloc(&app_scene_handlers, app);
    
    return app;
}
```

### 4. Scene Navigation

To navigate between scenes:

```c
// Go to next scene
scene_manager_next_scene(app->scene_manager, YourAppSceneMenu);

// Go back to previous scene
scene_manager_previous_scene(app->scene_manager);

// Search and switch to scene
scene_manager_search_and_switch_to_another_scene(app->scene_manager, YourAppSceneStart);
```

### 5. Scene State Management

You can store and retrieve scene-specific state:

```c
// Store state
scene_manager_set_scene_state(app->scene_manager, YourAppSceneMenu, your_state_value);

// Retrieve state
uint32_t state = scene_manager_get_scene_state(app->scene_manager, YourAppSceneMenu);
```

## Event Handling

Scene Manager supports three types of events:

1. **Custom Events** (`SceneManagerEventTypeCustom`):
   - Application-specific events
   - Handled using `scene_manager_handle_custom_event()`

2. **Back Events** (`SceneManagerEventTypeBack`):
   - System back button press
   - Handled using `scene_manager_handle_back_event()`

3. **Tick Events** (`SceneManagerEventTypeTick`):
   - Regular timer events
   - Handled using `scene_manager_handle_tick_event()`

## Clean Up

Don't forget to free the Scene Manager when your application exits:

```c
void yourapp_free(YourApp* app) {
    scene_manager_free(app->scene_manager);
    free(app);
}
```

## Best Practices

1. Always handle memory management properly
2. Use meaningful scene names and states
3. Keep scene handlers focused and simple
4. Handle all relevant events in each scene
5. Clean up resources in the exit handler
6. Use scene states to maintain scene-specific data

## Example from SubGhz Test

The SubGhz Test application demonstrates these concepts well. Here's a simplified version of its scene structure:

```c
typedef enum {
    SubGhzTestSceneStart,
    SubGhzTestScenePacketTest,
    // ... other scenes ...
} SubGhzTestScene;

// Scene handlers example
void subghz_test_scene_start_on_enter(void* context) {
    SubGhzTest* app = context;
    // Initialize view
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewStart);
}

bool subghz_test_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhzTest* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubGhzTestCustomEventViewStartPress:
            scene_manager_next_scene(app->scene_manager, SubGhzTestScenePacketTest);
            consumed = true;
            break;
        }
    }
    return consumed;
}
```

This tutorial should give you a solid foundation for using Scene Manager in your Flipper Zero applications. Remember to refer to the API documentation in `scene_manager.h` for detailed information about available functions.
