# Flipper Zero GUI Development Tutorial

## 1. Core Concepts

### 1.1 View System Architecture

The GUI system uses a layered architecture:

```
┌─────────────────────────┐
│     Application Logic   │
├─────────────────────────┤
│      View Manager      │  ViewDispatcher/ViewHolder/ViewStack
├─────────────────────────┤
│         Views          │  Menu, Dialog, TextBox etc.
├─────────────────────────┤
│       ViewPorts        │  Raw drawing & input
├─────────────────────────┤
│        Canvas          │  Graphics primitives
└─────────────────────────┘
```

### 1.2 ViewPort

ViewPort is the basic building block that:

- Represents a rectangular area on screen
- Handles drawing through callbacks
- Receives input events
- Can be positioned and layered

Basic ViewPort example:

```c
// Create viewport
ViewPort* view_port = view_port_alloc();

// Set up drawing callback
void draw_callback(Canvas* canvas, void* context) {
    canvas_draw_str(canvas, 0, 10, "Hello");
}
view_port_draw_callback_set(view_port, draw_callback, context);

// Set up input callback
void input_callback(InputEvent* event, void* context) {
    if(event->key == InputKeyOk) {
        // Handle OK button
    }
}
view_port_input_callback_set(view_port, input_callback, context);

// Add to GUI
gui_add_view_port(gui, view_port, GuiLayerFullscreen);
```

Key ViewPort functions:

```c
// Creation/deletion
ViewPort* view_port_alloc(void);
void view_port_free(ViewPort* view_port);

// Callbacks
void view_port_draw_callback_set(ViewPort*, ViewPortDrawCallback, void* context);
void view_port_input_callback_set(ViewPort*, ViewPortInputCallback, void* context);

// Enable/disable
void view_port_enabled_set(ViewPort*, bool enabled);

// Force redraw
void view_port_update(ViewPort*);
```

### Quiz 1: ViewPort Basics

1. What are the two main callback types for a ViewPort?
2. How do you force a ViewPort to redraw its contents?
3. What happens if you free a ViewPort that's still attached to the GUI?
4. In which order should you:
   a) Set callbacks
   b) Add to GUI
   c) Allocate ViewPort
   d) Free ViewPort

### Exercises 1: ViewPort Programming

1. Create a ViewPort that displays a counter and increases it when UP is pressed
2. Make a ViewPort that alternates between two text strings when OK is pressed
3. Create two ViewPorts that divide the screen horizontally and draw different content

## 2. Canvas Drawing

The Canvas API provides drawing primitives. All drawing happens in ViewPort draw callbacks.

### 2.1 Basic Drawing

```c
void draw_callback(Canvas* canvas, void* context) {
    // Clear the canvas
    canvas_clear(canvas);

    // Draw text
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Hello");

    // Draw shapes
    canvas_draw_line(canvas, 0, 0, 10, 10);
    canvas_draw_rect(canvas, 20, 20, 30, 30);
    canvas_draw_circle(canvas, 50, 50, 10);
}
```

### 2.2 Text Rendering

```c
// Font selection
canvas_set_font(canvas, FontPrimary);    // Large font
canvas_set_font(canvas, FontSecondary);  // Medium font
canvas_set_font(canvas, FontKeyboard);   // Small font

// Text alignment
canvas_draw_str_aligned(
    canvas,
    64,    // x center point
    32,    // y center point
    AlignCenter,  // horizontal alignment
    AlignCenter,  // vertical alignment
    "Centered Text"
);

// Multi-line text
canvas_draw_str_multiline(
    canvas,
    0,
    0,
    "Line 1\nLine 2\nLine 3"
);
```

### 2.3 Drawing State

```c
// Colors (black or white)
canvas_set_color(canvas, ColorBlack);
canvas_set_color(canvas, ColorWhite);

// Drawing modes
canvas_set_bitmap_mode(canvas, true);   // Transparent
canvas_set_bitmap_mode(canvas, false);  // Opaque
```

