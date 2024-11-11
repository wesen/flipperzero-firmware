# Flipper Zero ViewDispatcher Technical Documentation

## Introduction

The ViewDispatcher is a crucial component in the Flipper Zero firmware that manages complex user interfaces by coordinating multiple Views, handling user input events, and managing navigation between different screens. It serves as an intermediary between the application logic and the Flipper Zero's GUI system, providing a structured approach to creating multi-screen applications with sophisticated user interactions.

ViewDispatcher operates on three fundamental principles:

1. Centralized view management through a comprehensive registration system
2. Event-driven architecture with dedicated input and custom event queues
3. Hierarchical navigation with configurable callbacks for screen transitions

## Core Architecture

### Component Overview

The ViewDispatcher system consists of several interconnected components that work together to provide a cohesive UI management solution:

1. ViewDispatcher: The central manager that coordinates Views and handles events
2. Views: Individual screen components that implement specific UI functionality
3. Event Queues: Dedicated queues for handling input and custom events
4. Navigation System: Manages transitions between different Views

The ViewDispatcher maintains internal state information about:

- Currently active View
- Registered Views and their identifiers
- Input handling state
- Event callback registrations

### Memory Management

ViewDispatcher implements a robust memory management system where:

- The ViewDispatcher instance owns the registration of Views but not the Views themselves
- Views must be allocated before registration and freed after deregistration
- Event queues are automatically managed by the ViewDispatcher
- Resource cleanup follows a strict order to prevent memory leaks

## View Management

### Registration System

Views are registered with the ViewDispatcher using unique identifiers:

```c
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);
```

The registration process:

1. Validates the View and identifier haven't been previously registered
2. Sets up update callbacks for the View
3. Associates the View with the ViewDispatcher's message queue

### View Switching

View switching is handled through a controlled process that ensures proper lifecycle management:

```c
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);
```

During a view switch:

1. The current View receives an exit notification
2. The ViewDispatcher updates its internal state
3. The new View receives an enter notification
4. The display is updated to reflect the change

## Event System

### Input Event Handling

The ViewDispatcher implements a sophisticated input event handling system that:

1. Processes input events through a dedicated queue
2. Maintains input state consistency across view switches
3. Provides automatic input translation based on view orientation
4. Handles input event complementarity (press/release pairs)

### Custom Events

Custom events provide a flexible mechanism for application-specific notifications:

```c
void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);
```

Custom events are:

1. Queued for asynchronous processing
2. Delivered first to the current View
3. Forwarded to the registered callback if unhandled by the View

## Navigation System

### Navigation Events

The navigation system provides structured handling of back button events:

1. Events are first sent to the current View
2. If unhandled, they trigger the navigation callback
3. Navigation callbacks can implement complex navigation logic
4. Return values determine whether to stop the ViewDispatcher

### Navigation Patterns

Common navigation patterns include:

1. Linear Navigation: Simple back/forward movement between Views
2. Hierarchical Navigation: Tree-structured navigation with parent/child relationships
3. Modal Navigation: Temporary View presentation with forced return paths

## Integration Guide

### GUI Integration

ViewDispatcher integration with the Flipper Zero GUI system requires careful setup:

```c
void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type);
```

The integration process:

1. Associates the ViewDispatcher with a GUI instance
2. Configures the display layer type
3. Establishes event routing between systems

### Event Loop Integration

The ViewDispatcher provides access to its event loop for additional integrations:

```c
FuriEventLoop* view_dispatcher_get_event_loop(ViewDispatcher* view_dispatcher);
```

This allows:

1. Integration of additional event sources
2. Custom timer management
3. Synchronization with other system components

## Best Practices

### View Organization

Organize Views effectively by:

1. Using meaningful view identifiers
2. Implementing consistent navigation patterns
3. Managing view lifecycle events properly
4. Maintaining clear separation of concerns

### Event Handling

Follow these event handling guidelines:

1. Process events efficiently to maintain UI responsiveness
2. Implement proper event consumption rules
3. Use custom events judiciously
4. Handle input complementarity correctly

### Resource Management

Ensure proper resource management by:

1. Freeing Views after removing them from the ViewDispatcher
2. Cleaning up custom event handlers
3. Properly detaching from the GUI system
4. Managing view-specific resources appropriately

## Conclusion

The ViewDispatcher system provides a robust foundation for building complex user interfaces on the Flipper Zero platform. By understanding its architecture and following the prescribed patterns for view management, event handling, and navigation, developers can create sophisticated applications that provide excellent user experiences while maintaining code maintainability.

Success with ViewDispatcher requires careful attention to:

1. Proper resource management
2. Efficient event handling
3. Consistent navigation patterns
4. Clear separation of concerns

When implemented correctly, ViewDispatcher facilitates the creation of complex, maintainable UI systems that provide excellent user experiences on the Flipper Zero platform.
