> https://claude.ai/chat/66585e10-783b-4d04-88fe-a18cd5f06d61

# Scene Manager Technical Documentation

## For Flipper Zero Application Development

### Document Planning

#### Audience Analysis

- Primary: Flipper Zero application developers
- Expected Knowledge: C programming, basic GUI concepts
- Technical Level: Intermediate to Advanced

#### Key Components

1. Scene Management Architecture
2. Scene Lifecycle
3. Navigation and State Management
4. Integration with View System
5. Event Handling
6. Best Practices

### 1. Introduction

The Scene Manager is a core architectural component of the Flipper Zero application framework, designed to handle application state transitions and UI flow management. It provides a structured approach to organizing complex applications with multiple screens, states, and navigation paths.

At its core, the Scene Manager implements the State pattern, treating each screen or major application state as a discrete "scene" with well-defined entry points, exit conditions, and transition logic. This architecture enables developers to create maintainable applications with complex navigation flows while maintaining separation of concerns.

### 2. Architectural Overview

#### 2.1 Core Components

The Scene Manager architecture consists of several key components:

1. **Scene Manager**: The central orchestrator that manages scene transitions and maintains the scene stack.
2. **Scene Handlers**: A collection of callback functions that define scene behavior:
   - `on_enter`: Called when entering a scene
   - `on_event`: Called when processing scene events
   - `on_exit`: Called when exiting a scene
3. **Scene Stack**: Maintains the navigation history and enables back navigation
4. **Scene Context**: Application-specific data shared between scenes

#### 2.2 Scene Lifecycle

Each scene follows a defined lifecycle:

1. **Creation**: Scene is registered with the Scene Manager
2. **Entry**: `on_enter` handler is called, initializing the scene
3. **Active**: Scene processes events through `on_event` handler
4. **Exit**: `on_exit` handler is called during scene transition
5. **Destruction**: Scene resources are cleaned up when removed from stack

### 3. Implementation Guide

#### 3.1 Scene Definition

Scenes are typically defined using an enumeration and configuration macros:

```c
// Scene enumeration
typedef enum {
    MyAppSceneMain,
    MyAppSceneSettings,
    MyAppSceneConfig,
    MyAppSceneNum  // Total number of scenes
} MyAppScene;

// Scene handlers declaration
typedef struct {
    void (*on_enter)(void* context);
    bool (*on_event)(void* context, SceneManagerEvent event);
    void (*on_exit)(void* context);
} MyAppSceneHandlers;
```

#### 3.2 Scene Manager Initialization

Scene Manager initialization requires setting up handlers and context:

```c
// Initialize Scene Manager
app->scene_manager = scene_manager_alloc(&app_scene_handlers, app);

// Configure default scene
scene_manager_next_scene(app->scene_manager, MyAppSceneMain);
```

#### 3.3 Scene Implementation

Each scene requires implementation of the three main handlers:

```c
void scene_main_on_enter(void* context) {
    MyApp* app = context;
    // Initialize scene UI elements
    view_dispatcher_switch_to_view(app->view_dispatcher, MyAppViewMain);
}

bool scene_main_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
            case MyCustomEventOpenSettings:
                scene_manager_next_scene(app->scene_manager, MyAppSceneSettings);
                consumed = true;
                break;
        }
    }

    return consumed;
}

void scene_main_on_exit(void* context) {
    MyApp* app = context;
    // Clean up scene resources
}
```

### 4. Navigation and State Management

#### 4.1 Scene Transitions

The Scene Manager provides several methods for navigation:

1. **Basic Navigation**

- `scene_manager_next_scene(SceneManager*, uint32_t next_scene_id)`

  - Pushes new scene onto stack
  - Calls `on_exit` for current scene
  - Calls `on_enter` for new scene
  - Used for forward navigation

- `scene_manager_previous_scene(SceneManager*)`
  - Pops current scene from stack
  - Calls `on_exit` for current scene
  - Calls `on_enter` for previous scene
  - Returns false if stack is empty
  - Used for basic back navigation

2. **Advanced Navigation**

- `scene_manager_search_and_switch_to_previous_scene(SceneManager*, uint32_t scene_id)`

  - Searches backward in stack for specific scene ID
  - If found:
    - Removes all scenes after found scene
    - Calls `on_exit` for current scene
    - Calls `on_enter` for found scene
  - Returns false if scene not found
  - Used for jumping back to specific previous scene

- `scene_manager_search_and_switch_to_previous_scene_one_of(SceneManager*, const uint32_t* scene_ids, size_t scene_ids_size)`

  - Takes array of scene IDs
  - Searches for first matching scene ID in stack
  - If found:
    - Uses `search_and_switch_to_previous_scene` to navigate
  - Returns false if no matching scenes found
  - Used for jumping back to one of several possible scenes