### Quiz 2: Canvas Drawing

1. What are the three built-in font types?
2. How do you measure text width before drawing?
3. What's the difference between draw_rect() and draw_frame()?
4. What coordinate system does Canvas use? Where is (0,0)?

### Exercises 2: Canvas Drawing

1. Draw a battery icon with charge level indicator
2. Create a digital clock display with large numbers
3. Make a loading spinner animation using circles
4. Draw a graph with axes and data points

## 3. View Modules

The GUI system provides ready-to-use view modules for common UI patterns.

### 3.1 Dialog

Simple modal dialog:

```c
DialogEx* dialog = dialog_ex_alloc();

// Configure dialog
dialog_ex_set_header(dialog, "Title", 64, 0, AlignCenter, AlignTop);
dialog_ex_set_text(dialog, "Message text", 64, 32, AlignCenter, AlignCenter);
dialog_ex_set_left_button_text(dialog, "Cancel");
dialog_ex_set_right_button_text(dialog, "OK");

// Set result callback
void dialog_callback(DialogExResult result, void* context) {
    if(result == DialogExResultRight) {
        // OK pressed
    }
}
dialog_ex_set_result_callback(dialog, dialog_callback, context);
```

### 3.2 Menu

Scrollable menu with items:

```c
Submenu* submenu = submenu_alloc();

// Add menu items
submenu_add_item(
    submenu,
    "Menu Item 1",  // Label
    1,              // Index
    callback,       // Callback
    context         // Context
);

// Set description
submenu_set_header(submenu, "Menu Title");
```

### 3.3 Text Input

Text input with keyboard:

```c
TextInput* text_input = text_input_alloc();

// Configure
text_input_set_header_text(text_input, "Enter Text:");

// Set result callback
void input_callback(void* context) {
    char* text = text_input_get_text(text_input);
    // Use entered text
}
text_input_set_result_callback(
    text_input,
    input_callback,
    context
);
```

### Quiz 3: View Modules

1. What callbacks are available for DialogEx?
2. How do you get the selected item index from a Submenu?
3. What's the difference between Submenu and Menu?
4. How do you limit text input length?

### Exercises 3: View Modules

1. Create a settings menu with multiple options
2. Make a dialog with custom buttons and icon
3. Build a text input form with validation
4. Combine menu and dialog for confirmation

---

# 1. Core Concepts

## Concept Overview

The Flipper Zero GUI system uses a layered architecture that separates concerns from low-level drawing to high-level application logic. At its foundation is the ViewPort - a fundamental building block that represents a rectangular area on screen. Think of ViewPorts like windows in a desktop operating system - they define where content appears and how it responds to input. Above ViewPorts are Views, which encapsulate UI logic and state. Views are managed by components like ViewDispatcher that handle navigation and state management. This layered approach allows developers to work at the appropriate level of abstraction for their needs.

## Example Breakdown: Basic ViewPort

```c
ViewPort* view_port = view_port_alloc();

void draw_callback(Canvas* canvas, void* context) {
    canvas_draw_str(canvas, 0, 10, "Hello");
}
view_port_draw_callback_set(view_port, draw_callback, context);

void input_callback(InputEvent* event, void* context) {
    if(event->key == InputKeyOk) {
        // Handle OK button
    }
}
view_port_input_callback_set(view_port, input_callback, context);

gui_add_view_port(gui, view_port, GuiLayerFullscreen);
```

This example demonstrates the essential ViewPort lifecycle:

1. **Creation**: We allocate a new ViewPort using `view_port_alloc()`
2. **Drawing Setup**: We define how the ViewPort should render itself through a draw callback. This function receives a Canvas for drawing operations.
3. **Input Handling**: We set up input handling through an input callback that receives button events.
4. **GUI Integration**: Finally, we add the ViewPort to the GUI system so it becomes visible.

