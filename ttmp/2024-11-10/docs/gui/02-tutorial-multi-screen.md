# Tutorial: Building a Multi-Screen Application with ViewDispatcher

This tutorial will guide you through creating a multi-screen application using ViewDispatcher. We'll build a "Device Info" app that shows various system information across multiple screens.

## Overview

Our application will have:
- A main menu screen
- A battery info screen
- A system info screen
- A settings screen

## Part 1: Application Structure

First, let's set up our application structure and necessary includes:

```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>

// View IDs - used to identify different screens
typedef enum {
    DeviceInfoViewSubmenu,
    DeviceInfoViewBattery,
    DeviceInfoViewSystem,
    DeviceInfoViewSettings,
} DeviceInfoView;

// Main application structure
typedef struct {
    // GUI
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    
    // Views
    Submenu* submenu;
    Widget* battery_view;
    Widget* system_view;
    VariableItemList* settings_view;
} DeviceInfo;
```

## Part 2: View Management Functions

Let's implement functions to handle view navigation:

```c
// Callback for back button events
static bool device_info_navigation_event_callback(void* context) {
    DeviceInfo* app = context;
    
    // Return to menu when back is pressed
    if(view_dispatcher_get_current_view(app->view_dispatcher) != DeviceInfoViewSubmenu) {
        view_dispatcher_switch_to_view(app->view_dispatcher, DeviceInfoViewSubmenu);
        return true;
    }
    
    return false; // Exit app if we're already in menu
}

// Callback for menu item selection
static void device_info_submenu_callback(void* context, uint32_t index) {
    DeviceInfo* app = context;
    
    // Switch to the selected view
    view_dispatcher_switch_to_view(app->view_dispatcher, index);
}
```

## Part 3: View Creation

Now let's create our different views:

```c
static void device_info_submenu_init(DeviceInfo* app) {
    Submenu* submenu = app->submenu;
    
    submenu_add_item(
        submenu, "Battery Info", DeviceInfoViewBattery, device_info_submenu_callback, app);
    submenu_add_item(
        submenu, "System Info", DeviceInfoViewSystem, device_info_submenu_callback, app);
    submenu_add_item(
        submenu, "Settings", DeviceInfoViewSettings, device_info_submenu_callback, app);
}

static void device_info_battery_view_init(DeviceInfo* app) {
    Widget* widget = app->battery_view;
    
    // Add battery information using widget API
    widget_add_string_element(
        widget, 64, 5, AlignCenter, AlignTop, FontPrimary, "Battery Status");
        
    char temp_str[64];
    float voltage = furi_hal_power_get_battery_voltage();
    uint32_t charge = furi_hal_power_get_battery_level();
    
    snprintf(temp_str, sizeof(temp_str), "Voltage: %.2fV\nCharge: %lu%%", (double)voltage, charge);
    widget_add_string_multiline_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, temp_str);
}

static void device_info_system_view_init(DeviceInfo* app) {
    Widget* widget = app->system_view;
    
    // Add system information using widget API
    widget_add_string_element(
        widget, 64, 5, AlignCenter, AlignTop, FontPrimary, "System Info");
        
    const char* device_name = furi_hal_version_get_name_ptr();
    const Version* firmware = furi_hal_version_get_firmware_version();
    
    char temp_str[64];
    snprintf(
        temp_str,
        sizeof(temp_str),
        "Device: %s\nFW: %s",
        device_name,
        version_get_version(firmware));
        
    widget_add_string_multiline_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, temp_str);
}

static void device_info_settings_init(DeviceInfo* app) {
    VariableItemList* list = app->settings_view;
    
    // Add some example settings
    variable_item_list_add(list, "Setting 1", 2, NULL, NULL);
    variable_item_list_add(list, "Setting 2", 3, NULL, NULL);
}
```

## Part 4: Application Lifecycle

Here's how we manage the application lifecycle:

