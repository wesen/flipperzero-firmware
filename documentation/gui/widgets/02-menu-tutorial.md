# Tutorial: Using the Menu Widget in Flipper Zero

## Overview

The Menu widget provides a simple, scrollable list of options that users can navigate through. It's ideal for:

- Creating navigation menus
- Displaying lists of selectable items
- Building settings screens
- Implementing hierarchical navigation structures

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/menu.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Menu Structure

```c
typedef struct {
    Menu* menu;
    ViewDispatcher* view_dispatcher;
    // Add your application-specific state here
} MyApp;

// Callback function for menu items
static void menu_callback(void* context, uint32_t index) {
    MyApp* app = context;
    // Handle menu selection based on index
    switch(index) {
        case 0:
            // Handle first menu item
            break;
        case 1:
            // Handle second menu item
            break;
    }
}
```

## Step 3: Initialize the Menu

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate the menu
app->menu = menu_alloc();

// Add menu items
menu_add_item(
    app->menu,
    "First Option",     // Text
    &I_icon_1,         // Icon (optional, can be NULL)
    0,                 // Index
    menu_callback,     // Callback
    app                // Context
);

menu_add_item(
    app->menu,
    "Second Option",
    &I_icon_2,
    1,
    menu_callback,
    app
);
```

## Step 4: Integration with ViewDispatcher

```c
View* view = menu_get_view(app->menu);
view_dispatcher_add_view(view_dispatcher, MyMenuView, view);
```

## Example Application

Here's a complete example that creates a menu with three options:

```c
typedef struct {
    Menu* menu;
    ViewDispatcher* view_dispatcher;
} MenuDemo;

static void menu_callback(void* context, uint32_t index) {
    MenuDemo* app = context;
    FURI_LOG_I("Demo", "Selected menu item %lu", index);
    
    switch(index) {
        case 0:
            // Start game
            break;
        case 1:
            // Show settings
            break;
        case 2:
            // Show about
            break;
    }
}

int32_t menu_demo_app(void* p) {
    UNUSED(p);
    MenuDemo* app = malloc(sizeof(MenuDemo));

    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate menu
    app->menu = menu_alloc();
    
    // Add menu items
    menu_add_item(
        app->menu,
        "Start Game",
        &I_game_10px,  // Example icon
        0,
        menu_callback,
        app
    );
    
    menu_add_item(
        app->menu,
        "Settings",
        &I_settings_10px,  // Example icon
        1,
        menu_callback,
        app
    );
    
    menu_add_item(
        app->menu,
        "About",
        &I_info_10px,  // Example icon
        2,
        menu_callback,
        app
    );
    
    // Get view and add to dispatcher
    View* view = menu_get_view(app->menu);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    menu_free(app->menu);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
    
    return 0;
}
```

## Key Features

- Simple scrollable list interface
- Optional icons for menu items
- Automatic scrolling and navigation
- Support for multiple items
- Custom callback per menu item
- Built-in selection handling

## Common Customization Options

1. **Add Item with Icon:**
```c
menu_add_item(menu, "Item", &I_icon_10px, index, callback, context);
```

2. **Add Item without Icon:**
```c
menu_add_item(menu, "Item", NULL, index, callback, context);
```

3. **Reset Selection:**
```c
menu_reset(menu);
```

## Best Practices

1. Use clear and concise menu item labels
2. Keep icons consistent in size (typically 10x10 pixels)
3. Use meaningful indices for menu items
4. Handle all menu callbacks appropriately
5. Clean up resources when the application exits
6. Consider screen space when adding multiple items
7. Use icons consistently - either all items have icons or none do

## Exercises

1. Create a menu with five different options
2. Add custom icons for each menu item
3. Implement sub-menus using multiple views
4. Add a "back" option to return to the previous screen
5. Create a settings menu with toggleable options

## Common Issues and Solutions

1. **Menu items not displaying:**
   - Verify items are added correctly
   - Check if view is properly added to view dispatcher
   - Ensure menu is allocated and initialized

2. **Icons not showing:**
   - Verify icon format (should be 10x10 pixels)
   - Check if icon reference is valid
   - Ensure icon is included in resources

3. **Navigation issues:**
   - Check if callback function is properly set
   - Verify indices are unique for each item
   - Ensure view dispatcher is handling inputs correctly

4. **Memory management:**
   - Always free the menu when done
   - Clean up view dispatcher
   - Release any custom resources

## Tips for Advanced Usage

1. **Implementing Sub-menus:**
```c
// In your callback function:
static void menu_callback(void* context, uint32_t index) {
    MyApp* app = context;
    if(index == SUBMENU_INDEX) {
        // Create and switch to a new menu view
        view_dispatcher_switch_to_view(app->view_dispatcher, SUBMENU_VIEW_ID);
    }
}
```

2. **Dynamic Menu Items:**
```c
// Clear existing items
menu_reset(menu);

// Add new items dynamically
for(int i = 0; i < item_count; i++) {
    menu_add_item(menu, items[i].name, items[i].icon, i, callback, context);
}
```

3. **Handling Back Navigation:**
```c
bool handle_back(void* context) {
    MyApp* app = context;
    // Return true if you want to handle back navigation
    // Return false to allow default handling
    return false;
}

// Set back callback
view_set_previous_callback(menu_get_view(menu), handle_back);
```
