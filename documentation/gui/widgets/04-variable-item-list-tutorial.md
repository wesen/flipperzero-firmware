# Tutorial: Using the Variable Item List Widget in Flipper Zero

## Overview

The Variable Item List widget provides a powerful way to create settings menus or option lists where each item can have different types of values (text, numbers, toggles). It's particularly useful for:

- Creating settings menus
- Building configuration screens
- Making option selectors
- Implementing preference panels
- Creating dynamic value lists

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Variable Item List Structure

```c
typedef struct {
    VariableItemList* variable_item_list;
    ViewDispatcher* view_dispatcher;
    // Add your application-specific state here
    bool toggle_value;
    uint8_t number_value;
    uint8_t option_index;
} MyApp;

// Callback for when an item's value changes
static void item_change_callback(void* context, uint32_t index) {
    MyApp* app = context;
    // Handle value change based on index
    switch(index) {
        case 0:
            // Handle first item change
            break;
        case 1:
            // Handle second item change
            break;
    }
}
```

## Step 3: Initialize the Variable Item List

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate the variable item list
app->variable_item_list = variable_item_list_alloc();

// Add a toggle item
VariableItem* toggle_item = variable_item_list_add(
    app->variable_item_list,
    "Toggle Option",        // Label
    2,                      // Number of options (for toggle: 2)
    item_change_callback,   // Callback
    app                     // Context
);

// Add a numeric item
const char* number_options[] = {"1", "2", "3", "4", "5"};
VariableItem* number_item = variable_item_list_add(
    app->variable_item_list,
    "Number",
    5,                      // Number of options
    item_change_callback,
    app
);

// Add a text option item
const char* text_options[] = {"Low", "Medium", "High"};
VariableItem* text_item = variable_item_list_add(
    app->variable_item_list,
    "Level",
    3,                      // Number of options
    item_change_callback,
    app
);

// Set initial values
variable_item_set_current_value_index(toggle_item, app->toggle_value ? 1 : 0);
variable_item_set_current_value_index(number_item, app->number_value);
variable_item_set_current_value_index(text_item, app->option_index);

// Set value text
variable_item_set_current_value_text(toggle_item, app->toggle_value ? "ON" : "OFF");
variable_item_set_current_value_text(number_item, number_options[app->number_value]);
variable_item_set_current_value_text(text_item, text_options[app->option_index]);
```

## Step 4: Integration with ViewDispatcher

```c
View* view = variable_item_list_get_view(app->variable_item_list);
view_dispatcher_add_view(view_dispatcher, MyVariableListView, view);
```

## Example Application

Here's a complete example that creates a settings menu:

```c
typedef struct {
    VariableItemList* variable_item_list;
    ViewDispatcher* view_dispatcher;
    bool vibro_enabled;
    bool sound_enabled;
    uint8_t brightness;
    uint8_t volume;
} SettingsApp;

static void vibro_change_callback(void* context) {
    SettingsApp* app = context;
    app->vibro_enabled = !app->vibro_enabled;
    
    VariableItem* item = variable_item_list_get_item(app->variable_item_list, 0);
    variable_item_set_current_value_text(item, app->vibro_enabled ? "ON" : "OFF");
}

static void sound_change_callback(void* context) {
    SettingsApp* app = context;
    app->sound_enabled = !app->sound_enabled;
    
    VariableItem* item = variable_item_list_get_item(app->variable_item_list, 1);
    variable_item_set_current_value_text(item, app->sound_enabled ? "ON" : "OFF");
}

static void brightness_change_callback(void* context, uint8_t index) {
    SettingsApp* app = context;
    app->brightness = index;
    
    const char* brightness_values[] = {"0%", "25%", "50%", "75%", "100%"};
    VariableItem* item = variable_item_list_get_item(app->variable_item_list, 2);
    variable_item_set_current_value_text(item, brightness_values[index]);
}

static void volume_change_callback(void* context, uint8_t index) {
    SettingsApp* app = context;
    app->volume = index;
    
    const char* volume_values[] = {"Mute", "Low", "Medium", "High"};
    VariableItem* item = variable_item_list_get_item(app->variable_item_list, 3);
    variable_item_set_current_value_text(item, volume_values[index]);
}