```c
static DeviceInfo* device_info_app_alloc() {
    DeviceInfo* app = malloc(sizeof(DeviceInfo));
    
    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    
    // Initialize View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Set navigation callback
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, device_info_navigation_event_callback);
    
    // Allocate views
    app->submenu = submenu_alloc();
    app->battery_view = widget_alloc();
    app->system_view = widget_alloc();
    app->settings_view = variable_item_list_alloc();
    
    // Add views to the View Dispatcher
    view_dispatcher_add_view(
        app->view_dispatcher, DeviceInfoViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(
        app->view_dispatcher, DeviceInfoViewBattery, widget_get_view(app->battery_view));
    view_dispatcher_add_view(
        app->view_dispatcher, DeviceInfoViewSystem, widget_get_view(app->system_view));
    view_dispatcher_add_view(
        app->view_dispatcher,
        DeviceInfoViewSettings,
        variable_item_list_get_view(app->settings_view));
    
    // Initialize views
    device_info_submenu_init(app);
    device_info_battery_view_init(app);
    device_info_system_view_init(app);
    device_info_settings_init(app);
    
    return app;
}

static void device_info_app_free(DeviceInfo* app) {
    // Remove views from View Dispatcher
    view_dispatcher_remove_view(app->view_dispatcher, DeviceInfoViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, DeviceInfoViewBattery);
    view_dispatcher_remove_view(app->view_dispatcher, DeviceInfoViewSystem);
    view_dispatcher_remove_view(app->view_dispatcher, DeviceInfoViewSettings);
    
    // Free views
    submenu_free(app->submenu);
    widget_free(app->battery_view);
    widget_free(app->system_view);
    variable_item_list_free(app->settings_view);
    
    // Free View Dispatcher
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_GUI);
    
    free(app);
}
```

## Part 5: Main Entry Point

Finally, let's implement the main entry point:

```c
int32_t device_info_app(void* p) {
    UNUSED(p);
    
    DeviceInfo* app = device_info_app_alloc();
    
    // Start on the menu
    view_dispatcher_switch_to_view(app->view_dispatcher, DeviceInfoViewSubmenu);
    
    // Run the dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Free resources after dispatcher stops
    device_info_app_free(app);
    
    return 0;
}
```

## Part 6: Application Manifest

Create an `application.fam` file:

```python
App(
    appid="device_info",
    name="Device Info",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="device_info_app",
    requires=["gui"],
    stack_size=2 * 1024,
    fap_category="Tools",
)
```

## Key Concepts Explained

### ViewDispatcher

The ViewDispatcher manages multiple views and handles:
- View switching
- Input routing
- Navigation events

```c
// Basic ViewDispatcher usage:
ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
view_dispatcher_enable_queue(view_dispatcher);
view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
```

### Views

Each view represents a different screen. Common view types include:
- Submenu: List-style menu
- Widget: General-purpose view for custom content
- VariableItemList: Settings-style list with adjustable values

```c
// Adding a view:
view_dispatcher_add_view(view_dispatcher, view_id, view);

// Switching views:
view_dispatcher_switch_to_view(view_dispatcher, view_id);
```

### Navigation

Navigation is handled through:
- Back button events via navigation callback
- Menu selection events
- Custom events if needed

```c
// Setting navigation callback:
view_dispatcher_set_navigation_event_callback(
    view_dispatcher, navigation_callback);
```

## Enhancements You Could Add

1. **Real-time Updates**
   ```c
   // Add timer to update battery info periodically
   FuriTimer* timer = furi_timer_alloc(update_callback, FuriTimerTypePeriodic, app);
   furi_timer_start(timer, 1000);
   ```

2. **Persistent Settings**
   ```c
   // Save settings to storage
   Storage* storage = furi_record_open(RECORD_STORAGE);
   FlipperFormat* file = flipper_format_file_alloc(storage);
   ```

3. **Custom Events**
   ```c
   // Add custom event handling
   view_dispatcher_send_custom_event(view_dispatcher, custom_event);
   ```

4. **Animations**
   ```c
   // Add icon animations
   IconAnimation* animation = icon_animation_alloc(icon);
   icon_animation_start(animation);
   ```

## Best Practices

1. **Resource Management**
   - Always free allocated resources
   - Remove views before freeing ViewDispatcher
   - Close all records

2. **Error Handling**
   - Check allocation results
   - Validate user input
   - Handle navigation edge cases

3. **Code Organization**
   - Separate view initialization
   - Group related functionality
   - Use meaningful names

4. **Performance**
   - Minimize memory allocations
   - Update only when needed
   - Use appropriate view types

This tutorial demonstrated how to build a multi-screen application using ViewDispatcher. The example shows proper view management, navigation handling, and resource management while maintaining a clean and organized code structure.

Try extending the application with some of the suggested enhancements to better understand how different components work together in a more complex application.
