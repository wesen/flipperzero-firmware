# Submenu Widget Tutorial

## Overview

The Submenu widget provides a vertical list-based menu interface with optional header text. It's ideal for creating settings menus, option lists, and hierarchical navigation structures. The widget supports scrolling, selection highlighting, and callback-based interaction.

## Prerequisites

- Basic understanding of Flipper Zero's GUI framework
- Familiarity with view management and callbacks

## Key Features

1. Text-based menu items with automatic scrolling
2. Optional header text
3. Supports 3-4 visible items at once (3 with header, 4 without)
4. Built-in scrollbar for navigation
5. Highlight-based selection
6. Dynamic item management (add/modify/reset)
7. Built-in back navigation through exit callback

## Navigation Handling

The submenu widget automatically handles back navigation through its exit callback. You do not need to implement a separate back button callback. Simply set up the exit callback to return the view you want to switch to when the back button is pressed:

```c
static uint32_t submenu_exit_callback(void* context) {
    UNUSED(context);
    return PreviousViewId;  // Return the view to switch to
}

// Set the callback
view_set_previous_callback(submenu_get_view(app->submenu), submenu_exit_callback);
```

## Basic Implementation

### Header Inclusion

```c
#include <gui/modules/submenu.h>
```

## Struct Definition

```c
typedef struct {
    // Your application state variables
    Submenu* submenu;
    ViewDispatcher* view_dispatcher;
} YourApp;
```

## Initialization

```c
// Allocate submenu
app->submenu = submenu_alloc();

// Optional: Set header text
submenu_set_header(app->submenu, "Settings");

// Add menu items
submenu_add_item(
    app->submenu,
    "Option 1",
    0,  // item index
    your_callback_function,
    your_context);

submenu_add_item(
    app->submenu,
    "Option 2",
    1,  // item index
    your_callback_function,
    your_context);
```

## ViewDispatcher Integration

```c
// Add view to ViewDispatcher
view_dispatcher_add_view(
    app->view_dispatcher,
    YourAppViewSubmenu,  // your view ID
    submenu_get_view(app->submenu));
```

## Complete Example Application

```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

typedef struct {
    Submenu* submenu;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
} ExampleApp;

typedef enum {
    ExampleAppViewSubmenu,
    // Add other views here
} ExampleAppView;

static void submenu_callback(void* context, uint32_t index) {
    ExampleApp* app = context;
    // Handle menu selection based on index
    switch(index) {
    case 0:
        // Handle Option 1
        break;
    case 1:
        // Handle Option 2
        break;
    }
}

static ExampleApp* example_app_alloc() {
    ExampleApp* app = malloc(sizeof(ExampleApp));

    // Initialize GUI and ViewDispatcher
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize Submenu
    app->submenu = submenu_alloc();
    submenu_set_header(app->submenu, "Example Menu");

    // Add menu items
    submenu_add_item(app->submenu, "Option 1", 0, submenu_callback, app);
    submenu_add_item(app->submenu, "Option 2", 1, submenu_callback, app);
    submenu_add_item(app->submenu, "Option 3", 2, submenu_callback, app);

    // Add submenu view
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleAppViewSubmenu,
        submenu_get_view(app->submenu));

    return app;
}

static void example_app_free(ExampleApp* app) {
    // Remove and free views
    view_dispatcher_remove_view(app->view_dispatcher, ExampleAppViewSubmenu);
    submenu_free(app->submenu);

    // Free ViewDispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t example_app_main(void* p) {
    UNUSED(p);
    ExampleApp* app = example_app_alloc();

    // Set starting view
    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleAppViewSubmenu);

    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);

    // Free resources
    example_app_free(app);
    return 0;
}
```

## Customization Options

### Setting a Header

```c
submenu_set_header(submenu, "Your Header");
```

### Changing Item Labels

```c
submenu_change_item_label(submenu, item_index, "New Label");
```

### Setting Selected Item

```c
submenu_set_selected_item(submenu, item_index);
```

## Best Practices

1. Use meaningful item indices for easy identification in callbacks
2. Keep menu items concise and clear
3. Group related options together
4. Use headers to provide context
5. Consider navigation depth when designing menu hierarchy

## Common Use Cases

1. Settings menus
2. Configuration options
3. Feature selection
4. Multi-level navigation
5. List-based selections

## Common Issues and Solutions

1. **Issue**: Menu items not visible
   **Solution**: Ensure proper initialization and item addition

2. **Issue**: Callback not triggering
   **Solution**: Verify callback function registration and context passing

3. **Issue**: Incorrect item selection
   **Solution**: Check item indices and selection logic

## Exercises