The example is intentionally simple but shows the core pattern used by all ViewPort-based UIs. The separation between drawing and input handling demonstrates the system's event-driven nature.

# 2. Canvas Drawing

## Concept Overview

The Canvas is the Flipper Zero's drawing API that provides primitive operations for rendering text, shapes, and images. It uses a monochrome display model where each pixel can be either black or white. The Canvas operates on a coordinate system where (0,0) is at the top-left corner, with x increasing to the right and y increasing downward. All drawing operations happen within ViewPort draw callbacks, making the Canvas the foundation for all visual output.

## Example Breakdown: Basic Drawing

```c
void draw_callback(Canvas* canvas, void* context) {
    // Clear the canvas
    canvas_clear(canvas);

    // Draw text
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Hello");

    // Draw shapes
    canvas_draw_line(canvas, 0, 0, 10, 10);
    canvas_draw_rect(canvas, 20, 20, 30, 30);
    canvas_draw_circle(canvas, 50, 50, 10);
}
```

This example shows the basic Canvas operations:

1. **Canvas Clearing**: We start by clearing any previous content with `canvas_clear()`
2. **Text Rendering**: We demonstrate text drawing by:
   - Setting an appropriate font
   - Drawing text at specific coordinates
3. **Shape Drawing**: We show various primitive shapes:
   - Lines using start/end coordinates
   - Rectangles using position and size
   - Circles using center point and radius

The example progresses from simple to more complex operations to demonstrate the building blocks of UI rendering.

## Example Breakdown: Text Alignment

```c
canvas_draw_str_aligned(
    canvas,
    64,    // x center point
    32,    // y center point
    AlignCenter,  // horizontal alignment
    AlignCenter,  // vertical alignment
    "Centered Text"
);
```

This example shows advanced text positioning:

1. **Reference Point**: We use screen center coordinates (64,32)
2. **Alignment**: We specify both horizontal and vertical alignment relative to this point
3. **Result**: The text appears centered both horizontally and vertically

This pattern is used frequently when building UIs that need precise text positioning.

Each example builds on previous concepts while introducing new capabilities. The progression helps developers understand both the individual features and how they work together in real applications. The focus is on practical, reusable patterns rather than abstract concepts.

---

# 3. View Modules

## Concept Overview

View Modules are pre-built, reusable UI components that implement common interaction patterns. They abstract away the complexity of ViewPort management and drawing, providing a higher-level interface for building applications. Each module handles its own state management, input processing, and rendering, while maintaining a consistent look and feel across applications. This standardization not only speeds up development but also ensures a cohesive user experience.

## Example Breakdown: Dialog

```c
DialogEx* dialog = dialog_ex_alloc();

// Configure dialog
dialog_ex_set_header(dialog, "Title", 64, 0, AlignCenter, AlignTop);
dialog_ex_set_text(dialog, "Message text", 64, 32, AlignCenter, AlignCenter);
dialog_ex_set_left_button_text(dialog, "Cancel");
dialog_ex_set_right_button_text(dialog, "OK");

// Set result callback
void dialog_callback(DialogExResult result, void* context) {
    if(result == DialogExResultRight) {
        // OK pressed
    }
}
dialog_ex_set_result_callback(dialog, dialog_callback, context);
```

This example demonstrates a modal dialog:

1. **Creation**: Allocate the dialog component
2. **Content Setup**: Configure visual elements in a declarative way
   - Header text and position
   - Main message content
   - Button labels
3. **Interaction**: Define callback for handling user choices
   - Different result types for each button
   - Context passing for state management

## Example Breakdown: Menu

```c
Submenu* submenu = submenu_alloc();

submenu_add_item(
    submenu,
    "Menu Item 1",  // Label
    1,              // Index
    callback,       // Callback
    context         // Context
);

submenu_set_header(submenu, "Menu Title");
```

This menu example shows:

