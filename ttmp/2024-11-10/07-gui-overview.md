# Core GUI Concepts

## Views and ViewPorts

The GUI system is built around two key abstractions:

- **ViewPort**: The fundamental drawing unit that represents a rectangular area on screen. ViewPorts handle raw drawing operations and input events. They are lightweight and can be composed together.

- **View**: A higher-level abstraction that represents a complete screen or UI component. Views typically contain application logic and state, and use ViewPorts for actual rendering. Common view modules like menus, dialogs etc. are provided.

## View Management

There are three main ways to manage views:

1. **ViewHolder**: Simple way to display a single view. Good for basic applications that only need to show one screen.

2. **ViewDispatcher**: More advanced view manager that can handle multiple views and switch between them. Provides view transitions and navigation stack.

3. **ViewStack**: Allows stacking multiple views on top of each other, useful for overlays and modal dialogs.

## Input Handling

Input events from buttons are propagated through the view hierarchy:

- ViewPorts can register input callbacks to handle button events
- Views can consume events or let them propagate up
- Back button handling is typically done at the view manager level

# Key Components

## Canvas

The Canvas API provides low-level drawing primitives:

- Basic shapes: lines, rectangles, circles
- Text rendering with multiple fonts
- Icons and bitmaps
- Drawing state like colors and fonts

## Common View Modules

Ready-to-use view implementations for common UI patterns:

- Menus and submenus
- Dialogs and popups 
- Text input/display
- Variable lists
- File browsers
- Loading screens

## Scene Management

The SceneManager helps organize complex UIs into scenes:

- Each scene represents a distinct screen/state
- Handles scene transitions and navigation
- Maintains scene history for back navigation

# Usage Patterns

## Basic Application Structure

1. Initialize GUI and view management
2. Create and configure views
3. Set up input handling
4. Run main event loop
5. Clean up on exit

```c
// Basic ViewHolder example
ViewHolder* view_holder = view_holder_alloc();
view_holder_attach_to_gui(view_holder, gui);
view_holder_set_view(view_holder, my_view);
```

## Complex Applications

For multi-screen apps:

1. Use ViewDispatcher to manage multiple views
2. Define view IDs and transitions
3. Handle custom events for navigation
4. Use SceneManager for complex flows

```c
ViewDispatcher* dispatcher = view_dispatcher_alloc();
view_dispatcher_add_view(dispatcher, ViewId1, view1); 
view_dispatcher_add_view(dispatcher, ViewId2, view2);
view_dispatcher_switch_to_view(dispatcher, ViewId1);
```

## Drawing Best Practices

- Use Canvas API for custom drawing
- Leverage provided view modules when possible
- Keep drawing code efficient
- Handle screen orientation
- Support light/dark themes

# Key APIs

## ViewPort
- Drawing callbacks
- Input handling
- Layout management

## View
- State management
- Navigation
- Custom events

## Canvas 
- Graphics primitives
- Text rendering
- Icon handling

## ViewDispatcher
- View switching
- Navigation stack
- Event handling

## SceneManager
- Scene transitions
- Navigation history
- Scene lifecycle

The GUI library provides a flexible foundation for building UIs while offering higher-level abstractions for common patterns. Applications can choose the right level of abstraction based on their complexity needs.