1. Create a settings menu with multiple options
2. Implement submenu navigation between different views
3. Add dynamic item label updates based on state
4. Create a nested menu structure

## Advanced Usage

### Dynamic Menu Population

```c
// Clear all items
submenu_reset(submenu);

// Add new items dynamically
for(uint32_t i = 0; i < item_count; i++) {
    submenu_add_item(
        submenu,
        get_item_label(i),
        i,
        item_callback,
        context);
}
```

### State-Based Label Updates

```c
void update_item_label(Submenu* submenu, uint32_t index, bool state) {
    submenu_change_item_label(
        submenu,
        index,
        state ? "Option: ON" : "Option: OFF");
}
```

## User Experience Tips

1. Keep menu structures shallow (avoid deep nesting)
2. Use consistent naming conventions
3. Provide visual feedback for selections
4. Group related items together
5. Use clear and concise labels
6. Add headers for context when appropriate

## Menu Entry Management

### Adding Menu Entries

Menu entries consist of a label, index, callback function, and context. Here's how to add them:

```c
// Single menu entry
submenu_add_item(
    submenu,
    "Menu Item",     // Display label
    item_index,      // Unique identifier
    callback_fn,     // Handler function
    context);        // Usually your app instance
```

### Callback Structure

Your callback function should follow this pattern:

```c
static void menu_callback(void* context, uint32_t index) {
    YourApp* app = context;
    // Handle selection based on index
    switch(index) {
        case YourViewId1:
            // Handle first option
            break;
        case YourViewId2:
            // Handle second option
            break;
    }
}
```

### Real-World Example

The display test app demonstrates a clean implementation of menu handling. Let's break it down:

1. First, define your view IDs:

```29:33:applications/debug/display_test/display_test.c
typedef enum {
    DisplayTestViewSubmenu,
    DisplayTestViewConfigure,
    DisplayTestViewDisplayTest,
} DisplayTestView;

```

2. Create a callback handler:

```65:68:applications/debug/display_test/display_test.c
static void display_test_submenu_callback(void* context, uint32_t index) {
    DisplayTest* instance = (DisplayTest*)context;
    view_dispatcher_switch_to_view(instance->view_dispatcher, index);
}
```

3. Initialize and populate the menu:

```175:190:applications/debug/display_test/display_test.c
    // Menu
    instance->submenu = submenu_alloc();
    view = submenu_get_view(instance->submenu);
    view_set_previous_callback(view, display_test_exit_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewSubmenu, view);
    submenu_add_item(
        instance->submenu,
        "Test",
        DisplayTestViewDisplayTest,
        display_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu,
        "Configure",
        DisplayTestViewConfigure,
        display_test_submenu_callback,
```

### Best Practices for Menu Implementation

1. **View Management**

- Always pair menu creation with proper cleanup
- Use view dispatcher for handling view switches
- Set appropriate exit callbacks

Example cleanup:

```c
void your_app_free(YourApp* app) {
    // Remove view first
    view_dispatcher_remove_view(app->view_dispatcher, YourMenuViewId);
    // Then free the submenu
    submenu_free(app->submenu);
    // Finally clean up view dispatcher
    view_dispatcher_free(app->view_dispatcher);
}
```

2. **Navigation Flow**
   Set up proper navigation callbacks for back button handling:

```c
static uint32_t previous_view_callback(void* context) {
    // Return to previous view or exit
    return PreviousViewId;  // or VIEW_NONE to exit
}

// Add to view setup
view_set_previous_callback(view, previous_view_callback);
```

3. **Context Management**

- Pass your app instance as context for callbacks
- Use proper typecasting in callbacks
- Keep menu indices aligned with view IDs for consistency

### Advanced Menu Patterns

1. **Dynamic Menu Updates**

```c
// Update menu item text
submenu_change_item_label(submenu, item_index, "New Label");

// Clear and rebuild menu
submenu_reset(submenu);
// Add new items...
```

2. **State-Based Navigation**

```c
static void menu_callback(void* context, uint32_t index) {
    YourApp* app = context;

    if (app->current_state == StateReady) {
        view_dispatcher_switch_to_view(app->view_dispatcher, index);
    } else {
        // Handle invalid state
    }
}
```

3. **Hierarchical Menus**
   For nested menus, create separate submenu instances for each level:

```c
typedef struct {
    Submenu* main_menu;
    Submenu* settings_menu;
    ViewDispatcher* view_dispatcher;
} YourApp;

// Switch between menus
static void main_menu_callback(void* context, uint32_t index) {
    YourApp* app = context;
    if (index == SettingsMenuIndex) {
        view_dispatcher_switch_to_view(
            app->view_dispatcher,
            SettingsMenuViewId);
    }
}