1. **Component Creation**: Allocate menu structure
2. **Item Management**: Add entries with:
   - Display text
   - Unique identifier
   - Selection handler
3. **Visual Customization**: Set optional elements like header

The menu automatically handles:

- Scrolling for many items
- Selection highlighting
- Input navigation

# 4. View Management

## Concept Overview

View Management provides the framework for organizing and transitioning between different screens or UI states in an application. The three main options (ViewHolder, ViewDispatcher, ViewStack) offer increasing levels of complexity and flexibility. ViewHolder is perfect for single-screen apps, ViewDispatcher handles multiple independent views with navigation, and ViewStack allows layering views for modals and overlays.

## Example Breakdown: ViewHolder

```c
typedef struct {
    ViewHolder* view_holder;
    View* my_view;
    bool running;
} MyApp;

void back_callback(void* context) {
    MyApp* app = context;
    app->running = false;
}

void app_run(MyApp* app) {
    app->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(app->view_holder, app->gui);
    view_holder_set_back_callback(app->view_holder, back_callback, app);
    view_holder_set_view(app->view_holder, app->my_view);

    while(app->running) {
        furi_delay_ms(100);
    }
}
```

This example demonstrates:

1. **Application Structure**: Define app context and state
2. **View Management**: Simple single-view setup
3. **Navigation**: Basic back button handling
4. **Main Loop**: Simple event processing

## Example Breakdown: ViewDispatcher

```c
typedef enum {
    MyAppViewMenu,
    MyAppViewSettings,
} MyAppView;

bool app_navigation_callback(void* context) {
    MyApp* app = context;
    return false; // Allow default back behavior
}

bool app_custom_callback(void* context, uint32_t event) {
    MyApp* app = context;
    switch(event) {
        case MyCustomEventShowSettings:
            view_dispatcher_switch_to_view(
                app->view_dispatcher,
                MyAppViewSettings
            );
            return true;
    }
    return false;
}
```

This example shows:

1. **View Organization**: Enumerated view identifiers
2. **Navigation Control**: Custom navigation logic
3. **Event Handling**: Processing of custom events
4. **View Switching**: Explicit view transitions

The structure emphasizes:

- Clear view identification
- Centralized navigation control
- Event-driven architecture
- State management

# 5. Scene Management

## Concept Overview

Scene Management builds on top of view management to provide a more structured approach to application flow and state transitions. Scenes represent discrete application states with well-defined entry points, exit conditions, and transition logic. This pattern is particularly useful for complex applications where different screens need to share data and coordinate their behavior.

## Example Breakdown: Scene Setup

```c
typedef enum {
    SceneMain,
    SceneMenu,
    SceneSettings,
    SceneTotal
} SceneIndex;

void scene_main_on_enter(void* context) {
    MyApp* app = context;
    // Setup main scene
}

bool scene_main_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    switch(event.type) {
        case SceneManagerEventTypeCustom:
            // Handle custom events
            break;
        case SceneManagerEventTypeBack:
            // Handle back button
            return scene_manager_search_and_switch_to_another_scene(
                app->scene_manager,
                SceneMenu
            );
    }
    return false;
}
```

This example demonstrates:

1. **Scene Definition**: Enumerate possible application states
2. **Lifecycle Handlers**: Define enter/exit/event behaviors
3. **Event Processing**: Handle navigation and custom events
4. **State Transitions**: Manage scene switching logic

The structure provides:

- Clear state boundaries
- Consistent lifecycle management
- Centralized navigation control
- Clean separation of concerns

---

# Questions and Exercises

## 3. View Modules Quiz

1. **Basic Understanding**

   - What is the main difference between DialogEx and Dialog?
   - How can you update menu items dynamically?
   - What happens if you free a view module while it's being displayed?
   - Name three built-in view modules and their primary use cases.