int32_t settings_app(void* p) {
    UNUSED(p);
    SettingsApp* app = malloc(sizeof(SettingsApp));

    // Initialize default values
    app->vibro_enabled = false;
    app->sound_enabled = true;
    app->brightness = 2;  // 50%
    app->volume = 2;      // Medium
    
    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate variable item list
    app->variable_item_list = variable_item_list_alloc();
    
    // Add vibration toggle
    VariableItem* vibro_item = variable_item_list_add(
        app->variable_item_list,
        "Vibration",
        2,
        vibro_change_callback,
        app
    );
    variable_item_set_current_value_text(vibro_item, app->vibro_enabled ? "ON" : "OFF");
    
    // Add sound toggle
    VariableItem* sound_item = variable_item_list_add(
        app->variable_item_list,
        "Sound",
        2,
        sound_change_callback,
        app
    );
    variable_item_set_current_value_text(sound_item, app->sound_enabled ? "ON" : "OFF");
    
    // Add brightness control
    const char* brightness_values[] = {"0%", "25%", "50%", "75%", "100%"};
    VariableItem* brightness_item = variable_item_list_add(
        app->variable_item_list,
        "Brightness",
        5,
        brightness_change_callback,
        app
    );
    variable_item_set_current_value_text(brightness_item, brightness_values[app->brightness]);
    
    // Add volume control
    const char* volume_values[] = {"Mute", "Low", "Medium", "High"};
    VariableItem* volume_item = variable_item_list_add(
        app->variable_item_list,
        "Volume",
        4,
        volume_change_callback,
        app
    );
    variable_item_set_current_value_text(volume_item, volume_values[app->volume]);
    
    // Get view and add to dispatcher
    View* view = variable_item_list_get_view(app->variable_item_list);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    variable_item_list_free(app->variable_item_list);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
    
    return 0;
}
```

## Key Features

- Support for multiple item types (toggle, numeric, text)
- Customizable labels and values
- Automatic navigation handling
- Individual callbacks per item
- Value text customization
- Dynamic value updates

## Common Customization Options

1. **Add New Item:**
```c
VariableItem* item = variable_item_list_add(
    list,
    "Label",
    num_options,
    callback,
    context
);
```

2. **Set Value Text:**
```c
variable_item_set_current_value_text(item, "Value");
```

3. **Set Selected Index:**
```c
variable_item_set_current_value_index(item, index);
```

4. **Get Item by Index:**
```c
VariableItem* item = variable_item_list_get_item(list, index);
```

## Best Practices

1. Keep labels clear and concise
2. Use consistent value formats
3. Handle all item callbacks appropriately
4. Update value text when value changes
5. Clean up resources when done
6. Consider screen space when adding items
7. Group related settings together

## Common Use Cases

1. **Toggle Setting:**
```c
VariableItem* toggle = variable_item_list_add(list, "Enable", 2, toggle_callback, context);
variable_item_set_current_value_text(toggle, enabled ? "ON" : "OFF");
```

2. **Numeric Setting:**
```c
VariableItem* number = variable_item_list_add(list, "Value", 10, number_callback, context);
char value_str[8];
snprintf(value_str, sizeof(value_str), "%d", current_value);
variable_item_set_current_value_text(number, value_str);
```

3. **Option Selection:**
```c
const char* options[] = {"Low", "Medium", "High"};
VariableItem* option = variable_item_list_add(list, "Level", 3, option_callback, context);
variable_item_set_current_value_text(option, options[current_index]);
```

## Common Issues and Solutions

1. **Values not updating:**
   - Verify callback is being called
   - Check if value text is being set
   - Ensure correct item index is used

2. **Navigation issues:**
   - Verify number of options is correct
   - Check if view is properly added to dispatcher
   - Ensure list is properly allocated

3. **Memory management:**
   - Free all items when done
   - Clean up variable item list
   - Release any custom resources

## Exercises

1. Create a settings menu with 5 different types of options
2. Implement a theme selector with different color schemes
3. Build a configuration panel with interdependent settings
4. Create a profile selector with different user preferences
5. Implement a difficulty settings menu for a game

## Advanced Usage

1. **Dynamic Options:**
```c
// Update options based on conditions
void update_options(VariableItem* item, bool advanced_mode) {
    if(advanced_mode) {
        variable_item_set_values_count(item, 5);  // More options in advanced mode
    } else {
        variable_item_set_values_count(item, 3);  // Fewer options in basic mode
    }
}
```

2. **Linked Settings:**
```c
// Example of settings that affect each other
void update_linked_settings(VariableItemList* list, uint8_t master_value) {
    VariableItem* dependent_item = variable_item_list_get_item(list, 1);
    // Update dependent item based on master value
    variable_item_set_values_count(dependent_item, master_value + 1);
}
```

3. **Custom Value Formatting:**
```c
// Format value with units
void set_value_with_units(VariableItem* item, int value, const char* units) {
    char text[32];
    snprintf(text, sizeof(text), "%d %s", value, units);
    variable_item_set_current_value_text(item, text);
}
```
