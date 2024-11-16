# Tutorial: Using the Button Menu Widget in Flipper Zero

## Overview

The Button Menu widget provides a simple way to create a menu with button-style options. It's particularly useful when you need:

- A vertical list of selectable buttons
- Custom callback handling for each button
- Visual feedback for selected/pressed states
- Support for both left and right-handed layouts

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/button_menu.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Button Items

First, let's define the button items and their callbacks:

```c
typedef struct {
    ButtonMenu* button_menu;
    // Add your application-specific state here
} MyApp;

static void button_callback(void* context, uint32_t index) {
    MyApp* app = context;
    // Handle button press based on index
    switch(index) {
        case 0:
            // Handle first button
            break;
        case 1:
            // Handle second button
            break;
    }
}
```

## Step 3: Initialize the Button Menu

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate the button menu
app->button_menu = button_menu_alloc();

// Add buttons
button_menu_add_item(
    app->button_menu,
    "Option 1",    // Text
    0,             // Index
    button_callback,
    app            // Context
);

button_menu_add_item(
    app->button_menu,
    "Option 2",
    1,
    button_callback,
    app
);
```

## Step 4: Configure Layout and Style

```c
// Set the button menu orientation (optional)
button_menu_set_orientation(app->button_menu, ButtonMenuOrientationVertical);

// Set header text (optional)
button_menu_set_header(app->button_menu, "Select Option:");
```

## Step 5: Integration with ViewDispatcher

```c
View* view = button_menu_get_view(app->button_menu);
view_dispatcher_add_view(view_dispatcher, MyButtonMenuView, view);
```

## Example Application

Here's a complete example that creates a button menu with two options:

```c
typedef struct {
    ButtonMenu* button_menu;
    ViewDispatcher* view_dispatcher;
} ButtonMenuDemo;

static void button_callback(void* context, uint32_t index) {
    ButtonMenuDemo* app = context;
    if(index == 0) {
        // First button pressed
        FURI_LOG_I("Demo", "Button 1 pressed");
    } else if(index == 1) {
        // Second button pressed
        FURI_LOG_I("Demo", "Button 2 pressed");
    }
}

int32_t button_menu_demo_app(void* p) {
    UNUSED(p);
    ButtonMenuDemo* app = malloc(sizeof(ButtonMenuDemo));

    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate button menu
    app->button_menu = button_menu_alloc();
    
    // Add buttons
    button_menu_add_item(
        app->button_menu,
        "Play Sound",
        0,
        button_callback,
        app
    );
    
    button_menu_add_item(
        app->button_menu,
        "Show Info",
        1,
        button_callback,
        app
    );
    
    // Set header
    button_menu_set_header(app->button_menu, "Demo Menu");
    
    // Get view and add to dispatcher
    View* view = button_menu_get_view(app->button_menu);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    button_menu_free(app->button_menu);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
    
    return 0;
}
```

## Key Features

- Vertical or horizontal button layout
- Optional header text
- Automatic button focus handling
- Support for multiple buttons
- Custom callback per button
- Built-in navigation between buttons

## Common Customization Options

1. **Change Button Layout:**
```c
button_menu_set_orientation(button_menu, ButtonMenuOrientationHorizontal);
```

2. **Set Header Text:**
```c
button_menu_set_header(button_menu, "My Menu");
```

3. **Change Selected Item:**
```c
button_menu_set_selected_item(button_menu, index);
```

## Best Practices

1. Always free resources when the application exits
2. Keep button labels concise and clear
3. Use meaningful indices for buttons
4. Handle all button callbacks appropriately
5. Consider screen space when adding multiple buttons

## Exercises

1. Create a button menu with three options that perform different actions
2. Modify the example to switch between horizontal and vertical layouts
3. Add a confirmation dialog when certain buttons are pressed
4. Implement a back button handler to exit the application

## Common Issues and Solutions

1. **Buttons not responding:**
   - Ensure callbacks are properly set
   - Verify view is added to view dispatcher

2. **Layout issues:**
   - Check orientation settings
   - Ensure button labels aren't too long

3. **Memory leaks:**
   - Always pair allocations with corresponding free calls
   - Clean up in the correct order