2. **Implementation Details**
   - How do you handle different button combinations in DialogEx?
   - What's the maximum number of items a Submenu can hold?
   - How do you implement custom drawing in a TextBox?
   - How does text input validation work?

## View Modules Exercises

1. **Basic Dialog**

   ```c
   // Create a dialog that:
   // - Shows a custom icon
   // - Has three buttons
   // - Updates its message based on button presses
   ```

2. **Dynamic Menu**

   ```c
   // Create a menu that:
   // - Loads items from storage
   // - Allows adding/removing items
   // - Shows item count in header
   ```

3. **Complex Form**

   ```c
   // Build a form with:
   // - Multiple text inputs
   // - Input validation
   // - Save/cancel options
   ```

4. **Custom Module**
   - Create a new view module for a slider control
   - Implement value change animation
   - Add haptic feedback

## 4. View Management Quiz

1. **Architecture**

   - When would you use ViewStack instead of ViewDispatcher?
   - How do Views communicate in ViewDispatcher?
   - What's the lifecycle of a View in ViewHolder?
   - How do you handle orientation changes?

2. **Implementation**
   - How do you pass data between Views?
   - What's the proper way to clean up Views?
   - How do you handle view transitions?
   - When should you update a View's state?

## View Management Exercises

1. **ViewHolder Application**

   ```c
   // Create an app that:
   // - Uses ViewHolder
   // - Implements custom drawing
   // - Handles all input types
   // - Manages state properly
   ```

2. **Multi-View Navigation**

   ```c
   // Build an app with:
   // - At least 3 different views
   // - Custom transitions
   // - Shared state
   // - Proper cleanup
   ```

3. **Advanced ViewStack**
   - Implement modal dialogs
   - Add slide-in notifications
   - Handle view persistence
   - Manage memory efficiently

## 5. Scene Management Quiz

1. **Concepts**

   - What's the difference between a Scene and a View?
   - How do you share data between Scenes?
   - When should you use Scene state vs app state?
   - How do you handle Scene transitions?

2. **Implementation**
   - How do you manage Scene history?
   - What's the proper way to handle Scene cleanup?
   - How do you implement Scene-specific settings?
   - When should you create a new Scene vs View?

## Scene Management Exercises

1. **Basic Scene Flow**

   ```c
   // Create an app with:
   // - Main menu scene
   // - Settings scene
   // - Game scene with state
   // - Proper navigation
   ```

2. **Complex Scene Management**

   ```c
   // Implement:
   // - Scene history
   // - State preservation
   // - Custom transitions
   // - Error handling
   ```

3. **Advanced Scene Features**

   ```c
   // Build:
   // - Scene queuing system
   // - Scene state management
   // - Transition animations
   // - Scene persistence
   ```

4. **Full Application**
   - Create a file browser with:
     - Folder navigation scene
     - File preview scene
     - Settings scene
     - Loading scene

## Implementation Exercise Example

Here's a starter template for one of the exercises:

```c
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* main_view;
    View* settings_view;
    View* game_view;
    SceneManager* scene_manager;
    // Add your state here
} MyApp;

void scene_main_on_enter(void* context) {
    MyApp* app = context;
    // TODO: Initialize main scene
}

bool scene_main_on_event(void* context, SceneManagerEvent event) {
    MyApp* app = context;
    bool consumed = false;

    switch(event.type) {
        case SceneManagerEventTypeCustom:
            // TODO: Handle custom events
            break;
        case SceneManagerEventTypeBack:
            // TODO: Handle back navigation
            break;
    }

    return consumed;
}

// TODO: Implement other scene handlers

int32_t my_app_main(void* p) {
    UNUSED(p);
    MyApp* app = malloc(sizeof(MyApp));

    // TODO: Initialize your application

    // Run application
    view_dispatcher_run(app->view_dispatcher);

    // TODO: Cleanup

    free(app);
    return 0;
}
```

These exercises progress from basic concepts to complex implementations. Complete them in order to build understanding incrementally.