- `scene_manager_search_and_switch_to_another_scene(SceneManager*, uint32_t scene_id)`
  - Clears stack except first scene
  - Pushes new scene onto stack
  - Calls `on_exit` for current scene
  - Calls `on_enter` for new scene
  - Returns false if stack is empty
  - Used for resetting navigation to specific scene

3. **Helper Functions**

- `scene_manager_has_previous_scene(const SceneManager*, uint32_t scene_id)`
  - Searches backward in stack for specific scene ID
  - Returns true if scene is found in history
  - Does not modify navigation state
  - Used to check if scene exists in history

4. **Cleanup**

- `scene_manager_stop(SceneManager*)`
  - Calls `on_exit` for current scene if exists
  - Does not modify stack
  - Used for cleanup when shutting down

#### 4.2 State Management

Scene-specific state can be managed using the Scene Manager's state storage:

```c
// Store scene state
scene_manager_set_scene_state(scene_manager, scene_id, state);

// Retrieve scene state
uint32_t state = scene_manager_get_scene_state(scene_manager, scene_id);
```

### 5. Event Handling

#### 5.1 Event Types

The Scene Manager processes several types of events:

1. **Custom Events**: Application-specific events
2. **Back Events**: System back button presses
3. **Tick Events**: Periodic update events

#### 5.2 Event Processing

Events are processed through the scene's `on_event` handler:

```c
bool scene_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;

    switch(event.type) {
        case SceneManagerEventTypeCustom:
            // Handle custom events
            break;

        case SceneManagerEventTypeBack:
            // Handle back navigation
            consumed = scene_manager_search_and_switch_to_previous_scene(
                scene_manager,
                previous_scene_id
            );
            break;

        case SceneManagerEventTypeTick:
            // Handle periodic updates
            break;
    }

    return consumed;
}
```

### 6. Best Practices

#### 6.1 Scene Organization

1. **Modular Structure**: Keep each scene's code in separate files
2. **Clear Naming**: Use descriptive names for scenes and events
3. **State Isolation**: Minimize shared state between scenes
4. **Resource Management**: Clean up resources in `on_exit` handlers

#### 6.2 Navigation Flow

1. **Consistent Back Behavior**: Implement intuitive back navigation
2. **State Preservation**: Save important state before transitions
3. **Error Handling**: Handle navigation edge cases gracefully
4. **Transition Validation**: Verify scene transitions are valid

### 7. Common Patterns

#### 7.1 Modal Dialogs

Implementing modal dialogs using scenes:

```c
void show_modal_dialog(MyApp* app) {
    // Store current scene state
    uint32_t previous_state = scene_manager_get_scene_state(
        app->scene_manager,
        current_scene_id
    );

    // Switch to dialog scene
    scene_manager_next_scene(app->scene_manager, MyAppSceneDialog);

    // Pass previous state to dialog
    scene_manager_set_scene_state(
        app->scene_manager,
        MyAppSceneDialog,
        previous_state
    );
}
```

#### 7.2 Scene Transitions with Data

Passing data between scenes:

```c
// Store data for next scene
app->transition_data = data;
scene_manager_next_scene(app->scene_manager, next_scene_id);

// In next scene's on_enter
void next_scene_on_enter(void* context) {
    MyApp* app = context;
    process_transition_data(app->transition_data);
    app->transition_data = NULL;  // Clear after use
}
```

### 8. Troubleshooting

#### 8.1 Common Issues

1. **Memory Leaks**
   - Symptom: Growing memory usage
   - Solution: Ensure proper cleanup in `on_exit` handlers
2. **Navigation Loops**

   - Symptom: Infinite scene transitions
   - Solution: Implement proper navigation validation

3. **State Corruption**
   - Symptom: Unexpected application behavior
   - Solution: Use scene state storage properly

### 9. Conclusion

The Scene Manager provides a robust foundation for building complex Flipper Zero applications. By following the patterns and practices outlined in this documentation, developers can create maintainable applications with sophisticated navigation flows and state management.

Understanding the Scene Manager's architecture and lifecycle is crucial for effective Flipper Zero application development. The component-based approach, combined with clear separation of concerns, enables developers to focus on application logic while the framework handles navigation and state management complexities.

### 10. References

- Flipper Zero SDK Documentation
- Scene Manager Header Files
- Example Applications using Scene Manager

This documentation aims to provide a comprehensive understanding of the Scene Manager component. For specific implementation details, refer to the SDK documentation and example applications provided with the Flipper Zero development kit.
