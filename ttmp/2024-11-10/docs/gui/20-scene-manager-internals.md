# Scene Manager Internal Implementation

The Scene Manager is a core component of the Flipper Zero GUI system that handles scene (screen) transitions and state management. This document explains its internal implementation details.

## Core Data Structures

### SceneManager Structure
The Scene Manager maintains several key internal structures:

```c
typedef struct {
    void* context;                          // Application context
    const SceneManagerHandlers* handlers;   // Scene handlers
    SceneManagerIdStack_t scene_id_stack;   // Navigation stack
    AppScene scene[];                       // Flexible array for scene states
} SceneManager;
```

- `context`: Points to the application's context, passed to all scene callbacks
- `handlers`: Contains the scene callback functions for all scenes
- `scene_id_stack`: Stack data structure for scene navigation history
- `scene[]`: Flexible array member storing state for each scene

## Memory Management

### Allocation
The Scene Manager uses a single allocation strategy where the scene states are allocated together with the manager structure:

1. The main structure is allocated with additional space for scene states:
```c
malloc(sizeof(SceneManager) + (sizeof(AppScene) * app_scene_handlers->scene_num))
```

2. This ensures all scene states are contiguous in memory and freed together with the manager

### Scene State Storage
Each scene can maintain its own state using the `scene[]` array:
- States are accessed via scene IDs as indices
- The `scene_manager_set_scene_state()` and `scene_manager_get_scene_state()` functions provide safe access
- States persist throughout the scene's lifetime, even when not active

## Navigation System

### Scene Stack
The Scene Manager implements a stack-based navigation system:

1. The `scene_id_stack` maintains the navigation history
2. New scenes are pushed onto the stack via `scene_manager_next_scene()`
3. Back navigation pops scenes from the stack via `scene_manager_previous_scene()`
4. The top of the stack always represents the currently active scene

### Scene Transitions
During scene transitions, the following sequence occurs:

1. For scene entry:
   - Previous scene's `on_exit` handler is called (if exists)
   - New scene is pushed to the stack
   - New scene's `on_enter` handler is called

2. For scene exit:
   - Current scene's `on_exit` handler is called
   - Scene is popped from the stack
   - Previous scene's `on_enter` handler is called

## Event Handling System

The Scene Manager processes three types of events:

1. **Custom Events** (`SceneManagerEventTypeCustom`):
   - Application-specific events with custom event codes
   - Handled by the current scene's `on_event` handler
   - Return value indicates if event was consumed

2. **Back Events** (`SceneManagerEventTypeBack`):
   - Triggered when back navigation is requested
   - First handled by current scene's `on_event` handler
   - If not consumed, performs automatic back navigation

3. **Tick Events** (`SceneManagerEventTypeTick`):
   - Periodic events for animations or updates
   - Always sent to current scene's `on_event` handler
   - Return value is ignored

## Error Handling

The Scene Manager implements several safety checks:

1. Null pointer validation using `furi_check()`
2. Scene ID bounds checking against `scene_num`
3. Stack underflow prevention in navigation
4. Memory allocation validation

## Best Practices

1. **Scene State Management**:
   - Use scene states for persistent data between scene activations
   - Clear states when no longer needed
   - Don't store pointers in scene states

2. **Event Handling**:
   - Return true from `on_event` only when event is actually handled
   - Handle cleanup in `on_exit` rather than back event handler
   - Use tick events sparingly to avoid performance impact

3. **Navigation**:
   - Prefer using the stack-based navigation
   - Clear navigation stack when switching to unrelated scenes
   - Check for previous scenes before navigation when needed

## Implementation Details

The Scene Manager is implemented with these key design principles:

1. **Minimal Memory Footprint**:
   - Single allocation for manager and all scene states
   - No dynamic allocations during runtime
   - Efficient stack-based navigation

2. **Predictable Behavior**:
   - Consistent event handling order
   - Clear scene lifecycle
   - Explicit state management

3. **Error Prevention**:
   - Extensive parameter validation
   - Protected state access
   - Safe navigation operations

This implementation provides a robust foundation for building complex GUI applications while maintaining memory efficiency and predictable behavior.
