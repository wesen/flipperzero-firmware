=== BEGIN: applications/examples/example_ble_beacon/application.fam ===
App(
    appid="example_ble_beacon",
    name="Example: BLE Beacon",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="ble_beacon_app",
    requires=["gui"],
    stack_size=1 * 1024,
    fap_icon="example_ble_beacon_10px.png",
    fap_category="Examples",
    fap_icon_assets="images",
)

=== END: applications/examples/example_ble_beacon/application.fam ===

=== BEGIN: applications/examples/example_ble_beacon/ble_beacon_app.c ===
#include "ble_beacon_app.h"

#include <extra_beacon.h>
#include <furi_hal_version.h>

#include <string.h>

#define TAG "BleBeaconApp"

static bool ble_beacon_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BleBeaconApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool ble_beacon_app_back_event_callback(void* context) {
    furi_assert(context);
    BleBeaconApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void ble_beacon_app_tick_event_callback(void* context) {
    furi_assert(context);
    BleBeaconApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void ble_beacon_app_restore_beacon_state(BleBeaconApp* app) {
    // Restore beacon data from service
    GapExtraBeaconConfig* local_config = &app->beacon_config;
    const GapExtraBeaconConfig* config = furi_hal_bt_extra_beacon_get_config();
    if(config) {
        // We have a config, copy it
        memcpy(local_config, config, sizeof(app->beacon_config));
    } else {
        // No config, set up default values - they will stay until overriden or device is reset
        local_config->min_adv_interval_ms = 50;
        local_config->max_adv_interval_ms = 150;

        local_config->adv_channel_map = GapAdvChannelMapAll;
        local_config->adv_power_level = GapAdvPowerLevel_0dBm;

        local_config->address_type = GapAddressTypePublic;
        memcpy(
            local_config->address, furi_hal_version_get_ble_mac(), sizeof(local_config->address));
        // Modify MAC address to make it different from the one used by the main app
        local_config->address[0] ^= 0xFF;
        local_config->address[3] ^= 0xFF;

        furi_check(furi_hal_bt_extra_beacon_set_config(local_config));
    }

    // Get beacon state
    app->is_beacon_active = furi_hal_bt_extra_beacon_is_active();

    // Restore last beacon data
    app->beacon_data_len = furi_hal_bt_extra_beacon_get_data(app->beacon_data);
}

static BleBeaconApp* ble_beacon_app_alloc(void) {
    BleBeaconApp* app = malloc(sizeof(BleBeaconApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->scene_manager = scene_manager_alloc(&ble_beacon_app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    app->status_string = furi_string_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, ble_beacon_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, ble_beacon_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, ble_beacon_app_tick_event_callback, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewSubmenu, submenu_get_view(app->submenu));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewDialog, dialog_ex_get_view(app->dialog_ex));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BleBeaconAppViewByteInput, byte_input_get_view(app->byte_input));

    ble_beacon_app_restore_beacon_state(app);

    return app;
}

static void ble_beacon_app_free(BleBeaconApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewByteInput);
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, BleBeaconAppViewDialog);

    free(app->byte_input);
    free(app->submenu);
    free(app->dialog_ex);

    free(app->scene_manager);
    free(app->view_dispatcher);

    free(app->status_string);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

int32_t ble_beacon_app(void* args) {
    UNUSED(args);

    BleBeaconApp* app = ble_beacon_app_alloc();

    scene_manager_next_scene(app->scene_manager, BleBeaconAppSceneRunBeacon);

    view_dispatcher_run(app->view_dispatcher);

    ble_beacon_app_free(app);
    return 0;
}

void ble_beacon_app_update_state(BleBeaconApp* app) {
    furi_hal_bt_extra_beacon_stop();

    furi_check(furi_hal_bt_extra_beacon_set_config(&app->beacon_config));

    app->beacon_data_len = 0;
    while((app->beacon_data[app->beacon_data_len] != 0) &&
          (app->beacon_data_len < sizeof(app->beacon_data))) {
        app->beacon_data_len++;
    }

    FURI_LOG_I(TAG, "beacon_data_len: %d", app->beacon_data_len);

    furi_check(furi_hal_bt_extra_beacon_set_data(app->beacon_data, app->beacon_data_len));

    if(app->is_beacon_active) {
        furi_check(furi_hal_bt_extra_beacon_start());
    }
}

=== END: applications/examples/example_ble_beacon/ble_beacon_app.c ===

=== BEGIN: applications/examples/example_ble_beacon/ble_beacon_app.h ===
/**
 * @file ble_beacon_app.h
 * @brief BLE beacon example.
 */
#pragma once

#include "extra_beacon.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/dialog_ex.h>

#include <rpc/rpc_app.h>
#include <notification/notification_messages.h>

#include <furi_hal_bt.h>

#include "scenes/scenes.h"
#include <stdint.h>

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    ByteInput* byte_input;
    DialogEx* dialog_ex;

    FuriString* status_string;

    GapExtraBeaconConfig beacon_config;
    uint8_t beacon_data[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t beacon_data_len;
    bool is_beacon_active;
} BleBeaconApp;

typedef enum {
    BleBeaconAppViewSubmenu,
    BleBeaconAppViewByteInput,
    BleBeaconAppViewDialog,
} BleBeaconAppView;

typedef enum {
    BleBeaconAppCustomEventDataEditResult = 100,
} BleBeaconAppCustomEvent;

void ble_beacon_app_update_state(BleBeaconApp* app);

=== END: applications/examples/example_ble_beacon/ble_beacon_app.h ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_config.h ===
ADD_SCENE(ble_beacon_app, menu, Menu)
ADD_SCENE(ble_beacon_app, input_mac_addr, InputMacAddress)
ADD_SCENE(ble_beacon_app, input_beacon_data, InputBeaconData)
ADD_SCENE(ble_beacon_app, run_beacon, RunBeacon)

=== END: applications/examples/example_ble_beacon/scenes/scene_config.h ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_input_beacon_data.c ===
#include "../ble_beacon_app.h"

static void ble_beacon_app_scene_add_type_byte_input_callback(void* context) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(
        ble_beacon->view_dispatcher, BleBeaconAppCustomEventDataEditResult);
}

void ble_beacon_app_scene_input_beacon_data_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    byte_input_set_header_text(ble_beacon->byte_input, "Enter beacon data");

    byte_input_set_result_callback(
        ble_beacon->byte_input,
        ble_beacon_app_scene_add_type_byte_input_callback,
        NULL,
        context,
        ble_beacon->beacon_data,
        sizeof(ble_beacon->beacon_data));

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewByteInput);
}

bool ble_beacon_app_scene_input_beacon_data_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BleBeaconAppCustomEventDataEditResult) {
            ble_beacon_app_update_state(ble_beacon);
            scene_manager_previous_scene(scene_manager);
            return true;
        }
    }

    return false;
}

void ble_beacon_app_scene_input_beacon_data_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;

    byte_input_set_result_callback(ble_beacon->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ble_beacon->byte_input, NULL);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_input_beacon_data.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_input_mac_addr.c ===
#include "../ble_beacon_app.h"

static void ble_beacon_app_scene_add_type_byte_input_callback(void* context) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(
        ble_beacon->view_dispatcher, BleBeaconAppCustomEventDataEditResult);
}

void ble_beacon_app_scene_input_mac_addr_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    byte_input_set_header_text(ble_beacon->byte_input, "Enter MAC (reversed)");

    byte_input_set_result_callback(
        ble_beacon->byte_input,
        ble_beacon_app_scene_add_type_byte_input_callback,
        NULL,
        context,
        ble_beacon->beacon_config.address,
        sizeof(ble_beacon->beacon_config.address));

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewByteInput);
}

bool ble_beacon_app_scene_input_mac_addr_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BleBeaconAppCustomEventDataEditResult) {
            ble_beacon_app_update_state(ble_beacon);
            scene_manager_previous_scene(scene_manager);
            return true;
        }
    }

    return false;
}

void ble_beacon_app_scene_input_mac_addr_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;

    byte_input_set_result_callback(ble_beacon->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(ble_beacon->byte_input, NULL);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_input_mac_addr.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_menu.c ===
#include "../ble_beacon_app.h"

enum SubmenuIndex {
    SubmenuIndexSetMac,
    SubmenuIndexSetData,
};

static void ble_beacon_app_scene_menu_submenu_callback(void* context, uint32_t index) {
    BleBeaconApp* ble_beacon = context;
    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, index);
}

void ble_beacon_app_scene_menu_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;
    Submenu* submenu = ble_beacon->submenu;

    submenu_add_item(
        submenu,
        "Set MAC",
        SubmenuIndexSetMac,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);
    submenu_add_item(
        submenu,
        "Set Data",
        SubmenuIndexSetData,
        ble_beacon_app_scene_menu_submenu_callback,
        ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewSubmenu);
}

bool ble_beacon_app_scene_menu_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexSetMac) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputMacAddress);
            consumed = true;
        } else if(submenu_index == SubmenuIndexSetData) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneInputBeaconData);
            consumed = true;
        }
    }

    return consumed;
}

void ble_beacon_app_scene_menu_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    submenu_reset(ble_beacon->submenu);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_menu.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scene_run_beacon.c ===
#include "../ble_beacon_app.h"
#include <example_ble_beacon_icons.h>

static void
    ble_beacon_app_scene_run_beacon_confirm_dialog_callback(DialogExResult result, void* context) {
    BleBeaconApp* ble_beacon = context;

    view_dispatcher_send_custom_event(ble_beacon->view_dispatcher, result);
}

static void update_status_text(BleBeaconApp* ble_beacon) {
    DialogEx* dialog_ex = ble_beacon->dialog_ex;

    dialog_ex_set_header(dialog_ex, "BLE Beacon Demo", 64, 0, AlignCenter, AlignTop);

    FuriString* status = ble_beacon->status_string;

    furi_string_reset(status);

    furi_string_cat_str(status, "Status: ");
    if(ble_beacon->is_beacon_active) {
        furi_string_cat_str(status, "Running\n");
    } else {
        furi_string_cat_str(status, "Stopped\n");
    }

    // Output MAC in reverse order
    for(int i = sizeof(ble_beacon->beacon_config.address) - 1; i >= 0; i--) {
        furi_string_cat_printf(status, "%02X", ble_beacon->beacon_config.address[i]);
        if(i > 0) {
            furi_string_cat_str(status, ":");
        }
    }

    furi_string_cat_printf(status, "\nData length: %d", ble_beacon->beacon_data_len);

    dialog_ex_set_text(dialog_ex, furi_string_get_cstr(status), 0, 29, AlignLeft, AlignCenter);

    dialog_ex_set_icon(dialog_ex, 93, 20, &I_lighthouse_35x44);

    dialog_ex_set_left_button_text(dialog_ex, "Config");

    dialog_ex_set_center_button_text(dialog_ex, ble_beacon->is_beacon_active ? "Stop" : "Start");

    dialog_ex_set_result_callback(
        dialog_ex, ble_beacon_app_scene_run_beacon_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, ble_beacon);
}

void ble_beacon_app_scene_run_beacon_on_enter(void* context) {
    BleBeaconApp* ble_beacon = context;

    update_status_text(ble_beacon);

    view_dispatcher_switch_to_view(ble_beacon->view_dispatcher, BleBeaconAppViewDialog);
}

bool ble_beacon_app_scene_run_beacon_on_event(void* context, SceneManagerEvent event) {
    BleBeaconApp* ble_beacon = context;
    SceneManager* scene_manager = ble_beacon->scene_manager;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_next_scene(scene_manager, BleBeaconAppSceneMenu);
            return true;
        } else if(event.event == DialogExResultCenter) {
            ble_beacon->is_beacon_active = !ble_beacon->is_beacon_active;
            ble_beacon_app_update_state(ble_beacon);
            update_status_text(ble_beacon);
            return true;
        }
    }
    return false;
}

void ble_beacon_app_scene_run_beacon_on_exit(void* context) {
    BleBeaconApp* ble_beacon = context;
    UNUSED(ble_beacon);
}

=== END: applications/examples/example_ble_beacon/scenes/scene_run_beacon.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scenes.c ===
#include "scenes.h"

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const ble_beacon_app_on_enter_handlers[])(void*) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const ble_beacon_app_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const ble_beacon_app_on_exit_handlers[])(void* context) = {
#include "scene_config.h"
};
#undef ADD_SCENE

// Initialize scene handlers configuration structure
const SceneManagerHandlers ble_beacon_app_scene_handlers = {
    .on_enter_handlers = ble_beacon_app_on_enter_handlers,
    .on_event_handlers = ble_beacon_app_on_event_handlers,
    .on_exit_handlers = ble_beacon_app_on_exit_handlers,
    .scene_num = BleBeaconAppSceneNum,
};

=== END: applications/examples/example_ble_beacon/scenes/scenes.c ===

=== BEGIN: applications/examples/example_ble_beacon/scenes/scenes.h ===
#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) BleBeaconAppScene##id,
typedef enum {
#include "scene_config.h"
    BleBeaconAppSceneNum,
} BleBeaconAppScene;
#undef ADD_SCENE

extern const SceneManagerHandlers ble_beacon_app_scene_handlers;

// Generate scene on_enter handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_event handlers declaration
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "scene_config.h"
#undef ADD_SCENE

// Generate scene on_exit handlers declaration
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "scene_config.h"
#undef ADD_SCENE

=== END: applications/examples/example_ble_beacon/scenes/scenes.h ===


=== BEGIN: ttmp/2024-11-10/docs/gui/03-gui.md ===
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

=== END: ttmp/2024-11-10/docs/gui/03-gui.md ===

=== BEGIN: ttmp/2024-11-10/docs/gui/04-view-dispatcher.md ===
Here's a detailed documentation of how the ViewDispatcher works in the Flipper Zero GUI service:

# ViewDispatcher Documentation

## Overview

The ViewDispatcher is a key component in the Flipper Zero GUI system that manages multiple Views and handles switching between them. It acts as a coordinator that:

1. Maintains a registry of Views identified by unique IDs
2. Manages the current active View
3. Handles input events and routes them to the active View
4. Processes custom events and navigation (back button) events
5. Manages view transitions and lifecycle events

## Core Concepts

### Views and View IDs

- Views must be registered with the ViewDispatcher using unique numeric IDs
- Each View represents a distinct screen/interface in the application
- Multiple Views can be registered but only one is active at a time
- Views cannot be simultaneously registered with multiple ViewDispatchers

```c
typedef enum {
    MyFirstViewId,
    MySecondViewId,
    // ... other view IDs
} ViewId;

// Register a view
view_dispatcher_add_view(view_dispatcher, MyFirstViewId, my_view);
```

### Event Handling

The ViewDispatcher processes three main types of events:

1. Input Events

- Handles d-pad and button inputs
- Routes events to the current View's input callback
- Manages input state tracking for press/release pairs

2. Custom Events

- Application-defined events sent via `view_dispatcher_send_custom_event()`
- First passed to current View's custom callback
- If unhandled, passed to ViewDispatcher's custom event callback

3. Navigation Events

- Triggered by Back button if not handled by current View
- Can be used to implement view navigation/transitions
- Navigation callback determines if application should exit

## Initialization and Setup

### Creating a ViewDispatcher

```c
// Allocate
ViewDispatcher* view_dispatcher = view_dispatcher_alloc();

// Enable event queue (required for handling events)
view_dispatcher_enable_queue(view_dispatcher);

// Set event callbacks
view_dispatcher_set_event_callback_context(view_dispatcher, context);
view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);
view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);

// Attach to GUI
Gui* gui = furi_record_open(RECORD_GUI);
view_dispatcher_attach_to_gui(
    view_dispatcher,
    gui,
    ViewDispatcherTypeFullscreen  // or Window/Desktop
);
```

### Adding Views

```c
// Create and configure view
View* view = view_alloc();
view_set_draw_callback(view, draw_callback);
view_set_input_callback(view, input_callback);

// Add to dispatcher
view_dispatcher_add_view(view_dispatcher, VIEW_ID, view);

// Switch to view
view_dispatcher_switch_to_view(view_dispatcher, VIEW_ID);
```

## Event Flow

### Input Event Flow

1. GUI system captures input
2. ViewDispatcher receives input via registered ViewPort
3. Input queued to ViewDispatcher's input queue
4. ViewDispatcher processes queued input:
   - Tracks ongoing input state
   - Routes to current View's input callback
   - If unhandled and Back button, triggers navigation

### Custom Event Flow

1. Application sends custom event:

```c
view_dispatcher_send_custom_event(view_dispatcher, MY_EVENT);
```

2. Event queued to ViewDispatcher's event queue
3. ViewDispatcher processes event:
   - First sent to current View's custom callback
   - If unhandled, sent to ViewDispatcher's custom callback

### Navigation Event Flow

1. Back button pressed
2. Current View's input callback can handle it
3. If unhandled:
   - View's navigation callback called
   - Can return new view ID to switch to
4. If still unhandled:
   - ViewDispatcher's navigation callback called
   - Can return false to exit application

## View Lifecycle

### View Transitions

When switching views:

1. Current view's exit callback called
2. View orientation updated if needed
3. New view's enter callback called
4. ViewPort enabled/updated to trigger redraw

```c
void view_dispatcher_set_current_view(ViewDispatcher* dispatcher, View* view) {
    // Exit current view
    if(dispatcher->current_view) {
        view_exit(dispatcher->current_view);
    }

    // Set new view
    dispatcher->current_view = view;

    // Enter new view
    if(view) {
        view_enter(view);
        view_port_enabled_set(dispatcher->view_port, true);
        view_port_update(dispatcher->view_port);
    }
}
```

### Cleanup

When shutting down:

1. Remove all views
2. Free ViewDispatcher
3. Close GUI

```c
// Remove views
view_dispatcher_remove_view(view_dispatcher, VIEW_ID);
view_free(view);

// Cleanup dispatcher
view_dispatcher_free(view_dispatcher);
furi_record_close(RECORD_GUI);
```

## Best Practices

1. **Event Handling**

- Always handle or explicitly ignore input events
- Use custom events for View-to-View communication
- Keep navigation logic in navigation callbacks

2. **View Management**

- Free Views after removing from ViewDispatcher
- Don't share Views between ViewDispatchers
- Clean up resources in View exit callbacks

3. **Threading**

- ViewDispatcher callbacks run in GUI thread
- Don't block in callbacks
- Use message queues for thread communication

4. **Error Handling**

- Check return values from ViewDispatcher operations
- Maintain valid View state during transitions
- Clean up resources on error paths

## Example Use Case

Here's a basic example showing common ViewDispatcher usage:

```c
typedef struct {
    ViewDispatcher* view_dispatcher;
    View* main_view;
    View* settings_view;
} MyApp;

bool my_navigation_callback(void* context) {
    MyApp* app = context;
    return false; // Exit app on back
}

void my_custom_callback(void* context, uint32_t event) {
    MyApp* app = context;
    switch(event) {
        case ShowSettings:
            view_dispatcher_switch_to_view(
                app->view_dispatcher,
                SettingsViewId
            );
            break;
    }
}

MyApp* app_alloc() {
    MyApp* app = malloc(sizeof(MyApp));

    // Create dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(
        app->view_dispatcher,
        app
    );
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher,
        my_navigation_callback
    );
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher,
        my_custom_callback
    );

    // Add views
    app->main_view = view_alloc();
    app->settings_view = view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        MainViewId,
        app->main_view
    );
    view_dispatcher_add_view(
        app->view_dispatcher,
        SettingsViewId,
        app->settings_view
    );

    return app;
}
```

This documentation should help developers understand how the ViewDispatcher works and how to effectively use it in Flipper Zero applications.

=== END: ttmp/2024-11-10/docs/gui/04-view-dispatcher.md ===

=== BEGIN: ttmp/2024-11-10/docs/gui/05-viewdispatcher-course.md ===
# Flipper Zero ViewDispatcher Course

## Course Overview
This course teaches developers how to effectively use the ViewDispatcher system in Flipper Zero applications. Through progressive lessons and hands-on exercises, you'll learn to manage multiple views, handle user input, and create polished user interfaces.

## Section 1: Fundamentals of ViewDispatcher

### Understanding ViewDispatcher Basics

The ViewDispatcher is a core component that manages multiple views in a Flipper Zero application. Think of it as a traffic controller for your app's screens - it determines which view is currently visible and routes user input to the appropriate handlers.

At its most basic, the ViewDispatcher requires three key components:
1. The ViewDispatcher instance itself
2. One or more View objects 
3. A connection to the GUI system

Here's how these pieces come together:

```c
// Create the dispatcher
ViewDispatcher* view_dispatcher = view_dispatcher_alloc();

// Create a view
View* my_view = view_alloc();

// Get GUI handle and connect
Gui* gui = furi_record_open(RECORD_GUI);
view_dispatcher_attach_to_gui(
    view_dispatcher, 
    gui, 
    ViewDispatcherTypeFullscreen
);
```

The ViewDispatcher uses a simple identifier system to track views. Each view is registered with a numeric ID that you define:

```c
// Define view IDs
enum {
    MyMainViewId,
    MySettingsViewId
};

// Register views
view_dispatcher_add_view(view_dispatcher, MyMainViewId, my_view);
view_dispatcher_add_view(view_dispatcher, MySettingsViewId, settings_view);
```

To switch between views, you use these IDs:

```c
// Switch to main view
view_dispatcher_switch_to_view(view_dispatcher, MyMainViewId);
```

The ViewDispatcher must be explicitly started to begin processing events:

```c
view_dispatcher_run(view_dispatcher);
```

### Event Queue System

A key concept is that the ViewDispatcher uses an event queue to handle input and custom events. This must be enabled:

```c
view_dispatcher_enable_queue(view_dispatcher);
```

Once enabled, you can send custom events to trigger view changes or other actions:

```c
view_dispatcher_send_custom_event(view_dispatcher, MyCustomEventId);
```

### Exercises for Section 1

#### Exercise 1.1: Basic Setup
Create a minimal application that:
- Allocates a ViewDispatcher
- Creates one View
- Registers the View with ID=0
- Connects to GUI
- Runs the dispatcher
- Properly cleans up resources

#### Exercise 1.2: Multiple Views
Extend Exercise 1.1 to:
- Create two Views
- Register them with different IDs
- Switch between them after a 2-second delay
- Properly clean up both views

#### Exercise 1.3: Custom Events
Create an application that:
- Has two views
- Sends a custom event every second
- Switches between views when receiving the custom event
- Uses proper event queue initialization

#### Exercise 1.4: View Cleanup
Create an application demonstrating proper cleanup:
- Register 3 views
- Switch between them
- Implement a proper shutdown sequence
- Verify no memory leaks using the debug console

## Section 2: View Management and Callbacks

### Understanding View Lifecycle

Views in the ViewDispatcher system have a defined lifecycle with specific callbacks:

```c
// Draw callback - called when view needs to render
static void view_draw_callback(Canvas* canvas, void* context) {
    canvas_draw_str(canvas, 0, 10, "Hello World");
}

// Input callback - called for button presses
static bool view_input_callback(InputEvent* event, void* context) {
    if(event->type == InputTypeShort) {
        return true; // Event handled
    }
    return false; // Event not handled
}

// Setup callbacks
view_set_draw_callback(view, view_draw_callback);
view_set_input_callback(view, view_input_callback);
```

Enter and exit callbacks are triggered when switching views:

```c
static void view_enter_callback(void* context) {
    // Setup when view becomes active
}

static void view_exit_callback(void* context) {
    // Cleanup when view becomes inactive
}

view_set_enter_callback(view, view_enter_callback);
view_set_exit_callback(view, view_exit_callback);
```

### Exercises for Section 2

#### Exercise 2.1: View Callbacks
Create an application with:
- One view with all callbacks implemented
- Draw callback that shows how long view has been active
- Input callback that handles all button types
- Enter/exit callbacks that log to debug console

#### Exercise 2.2: Context Usage
Create an application that:
- Defines a custom context struct with counter
- Passes context through to all callbacks
- Updates counter in input callback
- Displays counter in draw callback

#### Exercise 2.3: View Switching
Create an application with:
- Two views sharing same context
- Different drawing styles for each view
- Proper state handling during switches
- Enter/exit callbacks that maintain view state

#### Exercise 2.4: Input Handling
Create an application demonstrating:
- Complex input handling (combinations)
- Input state tracking between views
- Custom event generation from input
- Proper input event consumption

## Section 3: Advanced ViewDispatcher Features

### Navigation and Custom Events

The ViewDispatcher can handle navigation (back button) and custom events with special callbacks:

```c
bool navigation_callback(void* context) {
    // Return true to handle back button
    // Return false to exit application
    return true;
}

bool custom_callback(void* context, uint32_t event) {
    // Handle custom events
    switch(event) {
        case MyCustomEvent:
            // Do something
            return true;
    }
    return false;
}

view_dispatcher_set_navigation_event_callback(
    view_dispatcher,
    navigation_callback
);

view_dispatcher_set_custom_event_callback(
    view_dispatcher,
    custom_callback
);
```

### Exercises for Section 3

#### Exercise 3.1: Navigation System
Create an application with:
- Three views in a hierarchical structure
- Navigation callback implementing back behavior
- Custom transition animations
- State preservation between views

#### Exercise 3.2: Custom Event System
Create an application demonstrating:
- Multiple custom event types
- Event handling at view and dispatcher level
- Event queueing and processing
- Error handling for events

#### Exercise 3.3: Complex State Management
Create an application that:
- Maintains complex state across views
- Handles state corruption scenarios
- Implements save/restore functionality
- Uses custom events for state updates

#### Exercise 3.4: Integration Project
Create a complete application that:
- Uses all ViewDispatcher features
- Implements a coherent UI flow
- Handles all error cases
- Follows best practices for resource management

## Final Project

### ViewDispatcher Weather Station

Create a weather station application that demonstrates mastery of the ViewDispatcher system:

Requirements:
1. Main view showing current readings
2. Settings view for configuration
3. History view showing graphs
4. About view with information
5. Proper navigation between all views
6. Custom events for sensor updates
7. State management for settings
8. Clean shutdown and resource handling

This project will test your understanding of:
- View management
- Event handling
- State management
- Resource cleanup
- User interface design
- Error handling
- Performance optimization

Submit your project with:
- Full source code
- Documentation
- Test cases
- Performance analysis

This course structure provides a progressive learning path from basic concepts to advanced features, with practical exercises reinforcing each concept. Each section builds on the previous knowledge while introducing new concepts and challenges.
=== END: ttmp/2024-11-10/docs/gui/05-viewdispatcher-course.md ===

=== BEGIN: ttmp/2024-11-10/docs/gui/14-viewdispatcher-documentation.md ===
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

=== END: ttmp/2024-11-10/docs/gui/14-viewdispatcher-documentation.md ===

=== BEGIN: ttmp/2024-11-10/docs/gui/15-viewdispatcher-tutorial.md ===
# Tutorial: Building a ViewDispatcher Demo Application for Flipper Zero

## Overview

In this tutorial, we'll build a complete ViewDispatcher demo application for the Flipper Zero that demonstrates key concepts including:

- Setting up a ViewDispatcher
- Creating and managing multiple Views
- Handling input events
- Switching between Views
- Coordinating UI updates

We'll create an interactive application that displays "Hello World" and allows moving a cursor using the d-pad, with the ability to switch screen orientation.

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Project Setup

First, let's create our application's entry point and required structures.

```c
typedef enum {
    MyViewId,          // First view ID
    MyOtherViewId,     // Second view ID
} ViewId;

// Global state variables
int x = 32;           // Cursor X position
int y = 48;           // Cursor Y position
ViewId current_view;  // Tracks current active view
```

**Exercise 1:** Try adding another view ID to the enum. Think about what kind of view it could represent.

## Step 2: Creating the Draw Callback

Let's implement the function that draws our UI:

```c
static void my_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 30, "Hello world");
    canvas_draw_str_aligned(canvas, x, y, AlignLeft, AlignTop, "^");
}
```

This callback:

1. Sets the primary font
2. Draws "Hello world" at position (5,30)
3. Draws a cursor "^" at the current (x,y) position

**Exercise 2:** Modify the draw callback to:

1. Add your name below "Hello world"
2. Change the cursor character to something else
3. Try different font alignments

## Step 3: Input Handling

Now we'll implement input handling to move the cursor:

```c
static bool my_input_callback(InputEvent* input_event, void* context) {
    furi_assert(context);
    bool handled = false;
    ViewDispatcher* view_dispatcher = context;

    if(input_event->type == InputTypeShort) {
        switch(input_event->key) {
            case InputKeyLeft:
                x--;
                handled = true;
                break;
            case InputKeyRight:
                x++;
                handled = true;
                break;
            case InputKeyUp:
                y--;
                handled = true;
                break;
            case InputKeyDown:
                y++;
                handled = true;
                break;
            case InputKeyOk:
                view_dispatcher_send_custom_event(view_dispatcher, 42);
                handled = true;
                break;
        }
    }

    return handled;
}
```

Key concepts:

- Returns `true` if the input was handled
- Modifies global x/y coordinates based on d-pad input
- Sends a custom event when OK is pressed

**Exercise 3:**

1. Add diagonal movement when two direction keys are pressed
2. Add boundary checking to prevent cursor from going off screen
3. Implement different cursor movement speeds

## Step 4: Custom Event Handling

Let's implement the handler for custom events:

```c
bool my_view_dispatcher_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ViewDispatcher* view_dispatcher = context;

    if(event == 42) {
        // Toggle between views
        current_view = (current_view == MyViewId) ? MyOtherViewId : MyViewId;
        view_dispatcher_switch_to_view(view_dispatcher, current_view);
        return true;
    }

    return false;
}
```

This callback:

1. Receives custom events (our magic number 42)
2. Toggles between our two views
3. Triggers the view switch

**Exercise 4:**

1. Add another custom event type
2. Implement different behaviors for different event values
3. Add a counter for view switches

## Step 5: Main Application Setup

Now let's put it all together:

```c
int32_t viewdispatcher_demo_app() {
    // 1. Allocate ViewDispatcher
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();
    void* my_context = view_dispatcher;

    // 2. Create Views with different orientations
    View* view1 = view_alloc();
    view_set_context(view1, my_context);
    view_set_draw_callback(view1, my_draw_callback);
    view_set_input_callback(view1, my_input_callback);
    view_set_orientation(view1, ViewOrientationHorizontal);

    View* view2 = view_alloc();
    view_set_context(view2, my_context);
    view_set_draw_callback(view2, my_draw_callback);
    view_set_input_callback(view2, my_input_callback);
    view_set_orientation(view2, ViewOrientationVertical);

    // 3. Configure ViewDispatcher
    view_dispatcher_set_event_callback_context(view_dispatcher, my_context);
    view_dispatcher_set_custom_event_callback(
        view_dispatcher, my_view_dispatcher_custom_event_callback);
    view_dispatcher_enable_queue(view_dispatcher);

    // 4. Add views to ViewDispatcher
    view_dispatcher_add_view(view_dispatcher, MyViewId, view1);
    view_dispatcher_add_view(view_dispatcher, MyOtherViewId, view2);

    // 5. Setup GUI and run
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    current_view = MyViewId;
    view_dispatcher_switch_to_view(view_dispatcher, current_view);
    view_dispatcher_run(view_dispatcher);

    // 6. Cleanup
    view_dispatcher_remove_view(view_dispatcher, MyViewId);
    view_dispatcher_remove_view(view_dispatcher, MyOtherViewId);
    view_free(view1);
    view_free(view2);
    view_dispatcher_free(view_dispatcher);
    furi_record_close(RECORD_GUI);

    return 0;
}
```

Let's break down the main setup:

1. Allocate ViewDispatcher and set context
2. Create two Views with different orientations but same callbacks
3. Configure ViewDispatcher event handling
4. Register Views with ViewDispatcher
5. Setup GUI integration and run main loop
6. Clean up resources properly

**Exercise 5:**

1. Add a third View with different orientation
2. Modify the cleanup code to handle the new View
3. Implement a view transition animation

## Step 6: Application Integration

Finally, create the application manifest (application.fam):

```python
App(
    appid="viewdispatcher_demo",
    name="ViewDispatcher demo",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="viewdispatcher_demo_app",
    requires=["gui"],
    stack_size=2 * 1024,
    fap_category="UI",
)
```

This defines:

- Application identifier and name
- Entry point function
- Required system modules
- Resource requirements

## Advanced Exercises

1. **State Management**

   - Add persistent state that survives view switches
   - Implement save/load functionality for cursor position
   - Add configuration options

2. **Enhanced Visualization**

   - Add multiple cursors with different colors
   - Implement cursor trails
   - Add animated elements

3. **Input Enhancement**

   - Add long-press handling
   - Implement gesture recognition
   - Add multi-key combinations

4. **View Transitions**
   - Add fade transitions between views
   - Implement slide animations
   - Add transition effects

## Common Pitfalls

1. **Memory Management**

   - Always free Views after removing them from ViewDispatcher
   - Clean up resources in correct order
   - Check for memory leaks

2. **Event Handling**

   - Ensure all input events are properly handled
   - Don't block in event callbacks
   - Handle edge cases in view transitions

3. **State Management**
   - Maintain consistent state across view switches
   - Handle orientation changes properly
   - Clean up state when application exits

## Conclusion

This tutorial demonstrated building a basic ViewDispatcher application. Key takeaways:

- Proper ViewDispatcher setup and teardown
- View management and switching
- Input and custom event handling
- Resource management

Continue experimenting with:

- Different view configurations
- Custom event patterns
- Advanced input handling
- UI animations and transitions

Remember to always:

- Test thoroughly
- Handle errors gracefully
- Clean up resources properly
- Follow Flipper Zero UI guidelines

=== END: ttmp/2024-11-10/docs/gui/15-viewdispatcher-tutorial.md ===


=== BEGIN: applications/services/gui/canvas.h ===
/**
 * @file canvas.h
 * GUI: Canvas API
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <gui/icon_animation.h>
#include <gui/icon.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Color enumeration */
typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
    ColorXOR = 0x02,
} Color;

/** Fonts enumeration */
typedef enum {
    FontPrimary,
    FontSecondary,
    FontKeyboard,
    FontBigNumbers,

    // Keep last for fonts number calculation
    FontTotalNumber,
} Font;

/** Alignment enumeration */
typedef enum {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenter,
} Align;

/** Canvas Orientation */
typedef enum {
    CanvasOrientationHorizontal,
    CanvasOrientationHorizontalFlip,
    CanvasOrientationVertical,
    CanvasOrientationVerticalFlip,
} CanvasOrientation;

/** Font Direction */
typedef enum {
    CanvasDirectionLeftToRight,
    CanvasDirectionTopToBottom,
    CanvasDirectionRightToLeft,
    CanvasDirectionBottomToTop,
} CanvasDirection;

/** Font parameters */
typedef struct {
    uint8_t leading_default;
    uint8_t leading_min;
    uint8_t height;
    uint8_t descender;
} CanvasFontParameters;

/** Icon flip */
typedef enum {
    IconFlipNone,
    IconFlipHorizontal,
    IconFlipVertical,
    IconFlipBoth,
} IconFlip;

/** Icon rotation */
typedef enum {
    IconRotation0,
    IconRotation90,
    IconRotation180,
    IconRotation270,
} IconRotation;

/** Canvas anonymous structure */
typedef struct Canvas Canvas;

/** Reset canvas drawing tools configuration
 *
 * @param      canvas  Canvas instance
 */
void canvas_reset(Canvas* canvas);

/** Commit canvas. Send buffer to display
 *
 * @param      canvas  Canvas instance
 */
void canvas_commit(Canvas* canvas);

/** Get Canvas width
 *
 * @param      canvas  Canvas instance
 *
 * @return     width in pixels.
 */
size_t canvas_width(const Canvas* canvas);

/** Get Canvas height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
size_t canvas_height(const Canvas* canvas);

/** Get current font height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
size_t canvas_current_font_height(const Canvas* canvas);

/** Get font parameters
 *
 * @param      canvas  Canvas instance
 * @param      font    Font
 *
 * @return     pointer to CanvasFontParameters structure
 */
const CanvasFontParameters* canvas_get_font_params(const Canvas* canvas, Font font);

/** Clear canvas
 *
 * @param      canvas  Canvas instance
 */
void canvas_clear(Canvas* canvas);

/** Set drawing color
 *
 * @param      canvas  Canvas instance
 * @param      color   Color
 */
void canvas_set_color(Canvas* canvas, Color color);

/** Set font swap Argument String Rotation Description
 *
 * @param      canvas  Canvas instance
 * @param      dir     Direction font
 */
void canvas_set_font_direction(Canvas* canvas, CanvasDirection dir);

/** Invert drawing color
 *
 * @param      canvas  Canvas instance
 */
void canvas_invert_color(Canvas* canvas);

/** Set drawing font
 *
 * @param      canvas  Canvas instance
 * @param      font    Font
 */
void canvas_set_font(Canvas* canvas, Font font);

/** Set custom drawing font
 *
 * @param      canvas  Canvas instance
 * @param      font    Pointer to u8g2 const uint8_t* font array
 */
void canvas_set_custom_u8g2_font(Canvas* canvas, const uint8_t* font);

/** Draw string at position of baseline defined by x, y.
 *
 * @param      canvas  Canvas instance
 * @param      x       anchor point x coordinate
 * @param      y       anchor point y coordinate
 * @param      str     C-string
 */
void canvas_draw_str(Canvas* canvas, int32_t x, int32_t y, const char* str);

/** Draw aligned string defined by x, y.
 *
 * Align calculated from position of baseline, string width and ascent (height
 * of the glyphs above the baseline)
 *
 * @param      canvas      Canvas instance
 * @param      x           anchor point x coordinate
 * @param      y           anchor point y coordinate
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 * @param      str         C-string
 */
void canvas_draw_str_aligned(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    Align horizontal,
    Align vertical,
    const char* str);

/** Get string width
 *
 * @param      canvas  Canvas instance
 * @param      str     C-string
 *
 * @return     width in pixels.
 */
uint16_t canvas_string_width(Canvas* canvas, const char* str);

/** Get glyph width
 *
 * @param      canvas  Canvas instance
 * @param[in]  symbol  character
 *
 * @return     width in pixels
 */
size_t canvas_glyph_width(Canvas* canvas, uint16_t symbol);

/** Draw bitmap picture at position defined by x,y.
 *
 * @param      canvas                  Canvas instance
 * @param      x                       x coordinate
 * @param      y                       y coordinate
 * @param      width                   width of bitmap
 * @param      height                  height of bitmap
 * @param      compressed_bitmap_data  compressed bitmap data
 */
void canvas_draw_bitmap(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* compressed_bitmap_data);

/** Draw icon at position defined by x,y with rotation and flip.
 *
 * @param      canvas    Canvas instance
 * @param      x         x coordinate
 * @param      y         y coordinate
 * @param      icon      Icon instance
 * @param      rotation  IconRotation
 */
void canvas_draw_icon_ex(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    const Icon* icon,
    IconRotation rotation);

/** Draw animation at position defined by x,y.
 *
 * @param      canvas          Canvas instance
 * @param      x               x coordinate
 * @param      y               y coordinate
 * @param      icon_animation  IconAnimation instance
 */
void canvas_draw_icon_animation(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    IconAnimation* icon_animation);

/** Draw icon at position defined by x,y.
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      icon    Icon instance
 */
void canvas_draw_icon(Canvas* canvas, int32_t x, int32_t y, const Icon* icon);

/** Draw XBM bitmap
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param[in]  width   bitmap width
 * @param[in]  height  bitmap height
 * @param      bitmap  pointer to XBM bitmap data
 */
void canvas_draw_xbm(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* bitmap);

/** Draw rotated XBM bitmap
 *
 * @param      canvas       Canvas instance
 * @param      x            x coordinate
 * @param      y            y coordinate
 * @param[in]  width        bitmap width
 * @param[in]  height       bitmap height
 * @param[in]  rotation     bitmap rotation
 * @param      bitmap_data  pointer to XBM bitmap data
 */
void canvas_draw_xbm_ex(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    IconRotation rotation,
    const uint8_t* bitmap_data);

/** Draw dot at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 */
void canvas_draw_dot(Canvas* canvas, int32_t x, int32_t y);

/** Draw box of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   box width
 * @param      height  box height
 */
void canvas_draw_box(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw frame of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   frame width
 * @param      height  frame height
 */
void canvas_draw_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw line from x1,y1 to x2,y2
 *
 * @param      canvas  Canvas instance
 * @param      x1      x1 coordinate
 * @param      y1      y1 coordinate
 * @param      x2      x2 coordinate
 * @param      y2      y2 coordinate
 */
void canvas_draw_line(Canvas* canvas, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/** Draw circle at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      radius  radius
 */
void canvas_draw_circle(Canvas* canvas, int32_t x, int32_t y, size_t radius);

/** Draw disc at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      radius  radius
 */
void canvas_draw_disc(Canvas* canvas, int32_t x, int32_t y, size_t radius);

/** Draw triangle with given base and height lengths and their intersection
 * coordinate
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate of base and height intersection
 * @param      y       y coordinate of base and height intersection
 * @param      base    length of triangle side
 * @param      height  length of triangle height
 * @param      dir     CanvasDirection triangle orientation
 */
void canvas_draw_triangle(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t base,
    size_t height,
    CanvasDirection dir);

/** Draw glyph
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      ch      character
 */
void canvas_draw_glyph(Canvas* canvas, int32_t x, int32_t y, uint16_t ch);

/** Set transparency mode
 *
 * @param      canvas  Canvas instance
 * @param      alpha   transparency mode
 */
void canvas_set_bitmap_mode(Canvas* canvas, bool alpha);

/** Draw rounded-corner frame of width, height at x,y, with round value radius
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   frame width
 * @param      height  frame height
 * @param      radius  frame corner radius
 */
void canvas_draw_rframe(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius);

/** Draw rounded-corner box of width, height at x,y, with round value raduis
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   box width
 * @param      height  box height
 * @param      radius  box corner radius
 */
void canvas_draw_rbox(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/canvas.h ===

=== BEGIN: applications/services/gui/canvas_i.h ===
/**
 * @file canvas_i.h
 * GUI: internal Canvas API
 */

#pragma once

#include "canvas.h"
#include <u8g2.h>
#include <toolbox/compress.h>
#include <m-array.h>
#include <m-algo.h>
#include <furi.h>

#define ICON_DECOMPRESSOR_BUFFER_SIZE (128u * 64 / 8)

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CanvasCommitCallback)(
    uint8_t* data,
    size_t size,
    CanvasOrientation orientation,
    void* context);

typedef struct {
    CanvasCommitCallback callback;
    void* context;
} CanvasCallbackPair;

ARRAY_DEF(CanvasCallbackPairArray, CanvasCallbackPair, M_POD_OPLIST);

#define M_OPL_CanvasCallbackPairArray_t() ARRAY_OPLIST(CanvasCallbackPairArray, M_POD_OPLIST)

ALGO_DEF(CanvasCallbackPairArray, CanvasCallbackPairArray_t);

/** Canvas structure
 */
struct Canvas {
    u8g2_t fb;
    CanvasOrientation orientation;
    size_t offset_x;
    size_t offset_y;
    size_t width;
    size_t height;
    CompressIcon* compress_icon;
    CanvasCallbackPairArray_t canvas_callback_pair;
    FuriMutex* mutex;
};

/** Allocate memory and initialize canvas
 *
 * @return     Canvas instance
 */
Canvas* canvas_init(void);

/** Free canvas memory
 *
 * @param      canvas  Canvas instance
 */
void canvas_free(Canvas* canvas);

/** Get canvas buffer.
 *
 * @param      canvas  Canvas instance
 *
 * @return     pointer to buffer
 */
uint8_t* canvas_get_buffer(Canvas* canvas);

/** Get canvas buffer size.
 *
 * @param      canvas  Canvas instance
 *
 * @return     size of canvas in bytes
 */
size_t canvas_get_buffer_size(const Canvas* canvas);

/** Set drawing region relative to real screen buffer
 *
 * @param      canvas    Canvas instance
 * @param      offset_x  x coordinate offset
 * @param      offset_y  y coordinate offset
 * @param      width     width
 * @param      height    height
 */
void canvas_frame_set(
    Canvas* canvas,
    int32_t offset_x,
    int32_t offset_y,
    size_t width,
    size_t height);

/** Set canvas orientation
 *
 * @param      canvas       Canvas instance
 * @param      orientation  CanvasOrientation
 */
void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation);

/** Get canvas orientation
 *
 * @param      canvas  Canvas instance
 *
 * @return     CanvasOrientation
 */
CanvasOrientation canvas_get_orientation(const Canvas* canvas);

/** Draw a u8g2 bitmap
 *
 * @param      u8g2     u8g2 instance
 * @param      x        x coordinate
 * @param      y        y coordinate
 * @param      width    width
 * @param      height   height
 * @param      bitmap   bitmap
 * @param      rotation rotation
 */
void canvas_draw_u8g2_bitmap(
    u8g2_t* u8g2,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* bitmap,
    IconRotation rotation);

/** Add canvas commit callback.
 *
 * This callback will be called upon Canvas commit.
 * 
 * @param      canvas    Canvas instance
 * @param      callback  CanvasCommitCallback
 * @param      context   CanvasCommitCallback context
 */
void canvas_add_framebuffer_callback(Canvas* canvas, CanvasCommitCallback callback, void* context);

/** Remove canvas commit callback.
 *
 * @param      canvas    Canvas instance
 * @param      callback  CanvasCommitCallback
 * @param      context   CanvasCommitCallback context
 */
void canvas_remove_framebuffer_callback(
    Canvas* canvas,
    CanvasCommitCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/canvas_i.h ===

=== BEGIN: applications/services/gui/elements.h ===
/**
 * @file elements.h
 * GUI: Elements API
 * 
 * Canvas helpers and UI building blocks.
 * 
 */

#pragma once

#include <stdint.h>
#include <furi.h>
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ELEMENTS_MAX_LINES_NUM  (7)
#define ELEMENTS_BOLD_MARKER    '#'
#define ELEMENTS_MONO_MARKER    '*'
#define ELEMENTS_INVERSE_MARKER '!'

/** Draw progress bar.
 *
 * @param   canvas      Canvas instance
 * @param   x           progress bar position on X axis
 * @param   y           progress bar position on Y axis
 * @param   width       progress bar width
 * @param   progress    progress (0.0 - 1.0)
 */
void elements_progress_bar(Canvas* canvas, int32_t x, int32_t y, size_t width, float progress);

/** Draw progress bar with text.
 *
 * @param   canvas      Canvas instance
 * @param   x           progress bar position on X axis
 * @param   y           progress bar position on Y axis
 * @param   width       progress bar width
 * @param   progress    progress (0.0 - 1.0)
 * @param   text        text to draw
 */
void elements_progress_bar_with_text(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    float progress,
    const char* text);

/** Draw scrollbar on canvas at specific position.
 *
 * @param   canvas  Canvas instance
 * @param   x       scrollbar position on X axis
 * @param   y       scrollbar position on Y axis
 * @param   height  scrollbar height
 * @param   pos     current element
 * @param   total   total elements
 */
void elements_scrollbar_pos(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t height,
    size_t pos,
    size_t total);

/** Draw scrollbar on canvas.
 * @note    width 3px, height equal to canvas height
 *
 * @param   canvas  Canvas instance
 * @param   pos     current element of total elements
 * @param   total   total elements
 */
void elements_scrollbar(Canvas* canvas, size_t pos, size_t total);

/** Draw rounded frame
 *
 * @param   canvas          Canvas instance
 * @param   x, y            top left corner coordinates
 * @param   width, height   frame width and height
 */
void elements_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw button in left corner
 *
 * @param   canvas  Canvas instance
 * @param   str     button text
 */
void elements_button_left(Canvas* canvas, const char* str);

/** Draw button in right corner
 *
 * @param   canvas  Canvas instance
 * @param   str     button text
 */
void elements_button_right(Canvas* canvas, const char* str);

/**
 * @brief This function draws a button in the top left corner of the canvas with icon and string.
 *
 * The design and layout of the button is defined within this function.
 *
 * @param[in] canvas This is a pointer to the @c Canvas structure where the button will be drawn.
 * @param[in] str This is a pointer to the character string that will be drawn within the button.
 *
 */
void elements_button_up(Canvas* canvas, const char* str);

/**
 * @brief This function draws a button in the top right corner of the canvas with icon and string.
 *
 * The design and layout of the button is defined within this function.
 *
 * @param[in] canvas This is a pointer to the @c Canvas structure where the button will be drawn.
 * @param[in] str This is a pointer to the character string that will be drawn within the button.
 *
 */
void elements_button_down(Canvas* canvas, const char* str);

/** Draw button in center
 *
 * @param   canvas  Canvas instance
 * @param   str     button text
 */
void elements_button_center(Canvas* canvas, const char* str);

/** Draw aligned multiline text
 *
 * @param   canvas                  Canvas instance
 * @param   x, y                    coordinates based on align param
 * @param   horizontal, vertical    alignment of multiline text
 * @param   text                    string (possible multiline)
 */
void elements_multiline_text_aligned(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    Align horizontal,
    Align vertical,
    const char* text);

/** Draw multiline text
 *
 * @param   canvas  Canvas instance
 * @param   x       top left corner coordinates
 * @param   y       top left corner coordinates
 * @param   text    string (possible multiline)
 */
void elements_multiline_text(Canvas* canvas, int32_t x, int32_t y, const char* text);

/** Draw framed multiline text
 *
 * @param   canvas  Canvas instance
 * @param   x       top left corner coordinates
 * @param   y       top left corner coordinates
 * @param   text    string (possible multiline)
 */
void elements_multiline_text_framed(Canvas* canvas, int32_t x, int32_t y, const char* text);

/** Draw slightly rounded frame
 *
 * @param   canvas          Canvas instance
 * @param   x               top left corner coordinates
 * @param   y               top left corner coordinates
 * @param   width           width of frame
 * @param   height          height of frame
 */
void elements_slightly_rounded_frame(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height);

/** Draw slightly rounded box
 *
 * @param   canvas          Canvas instance
 * @param   x               top left corner coordinates
 * @param   y               top left corner coordinates
 * @param   width           height of box
 * @param   height          height of box
 */
void elements_slightly_rounded_box(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height);

/** Draw bold rounded frame
 *
 * @param   canvas          Canvas instance
 * @param   x               top left corner coordinates
 * @param   y               top left corner coordinates
 * @param   width           width of frame
 * @param   height          height of frame
 */
void elements_bold_rounded_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw bubble frame for text
 *
 * @param   canvas  Canvas instance
 * @param   x       left x coordinates
 * @param   y       top y coordinate
 * @param   width   bubble width
 * @param   height  bubble height
 */
void elements_bubble(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw bubble frame for text with corner
 *
 * @param   canvas      Canvas instance
 * @param   x           left x coordinates
 * @param   y           top y coordinate
 * @param   text        text to display
 * @param   horizontal  horizontal aligning
 * @param   vertical    aligning
 */
void elements_bubble_str(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    const char* text,
    Align horizontal,
    Align vertical);

/** Trim string buffer to fit width in pixels
 *
 * @param   canvas  Canvas instance
 * @param   string  string to trim
 * @param   width   max width
 */
void elements_string_fit_width(Canvas* canvas, FuriString* string, size_t width);

/** Draw scrollable text line
 *
 * @param      canvas    The canvas
 * @param[in]  x         X coordinate
 * @param[in]  y         Y coordinate
 * @param[in]  width     The width
 * @param      string    The string
 * @param[in]  scroll    The scroll counter: 0 - no scroll, any other number - scroll. Just count up, everything else will be calculated on the inside.
 * @param[in]  ellipsis  The ellipsis flag: true to add ellipse
 */
void elements_scrollable_text_line(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    FuriString* string,
    size_t scroll,
    bool ellipsis);

/** Draw text box element
 *
 * @param       canvas          Canvas instance
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       width           width to fit text
 * @param       height          height to fit text
 * @param       horizontal      Align instance
 * @param       vertical        Align instance
 * @param[in]   text            Formatted text. The following formats are available:
 *                              "\e#Bold text\e#" - bold font is used
 *                              "\e*Monospaced text\e*" - monospaced font is used
 *                              "\e!Inverted text\e!" - white text on black background
 * @param      strip_to_dots    Strip text to ... if does not fit to width
 */
void elements_text_box(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/elements.h ===

=== BEGIN: applications/services/gui/gui.h ===
/**
 * @file gui.h
 * GUI: main API
 */

#pragma once

#include "view_port.h"
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Gui layers */
typedef enum {
    GuiLayerDesktop, /**< Desktop layer for internal use. Like fullscreen but with status bar */

    GuiLayerWindow, /**< Window layer, status bar is shown */

    GuiLayerStatusBarLeft, /**< Status bar left-side layer, auto-layout */
    GuiLayerStatusBarRight, /**< Status bar right-side layer, auto-layout */

    GuiLayerFullscreen, /**< Fullscreen layer, no status bar */

    GuiLayerMAX /**< Don't use or move, special value */
} GuiLayer;

/** Gui Canvas Commit Callback */
typedef void (*GuiCanvasCommitCallback)(
    uint8_t* data,
    size_t size,
    CanvasOrientation orientation,
    void* context);

#define RECORD_GUI "gui"

typedef struct Gui Gui;

/** Add view_port to view_port tree
 *
 * @remark     thread safe
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 * @param[in]  layer      GuiLayer where to place view_port
 */
void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer);

/** Remove view_port from rendering tree
 *
 * @remark     thread safe
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_remove_view_port(Gui* gui, ViewPort* view_port);

/** Send ViewPort to the front
 *
 * Places selected ViewPort to the top of the drawing stack
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_view_port_send_to_front(Gui* gui, ViewPort* view_port);

/** Send ViewPort to the back
 *
 * Places selected ViewPort to the bottom of the drawing stack
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_view_port_send_to_back(Gui* gui, ViewPort* view_port);

/** Add gui canvas commit callback
 *
 * This callback will be called upon Canvas commit Callback dispatched from GUI
 * thread and is time critical
 *
 * @param      gui       Gui instance
 * @param      callback  GuiCanvasCommitCallback
 * @param      context   GuiCanvasCommitCallback context
 */
void gui_add_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback, void* context);

/** Remove gui canvas commit callback
 *
 * @param      gui       Gui instance
 * @param      callback  GuiCanvasCommitCallback
 * @param      context   GuiCanvasCommitCallback context
 */
void gui_remove_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback, void* context);

/** Get gui canvas frame buffer size
 * *
 * @param      gui       Gui instance
 * @return     size_t    size of frame buffer in bytes
 */
size_t gui_get_framebuffer_size(const Gui* gui);

/** Set lockdown mode
 *
 * When lockdown mode is enabled, only GuiLayerDesktop is shown.
 * This feature prevents services from showing sensitive information when flipper is locked.
 *
 * @param      gui       Gui instance
 * @param      lockdown  bool, true if enabled
 */
void gui_set_lockdown(Gui* gui, bool lockdown);

/** Acquire Direct Draw lock and get Canvas instance
 *
 * This method return Canvas instance for use in monopoly mode. Direct draw lock
 * disables input and draw call dispatch functions in GUI service. No other
 * applications or services will be able to draw until gui_direct_draw_release
 * call.
 *
 * @param      gui   The graphical user interface
 *
 * @return     Canvas instance
 */
Canvas* gui_direct_draw_acquire(Gui* gui);

/** Release Direct Draw Lock
 *
 * Release Direct Draw Lock, enables Input and Draw call processing. Canvas
 * acquired in gui_direct_draw_acquire will become invalid after this call.
 *
 * @param      gui   Gui instance
 */
void gui_direct_draw_release(Gui* gui);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/gui.h ===

=== BEGIN: applications/services/gui/gui_i.h ===
/**
 * @file gui_i.h
 * GUI: main API internals
 */

#pragma once

#include "gui.h"

#include <furi.h>
#include <furi_hal_rtc.h>
#include <m-array.h>
#include <stdio.h>

#include "canvas.h"
#include "canvas_i.h"
#include "view_port.h"
#include "view_port_i.h"

#define GUI_DISPLAY_WIDTH  128
#define GUI_DISPLAY_HEIGHT 64

#define GUI_STATUS_BAR_X               0
#define GUI_STATUS_BAR_Y               0
#define GUI_STATUS_BAR_WIDTH           GUI_DISPLAY_WIDTH
/* 0-1 pixels for upper thin frame
 * 2-9 pixels for icons (battery, sd card, etc)
 * 10-12 pixels for lower bold line */
#define GUI_STATUS_BAR_HEIGHT          13
/* icon itself area (battery, sd card, etc) excluding frame.
 * painted 2 pixels below GUI_STATUS_BAR_X.
 */
#define GUI_STATUS_BAR_WORKAREA_HEIGHT 8

#define GUI_WINDOW_X      0
#define GUI_WINDOW_Y      GUI_STATUS_BAR_HEIGHT
#define GUI_WINDOW_WIDTH  GUI_DISPLAY_WIDTH
#define GUI_WINDOW_HEIGHT (GUI_DISPLAY_HEIGHT - GUI_WINDOW_Y)

#define GUI_THREAD_FLAG_DRAW  (1 << 0)
#define GUI_THREAD_FLAG_INPUT (1 << 1)
#define GUI_THREAD_FLAG_ALL   (GUI_THREAD_FLAG_DRAW | GUI_THREAD_FLAG_INPUT)

ARRAY_DEF(ViewPortArray, ViewPort*, M_PTR_OPLIST);

/** Gui structure */
struct Gui {
    // Thread and lock
    FuriThreadId thread_id;
    FuriMutex* mutex;

    // Layers and Canvas
    bool lockdown;
    bool direct_draw;
    ViewPortArray_t layers[GuiLayerMAX];
    Canvas* canvas;

    // Input
    FuriMessageQueue* input_queue;
    FuriPubSub* input_events;
    uint8_t ongoing_input;
    ViewPort* ongoing_input_view_port;
};

/** Find enabled ViewPort in ViewPortArray
 *
 * @param[in]  array  The ViewPortArray instance
 *
 * @return     ViewPort instance or NULL
 */
ViewPort* gui_view_port_find_enabled(ViewPortArray_t array);

/** Update GUI, request redraw
 *
 * @param      gui   Gui instance
 */
void gui_update(Gui* gui);

/** Input event callback
 * 
 * Used to receive input from input service or to inject new input events
 *
 * @param[in]  value  The value pointer (InputEvent*)
 * @param      ctx    The context (Gui instance)
 */
void gui_input_events_callback(const void* value, void* ctx);

/** Get count of view ports in layer
 *
 * @param      gui        The Gui instance
 * @param[in]  layer      GuiLayer that we want to get count of view ports
 */
size_t gui_active_view_port_count(Gui* gui, GuiLayer layer);

/** Lock GUI
 *
 * @param      gui   The Gui instance
 */
void gui_lock(Gui* gui);

/** Unlock GUI
 *
 * @param      gui   The Gui instance
 */
void gui_unlock(Gui* gui);

=== END: applications/services/gui/gui_i.h ===

=== BEGIN: applications/services/gui/icon.h ===
/**
 * @file icon.h
 * GUI: Icon API
 */

#pragma once

#include <stdint.h>
#include <core/common_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Icon Icon;

/** Get icon width 
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     width in pixels
 */
uint16_t icon_get_width(const Icon* instance);

/** Get icon height
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     height in pixels
 */
uint16_t icon_get_height(const Icon* instance);

/** Get Icon XBM bitmap data for the first frame
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     pointer to compressed XBM bitmap data
 */
FURI_DEPRECATED const uint8_t* icon_get_data(const Icon* instance);

/** Get Icon frame count
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     frame count
 */
uint32_t icon_get_frame_count(const Icon* instance);

/** Get Icon XBM bitmap data for a particular frame
 *
 * @param[in]  instance  pointer to Icon data
 * @param[in]  frame     frame index
 *
 * @return     pointer to compressed XBM bitmap data
 */
const uint8_t* icon_get_frame_data(const Icon* instance, uint32_t frame);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/icon.h ===

=== BEGIN: applications/services/gui/icon_animation.h ===
/**
 * @file icon_animation.h
 * GUI: IconAnimation API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/icon.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Icon Animation */
typedef struct IconAnimation IconAnimation;

/** Icon Animation Callback. Used for update notification */
typedef void (*IconAnimationCallback)(IconAnimation* instance, void* context);

/** Allocate icon animation instance with const icon data.
 * 
 * always returns Icon or stops system if not enough memory
 *
 * @param[in]  icon  pointer to Icon data
 *
 * @return     IconAnimation instance
 */
IconAnimation* icon_animation_alloc(const Icon* icon);

/** Release icon animation instance
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_free(IconAnimation* instance);

/** Set IconAnimation update callback
 *
 * Normally you do not need to use this function, use view_tie_icon_animation
 * instead.
 *
 * @param      instance  IconAnimation instance
 * @param[in]  callback  IconAnimationCallback
 * @param      context   callback context
 */
void icon_animation_set_update_callback(
    IconAnimation* instance,
    IconAnimationCallback callback,
    void* context);

/** Get icon animation width
 *
 * @param      instance  IconAnimation instance
 *
 * @return     width in pixels
 */
uint8_t icon_animation_get_width(const IconAnimation* instance);

/** Get icon animation height
 *
 * @param      instance  IconAnimation instance
 *
 * @return     height in pixels
 */
uint8_t icon_animation_get_height(const IconAnimation* instance);

/** Start icon animation
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_start(IconAnimation* instance);

/** Stop icon animation
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_stop(IconAnimation* instance);

/** Returns true if current frame is a last one
 *
 * @param      instance  IconAnimation instance
 *
 * @return     true if last frame
 */
bool icon_animation_is_last_frame(const IconAnimation* instance);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/icon_animation.h ===

=== BEGIN: applications/services/gui/icon_animation_i.h ===
/**
 * @file icon_animation_i.h
 * GUI: internal IconAnimation API
 */

#pragma once

#include "icon_animation.h"

#include <furi.h>

struct IconAnimation {
    const Icon* icon;
    uint8_t frame;
    bool animating;
    FuriTimer* timer;
    IconAnimationCallback callback;
    void* callback_context;
};

/** Get pointer to current frame data
 *
 * @param      instance  IconAnimation instance
 *
 * @return     pointer to current frame XBM bitmap data
 */
const uint8_t* icon_animation_get_data(const IconAnimation* instance);

/** Advance to next frame
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_next_frame(IconAnimation* instance);

/** IconAnimation timer callback
 *
 * @param      context  pointer to IconAnimation
 */
void icon_animation_timer_callback(void* context);

=== END: applications/services/gui/icon_animation_i.h ===

=== BEGIN: applications/services/gui/icon_i.h ===
/**
 * @file icon_i.h
 * GUI: internal Icon API
 */

#pragma once
#include <stdint.h>

struct Icon {
    const uint16_t width;
    const uint16_t height;
    const uint8_t frame_count;
    const uint8_t frame_rate;
    const uint8_t* const* frames;
};

=== END: applications/services/gui/icon_i.h ===

=== BEGIN: applications/services/gui/modules/button_menu.h ===
/**
 * @file button_menu.h
 * GUI: ButtonMenu view module API
 */

#pragma once

#include <stdint.h>
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ButtonMenu anonymous structure */
typedef struct ButtonMenu ButtonMenu;

/** ButtonMenuItem anonymous structure */
typedef struct ButtonMenuItem ButtonMenuItem;

/** Callback for any button menu actions */
typedef void (*ButtonMenuItemCallback)(void* context, int32_t index, InputType type);

/** Type of button. Difference in drawing buttons. */
typedef enum {
    ButtonMenuItemTypeCommon,
    ButtonMenuItemTypeControl,
} ButtonMenuItemType;

/** Get button menu view
 *
 * @param      button_menu  ButtonMenu instance
 *
 * @return     View instance that can be used for embedding
 */
View* button_menu_get_view(ButtonMenu* button_menu);

/** Clean button menu
 *
 * @param      button_menu  ButtonMenu instance
 */
void button_menu_reset(ButtonMenu* button_menu);

/** Add item to button menu instance
 *
 * @param      button_menu       ButtonMenu instance
 * @param      label             text inside new button
 * @param      index             value to distinct between buttons inside
 *                               ButtonMenuItemCallback
 * @param      callback          The callback
 * @param      type              type of button to create. Differ by button
 *                               drawing. Control buttons have no frames, and
 *                               have more squared borders.
 * @param      callback_context  The callback context
 *
 * @return     pointer to just-created item
 */
ButtonMenuItem* button_menu_add_item(
    ButtonMenu* button_menu,
    const char* label,
    int32_t index,
    ButtonMenuItemCallback callback,
    ButtonMenuItemType type,
    void* callback_context);

/** Allocate and initialize new instance of ButtonMenu model
 *
 * @return     just-created ButtonMenu model
 */
ButtonMenu* button_menu_alloc(void);

/** Free ButtonMenu element
 *
 * @param      button_menu  ButtonMenu instance
 */
void button_menu_free(ButtonMenu* button_menu);

/** Set ButtonMenu header on top of canvas
 *
 * @param      button_menu  ButtonMenu instance
 * @param      header       header on the top of button menu
 */
void button_menu_set_header(ButtonMenu* button_menu, const char* header);

/** Set selected item
 *
 * @param      button_menu  ButtonMenu instance
 * @param      index        index of ButtonMenu to be selected
 */
void button_menu_set_selected_item(ButtonMenu* button_menu, uint32_t index);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/button_menu.h ===

=== BEGIN: applications/services/gui/modules/button_panel.h ===
/**
 * @file button_panel.h
 * GUI: ButtonPanel view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Button panel module descriptor */
typedef struct ButtonPanel ButtonPanel;

/** Callback type to call for handling selecting button_panel items */
typedef void (*ButtonItemCallback)(void* context, uint32_t index);

/** Allocate new button_panel module.
 *
 * @return     ButtonPanel instance
 */
ButtonPanel* button_panel_alloc(void);

/** Free button_panel module.
 *
 * @param      button_panel  ButtonPanel instance
 */
void button_panel_free(ButtonPanel* button_panel);

/** Free items from button_panel module. Preallocated matrix stays unchanged.
 *
 * @param      button_panel  ButtonPanel instance
 */
void button_panel_reset(ButtonPanel* button_panel);

/** Reserve space for adding items.
 *
 * One does not simply use button_panel_add_item() without this function. It
 * should be allocated space for it first.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      reserve_x     number of columns in button_panel
 * @param      reserve_y     number of rows in button_panel
 */
void button_panel_reserve(ButtonPanel* button_panel, size_t reserve_x, size_t reserve_y);

/** Add item to button_panel module.
 *
 * Have to set element in bounds of allocated size by X and by Y.
 *
 * @param      button_panel        ButtonPanel instance
 * @param      index               value to pass to callback
 * @param      matrix_place_x      coordinates by x-axis on virtual grid, it
 *                                 is only used for navigation
 * @param      matrix_place_y      coordinates by y-axis on virtual grid, it
 *                                 is only used for naviagation
 * @param      x                   x-coordinate to draw icon on
 * @param      y                   y-coordinate to draw icon on
 * @param      icon_name           name of the icon to draw
 * @param      icon_name_selected  name of the icon to draw when current
 *                                 element is selected
 * @param      callback            function to call when specific element is
 *                                 selected (pressed Ok on selected item)
 * @param      callback_context    context to pass to callback
 */
void button_panel_add_item(
    ButtonPanel* button_panel,
    uint32_t index,
    uint16_t matrix_place_x,
    uint16_t matrix_place_y,
    uint16_t x,
    uint16_t y,
    const Icon* icon_name,
    const Icon* icon_name_selected,
    ButtonItemCallback callback,
    void* callback_context);

/** Get button_panel view.
 *
 * @param      button_panel  ButtonPanel instance
 *
 * @return     acquired view
 */
View* button_panel_get_view(ButtonPanel* button_panel);

/** Add label to button_panel module.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      x             x-coordinate to place label
 * @param      y             y-coordinate to place label
 * @param      font          font to write label with
 * @param      label_str     string label to write
 */
void button_panel_add_label(
    ButtonPanel* button_panel,
    uint16_t x,
    uint16_t y,
    Font font,
    const char* label_str);

/** Add a non-button icon to button_panel module.
 *
 * @param      button_panel  ButtonPanel instance
 * @param      x             x-coordinate to place icon
 * @param      y             y-coordinate to place icon
 * @param      icon_name     name of the icon to draw
 */
void button_panel_add_icon(
    ButtonPanel* button_panel,
    uint16_t x,
    uint16_t y,
    const Icon* icon_name);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/button_panel.h ===

=== BEGIN: applications/services/gui/modules/byte_input.h ===
/**
 * @file byte_input.h
 * GUI: ByteInput keyboard view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Byte input anonymous structure  */
typedef struct ByteInput ByteInput;

/** callback that is executed on save button press */
typedef void (*ByteInputCallback)(void* context);

/** callback that is executed when byte buffer is changed */
typedef void (*ByteChangedCallback)(void* context);

/** Allocate and initialize byte input. This byte input is used to enter bytes.
 *
 * @return     ByteInput instance pointer
 */
ByteInput* byte_input_alloc(void);

/** Deinitialize and free byte input
 *
 * @param      byte_input  Byte input instance
 */
void byte_input_free(ByteInput* byte_input);

/** Get byte input view
 *
 * @param      byte_input  byte input instance
 *
 * @return     View instance that can be used for embedding
 */
View* byte_input_get_view(ByteInput* byte_input);

/** Set byte input result callback
 *
 * @param      byte_input        byte input instance
 * @param      input_callback    input callback fn
 * @param      changed_callback  changed callback fn
 * @param      callback_context  callback context
 * @param      bytes             buffer to use
 * @param      bytes_count       buffer length
 */
void byte_input_set_result_callback(
    ByteInput* byte_input,
    ByteInputCallback input_callback,
    ByteChangedCallback changed_callback,
    void* callback_context,
    uint8_t* bytes,
    uint8_t bytes_count);

/** Set byte input header text
 *
 * @param      byte_input  byte input instance
 * @param      text        text to be shown
 */
void byte_input_set_header_text(ByteInput* byte_input, const char* text);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/byte_input.h ===

=== BEGIN: applications/services/gui/modules/dialog_ex.h ===
/**
 * @file dialog_ex.h
 * GUI: DialogEx view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Dialog anonymous structure */
typedef struct DialogEx DialogEx;

/** DialogEx result */
typedef enum {
    DialogExResultLeft,
    DialogExResultCenter,
    DialogExResultRight,
    DialogExPressLeft,
    DialogExPressCenter,
    DialogExPressRight,
    DialogExReleaseLeft,
    DialogExReleaseCenter,
    DialogExReleaseRight,
} DialogExResult;

/** DialogEx result callback type
 * @warning comes from GUI thread
 */
typedef void (*DialogExResultCallback)(DialogExResult result, void* context);

/** Allocate and initialize dialog
 *
 * This dialog used to ask simple questions
 *
 * @return     DialogEx instance
 */
DialogEx* dialog_ex_alloc(void);

/** Deinitialize and free dialog
 *
 * @param      dialog_ex  DialogEx instance
 */
void dialog_ex_free(DialogEx* dialog_ex);

/** Get dialog view
 *
 * @param      dialog_ex  DialogEx instance
 *
 * @return     View instance that can be used for embedding
 */
View* dialog_ex_get_view(DialogEx* dialog_ex);

/** Set dialog result callback
 *
 * @param      dialog_ex  DialogEx instance
 * @param      callback   result callback function
 */
void dialog_ex_set_result_callback(DialogEx* dialog_ex, DialogExResultCallback callback);

/** Set dialog context
 *
 * @param      dialog_ex  DialogEx instance
 * @param      context    context pointer, will be passed to result callback
 */
void dialog_ex_set_context(DialogEx* dialog_ex, void* context);

/** Set dialog header text
 *
 * If text is null, dialog header will not be rendered
 *
 * @param      dialog_ex   DialogEx instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal text alignment
 * @param      vertical    vertical text alignment
 */
void dialog_ex_set_header(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set dialog text
 *
 * If text is null, dialog text will not be rendered
 *
 * @param      dialog_ex   DialogEx instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal text alignment
 * @param      vertical    vertical text alignment
 */
void dialog_ex_set_text(
    DialogEx* dialog_ex,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set dialog icon
 *
 * If x or y is negative, dialog icon will not be rendered
 *
 * @param      dialog_ex  DialogEx instance
 * @param      x          x position
 * @param      y          y position
 * @param      icon       The icon
 */
void dialog_ex_set_icon(DialogEx* dialog_ex, uint8_t x, uint8_t y, const Icon* icon);

/** Set left button text
 *
 * If text is null, left button will not be rendered and processed
 *
 * @param      dialog_ex  DialogEx instance
 * @param      text       text to be shown
 */
void dialog_ex_set_left_button_text(DialogEx* dialog_ex, const char* text);

/** Set center button text
 *
 * If text is null, center button will not be rendered and processed
 *
 * @param      dialog_ex  DialogEx instance
 * @param      text       text to be shown
 */
void dialog_ex_set_center_button_text(DialogEx* dialog_ex, const char* text);

/** Set right button text
 *
 * If text is null, right button will not be rendered and processed
 *
 * @param      dialog_ex  DialogEx instance
 * @param      text       text to be shown
 */
void dialog_ex_set_right_button_text(DialogEx* dialog_ex, const char* text);

/** Clean dialog
 *
 * @param      dialog_ex  DialogEx instance
 */
void dialog_ex_reset(DialogEx* dialog_ex);

/** Enable press/release events
 *
 * @param      dialog_ex  DialogEx instance
 */
void dialog_ex_enable_extended_events(DialogEx* dialog_ex);

/** Disable press/release events
 *
 * @param      dialog_ex  DialogEx instance
 */
void dialog_ex_disable_extended_events(DialogEx* dialog_ex);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/dialog_ex.h ===

=== BEGIN: applications/services/gui/modules/empty_screen.h ===
/**
 * @file empty_screen.h
 * GUI: EmptyScreen view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Empty screen anonymous structure */
typedef struct EmptyScreen EmptyScreen;

/** Allocate and initialize empty screen
 *
 * This empty screen used to ask simple questions like Yes/
 *
 * @return     EmptyScreen instance
 */
EmptyScreen* empty_screen_alloc(void);

/** Deinitialize and free empty screen
 *
 * @param      empty_screen  Empty screen instance
 */
void empty_screen_free(EmptyScreen* empty_screen);

/** Get empty screen view
 *
 * @param      empty_screen  Empty screen instance
 *
 * @return     View instance that can be used for embedding
 */
View* empty_screen_get_view(EmptyScreen* empty_screen);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/empty_screen.h ===

=== BEGIN: applications/services/gui/modules/file_browser.h ===
/**
 * @file file_browser.h
 * GUI: FileBrowser view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FileBrowser FileBrowser;
typedef void (*FileBrowserCallback)(void* context);

typedef bool (*FileBrowserLoadItemCallback)(
    FuriString* path,
    void* context,
    uint8_t** icon,
    FuriString* item_name);

FileBrowser* file_browser_alloc(FuriString* result_path);

void file_browser_free(FileBrowser* browser);

View* file_browser_get_view(FileBrowser* browser);

void file_browser_configure(
    FileBrowser* browser,
    const char* extension,
    const char* base_path,
    bool skip_assets,
    bool hide_dot_files,
    const Icon* file_icon,
    bool hide_ext);

void file_browser_start(FileBrowser* browser, FuriString* path);

void file_browser_stop(FileBrowser* browser);

void file_browser_set_callback(FileBrowser* browser, FileBrowserCallback callback, void* context);

void file_browser_set_item_callback(
    FileBrowser* browser,
    FileBrowserLoadItemCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/file_browser.h ===

=== BEGIN: applications/services/gui/modules/file_browser_worker.h ===
#pragma once

#include <gui/view.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BrowserWorker BrowserWorker;
typedef void (*BrowserWorkerFolderOpenCallback)(
    void* context,
    uint32_t item_cnt,
    int32_t file_idx,
    bool is_root);
typedef void (*BrowserWorkerListLoadCallback)(void* context, uint32_t list_load_offset);
typedef void (*BrowserWorkerListItemCallback)(
    void* context,
    FuriString* item_path,
    bool is_folder,
    bool is_last);
typedef void (*BrowserWorkerLongLoadCallback)(void* context);

BrowserWorker* file_browser_worker_alloc(
    FuriString* path,
    const char* base_path,
    const char* ext_filter,
    bool skip_assets,
    bool hide_dot_files);

void file_browser_worker_free(BrowserWorker* browser);

void file_browser_worker_set_callback_context(BrowserWorker* browser, void* context);

void file_browser_worker_set_folder_callback(
    BrowserWorker* browser,
    BrowserWorkerFolderOpenCallback cb);

void file_browser_worker_set_list_callback(
    BrowserWorker* browser,
    BrowserWorkerListLoadCallback cb);

void file_browser_worker_set_item_callback(
    BrowserWorker* browser,
    BrowserWorkerListItemCallback cb);

void file_browser_worker_set_long_load_callback(
    BrowserWorker* browser,
    BrowserWorkerLongLoadCallback cb);

void file_browser_worker_set_config(
    BrowserWorker* browser,
    FuriString* path,
    const char* ext_filter,
    bool skip_assets,
    bool hide_dot_files);

void file_browser_worker_folder_enter(BrowserWorker* browser, FuriString* path, int32_t item_idx);

bool file_browser_worker_is_in_start_folder(BrowserWorker* browser);

void file_browser_worker_folder_exit(BrowserWorker* browser);

void file_browser_worker_folder_refresh(BrowserWorker* browser, int32_t item_idx);

void file_browser_worker_load(BrowserWorker* browser, uint32_t offset, uint32_t count);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/file_browser_worker.h ===

=== BEGIN: applications/services/gui/modules/loading.h ===
#pragma once
#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Loading anonymous structure */
typedef struct Loading Loading;

/** Allocate and initialize
 *
 * This View used to show system is doing some processing
 *
 * @return     Loading View instance
 */
Loading* loading_alloc(void);

/** Deinitialize and free Loading View
 *
 * @param      instance  Loading instance
 */
void loading_free(Loading* instance);

/** Get Loading view
 *
 * @param      instance  Loading instance
 *
 * @return     View instance that can be used for embedding
 */
View* loading_get_view(Loading* instance);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/loading.h ===

=== BEGIN: applications/services/gui/modules/menu.h ===
/**
 * @file menu.h
 * GUI: Menu view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Menu anonymous structure */
typedef struct Menu Menu;

/** Menu Item Callback */
typedef void (*MenuItemCallback)(void* context, uint32_t index);

/** Menu allocation and initialization
 *
 * @return     Menu instance
 */
Menu* menu_alloc(void);

/** Free menu
 *
 * @param      menu  Menu instance
 */
void menu_free(Menu* menu);

/** Get Menu view
 *
 * @param      menu  Menu instance
 *
 * @return     View instance
 */
View* menu_get_view(Menu* menu);

/** Add item to menu
 *
 * @param      menu      Menu instance
 * @param      label     menu item string label
 * @param      icon      IconAnimation instance
 * @param      index     menu item index
 * @param      callback  MenuItemCallback instance
 * @param      context   pointer to context
 */
void menu_add_item(
    Menu* menu,
    const char* label,
    const Icon* icon,
    uint32_t index,
    MenuItemCallback callback,
    void* context);

/** Clean menu
 * @note       this function does not free menu instance
 *
 * @param      menu  Menu instance
 */
void menu_reset(Menu* menu);

/** Set current menu item
 *
 * @param      menu   Menu instance
 * @param      index  The index
 */
void menu_set_selected_item(Menu* menu, uint32_t index);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/menu.h ===

=== BEGIN: applications/services/gui/modules/number_input.h ===
/**
 * @file number_input.h
 * GUI: Integer string keyboard view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number input anonymous structure  */
typedef struct NumberInput NumberInput;

/** Callback to be called on save button press */
typedef void (*NumberInputCallback)(void* context, int32_t number);

/** Allocate and initialize Number input.
 *
 * This Number input is used to enter Numbers (Integers).
 *
 * @return     NumberInput instance pointer
 */
NumberInput* number_input_alloc(void);

/** Deinitialize and free byte input
 *
 * @param      number_input  Number input instance
 */
void number_input_free(NumberInput* number_input);

/** Get byte input view
 *
 * @param      number_input  byte input instance
 *
 * @return     View instance that can be used for embedding
 */
View* number_input_get_view(NumberInput* number_input);

/** Set byte input result callback
 *
 * @param      number_input      byte input instance
 * @param      input_callback    input callback fn
 * @param      callback_context  callback context
 * @param[in]  current_number    The current number
 * @param      min_value         Min number value
 * @param      max_value         Max number value
 */

void number_input_set_result_callback(
    NumberInput* number_input,
    NumberInputCallback input_callback,
    void* callback_context,
    int32_t current_number,
    int32_t min_value,
    int32_t max_value);

/** Set byte input header text
 *
 * @param      number_input  byte input instance
 * @param      text          text to be shown
 */
void number_input_set_header_text(NumberInput* number_input, const char* text);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/number_input.h ===

=== BEGIN: applications/services/gui/modules/popup.h ===
/**
 * @file popup.h
 * GUI: Popup view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Popup anonymous structure */
typedef struct Popup Popup;

/** Popup result callback type
 * @warning    comes from GUI thread
 */
typedef void (*PopupCallback)(void* context);

/** Allocate and initialize popup
 *
 * This popup used to ask simple questions like Yes/
 *
 * @return     Popup instance
 */
Popup* popup_alloc(void);

/** Deinitialize and free popup
 *
 * @param      popup  Popup instance
 */
void popup_free(Popup* popup);

/** Get popup view
 *
 * @param      popup  Popup instance
 *
 * @return     View instance that can be used for embedding
 */
View* popup_get_view(Popup* popup);

/** Set popup header text
 *
 * @param      popup     Popup instance
 * @param      callback  PopupCallback
 */
void popup_set_callback(Popup* popup, PopupCallback callback);

/** Set popup context
 *
 * @param      popup    Popup instance
 * @param      context  context pointer, will be passed to result callback
 */
void popup_set_context(Popup* popup, void* context);

/** Set popup header text
 *
 * If text is null, popup header will not be rendered
 *
 * @param      popup       Popup instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 */
void popup_set_header(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set popup text
 *
 * If text is null, popup text will not be rendered
 *
 * @param      popup       Popup instance
 * @param      text        text to be shown, can be multiline
 * @param      x           x position
 * @param      y           y position
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 */
void popup_set_text(
    Popup* popup,
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical);

/** Set popup icon
 *
 * If icon position is negative, popup icon will not be rendered
 *
 * @param      popup  Popup instance
 * @param      x      x position
 * @param      y      y position
 * @param      icon   pointer to Icon data
 */
void popup_set_icon(Popup* popup, uint8_t x, uint8_t y, const Icon* icon);

/** Set popup timeout
 *
 * @param      popup          Popup instance
 * @param      timeout_in_ms  popup timeout value in milliseconds
 */
void popup_set_timeout(Popup* popup, uint32_t timeout_in_ms);

/** Enable popup timeout
 *
 * @param      popup  Popup instance
 */
void popup_enable_timeout(Popup* popup);

/** Disable popup timeout
 *
 * @param      popup  Popup instance
 */
void popup_disable_timeout(Popup* popup);

/** Reset popup instance state
 *
 * @param       popup Popup instance
 */
void popup_reset(Popup* popup);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/popup.h ===

=== BEGIN: applications/services/gui/modules/submenu.h ===
/**
 * @file submenu.h
 * GUI: SubMenu view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Submenu anonymous structure */
typedef struct Submenu Submenu;
typedef void (*SubmenuItemCallback)(void* context, uint32_t index);

/** Allocate and initialize submenu 
 * 
 * This submenu is used to select one option
 *
 * @return     Submenu instance
 */
Submenu* submenu_alloc(void);

/** Deinitialize and free submenu
 *
 * @param      submenu  Submenu instance
 */
void submenu_free(Submenu* submenu);

/** Get submenu view
 *
 * @param      submenu  Submenu instance
 *
 * @return     View instance that can be used for embedding
 */
View* submenu_get_view(Submenu* submenu);

/** Add item to submenu
 *
 * @param      submenu           Submenu instance
 * @param      label             menu item label
 * @param      index             menu item index, used for callback, may be
 *                               the same with other items
 * @param      callback          menu item callback
 * @param      callback_context  menu item callback context
 */
void submenu_add_item(
    Submenu* submenu,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* callback_context);

/** Change label of an existing item
 * 
 * @param      submenu  Submenu instance
 * @param      index    The index of the item
 * @param      label    The new label
 */
void submenu_change_item_label(Submenu* submenu, uint32_t index, const char* label);

/** Remove all items from submenu
 *
 * @param      submenu  Submenu instance
 */
void submenu_reset(Submenu* submenu);

/** Get submenu selected item index
 *
 * @param      submenu  Submenu instance
 *
 * @return     Index of the selected item
 */
uint32_t submenu_get_selected_item(Submenu* submenu);

/** Set submenu selected item by index
 *
 * @param      submenu  Submenu instance
 * @param      index    The index of the selected item
 */
void submenu_set_selected_item(Submenu* submenu, uint32_t index);

/** Set optional header for submenu
 *
 * @param      submenu  Submenu instance
 * @param      header   header to set
 */
void submenu_set_header(Submenu* submenu, const char* header);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/submenu.h ===

=== BEGIN: applications/services/gui/modules/text_box.h ===
/**
 * @file text_box.h
 * GUI: TextBox view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TextBox anonymous structure */
typedef struct TextBox TextBox;

typedef enum {
    TextBoxFontText,
    TextBoxFontHex,
} TextBoxFont;

typedef enum {
    TextBoxFocusStart,
    TextBoxFocusEnd,
} TextBoxFocus;

/** Allocate and initialize text_box
 *
 * @return     TextBox instance
 */
TextBox* text_box_alloc(void);

/** Deinitialize and free text_box
 *
 * @param      text_box  text_box instance
 */
void text_box_free(TextBox* text_box);

/** Get text_box view
 *
 * @param      text_box  TextBox instance
 *
 * @return     View instance that can be used for embedding
 */
View* text_box_get_view(TextBox* text_box);

/** Clean text_box
 *
 * @param      text_box  TextBox instance
 */
void text_box_reset(TextBox* text_box);

/** Set text for text_box
 *
 * @param      text_box  TextBox instance
 * @param      text      text to set
 */
void text_box_set_text(TextBox* text_box, const char* text);

/** Set TextBox font
 *
 * @param      text_box  TextBox instance
 * @param      font      TextBoxFont instance
 */
void text_box_set_font(TextBox* text_box, TextBoxFont font);

/** Set TextBox focus
 * @note Use to display from start or from end
 *
 * @param      text_box  TextBox instance
 * @param      focus     TextBoxFocus instance
 */
void text_box_set_focus(TextBox* text_box, TextBoxFocus focus);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/text_box.h ===

=== BEGIN: applications/services/gui/modules/text_input.h ===
/**
 * @file text_input.h
 * GUI: TextInput keyboard view module API
 */

#pragma once

#include <gui/view.h>
#include "validators.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Text input anonymous structure */
typedef struct TextInput TextInput;
typedef void (*TextInputCallback)(void* context);
typedef bool (*TextInputValidatorCallback)(const char* text, FuriString* error, void* context);

/** Allocate and initialize text input 
 * 
 * This text input is used to enter string
 *
 * @return     TextInput instance
 */
TextInput* text_input_alloc(void);

/** Deinitialize and free text input
 *
 * @param      text_input  TextInput instance
 */
void text_input_free(TextInput* text_input);

/** Clean text input view Note: this function does not free memory
 *
 * @param      text_input  Text input instance
 */
void text_input_reset(TextInput* text_input);

/** Get text input view
 *
 * @param      text_input  TextInput instance
 *
 * @return     View instance that can be used for embedding
 */
View* text_input_get_view(TextInput* text_input);

/** Set text input result callback
 *
 * @param      text_input          TextInput instance
 * @param      callback            callback fn
 * @param      callback_context    callback context
 * @param      text_buffer         pointer to YOUR text buffer, that we going
 *                                 to modify
 * @param      text_buffer_size    YOUR text buffer size in bytes. Max string
 *                                 length will be text_buffer_size-1.
 * @param      clear_default_text  clear text from text_buffer on first OK
 *                                 event
 */
void text_input_set_result_callback(
    TextInput* text_input,
    TextInputCallback callback,
    void* callback_context,
    char* text_buffer,
    size_t text_buffer_size,
    bool clear_default_text);

/**
 * @brief Sets the minimum length of a TextInput
 * @param [in] text_input TextInput
 * @param [in] minimum_length Minimum input length
 */
void text_input_set_minimum_length(TextInput* text_input, size_t minimum_length);

void text_input_set_validator(
    TextInput* text_input,
    TextInputValidatorCallback callback,
    void* callback_context);

TextInputValidatorCallback text_input_get_validator_callback(TextInput* text_input);

void* text_input_get_validator_callback_context(TextInput* text_input);

/** Set text input header text
 *
 * @param      text_input  TextInput instance
 * @param      text        text to be shown
 */
void text_input_set_header_text(TextInput* text_input, const char* text);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/text_input.h ===

=== BEGIN: applications/services/gui/modules/validators.h ===
#pragma once

#include <core/common_defines.h>
#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct ValidatorIsFile ValidatorIsFile;

ValidatorIsFile* validator_is_file_alloc_init(
    const char* app_path_folder,
    const char* app_extension,
    const char* current_name);

void validator_is_file_free(ValidatorIsFile* instance);

bool validator_is_file_callback(const char* text, FuriString* error, void* context);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/validators.h ===

=== BEGIN: applications/services/gui/modules/variable_item_list.h ===
/**
 * @file variable_item_list.h
 * GUI: VariableItemList view module API
 */

#pragma once

#include <gui/view.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VariableItemList VariableItemList;
typedef struct VariableItem VariableItem;
typedef void (*VariableItemChangeCallback)(VariableItem* item);
typedef void (*VariableItemListEnterCallback)(void* context, uint32_t index);

/** Allocate and initialize VariableItemList
 *
 * @return     VariableItemList*
 */
VariableItemList* variable_item_list_alloc(void);

/** Deinitialize and free VariableItemList
 *
 * @param      variable_item_list  VariableItemList instance
 */
void variable_item_list_free(VariableItemList* variable_item_list);

/** Clear all elements from list
 *
 * @param      variable_item_list  VariableItemList instance
 */
void variable_item_list_reset(VariableItemList* variable_item_list);

/** Get VariableItemList View instance
 *
 * @param      variable_item_list  VariableItemList instance
 *
 * @return     View instance
 */
View* variable_item_list_get_view(VariableItemList* variable_item_list);

/** Add item to VariableItemList
 *
 * @param      variable_item_list  VariableItemList instance
 * @param      label               item name
 * @param      values_count        item values count
 * @param      change_callback     called on value change in gui
 * @param      context             item context
 *
 * @return     VariableItem* item instance
 */
VariableItem* variable_item_list_add(
    VariableItemList* variable_item_list,
    const char* label,
    uint8_t values_count,
    VariableItemChangeCallback change_callback,
    void* context);

/** Set enter callback
 *
 * @param      variable_item_list  VariableItemList instance
 * @param      callback            VariableItemListEnterCallback instance
 * @param      context             pointer to context
 */
void variable_item_list_set_enter_callback(
    VariableItemList* variable_item_list,
    VariableItemListEnterCallback callback,
    void* context);

void variable_item_list_set_selected_item(VariableItemList* variable_item_list, uint8_t index);

uint8_t variable_item_list_get_selected_item_index(VariableItemList* variable_item_list);

/** Set item current selected index
 *
 * @param      item                 VariableItem* instance
 * @param      current_value_index  The current value index
 */
void variable_item_set_current_value_index(VariableItem* item, uint8_t current_value_index);

/** Set number of values for item
 *
 * @param      item                 VariableItem* instance
 * @param      values_count         The new values count
 */
void variable_item_set_values_count(VariableItem* item, uint8_t values_count);

/** Set item current selected text
 *
 * @param      item                VariableItem* instance
 * @param      current_value_text  The current value text
 */
void variable_item_set_current_value_text(VariableItem* item, const char* current_value_text);

/** Get item current selected index
 *
 * @param      item  VariableItem* instance
 *
 * @return     uint8_t current selected index
 */
uint8_t variable_item_get_current_value_index(VariableItem* item);

/** Get item context
 *
 * @param      item  VariableItem* instance
 *
 * @return     void* item context
 */
void* variable_item_get_context(VariableItem* item);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/variable_item_list.h ===

=== BEGIN: applications/services/gui/modules/widget.h ===
/**
 * @file widget.h
 * GUI: Widget view module API
 */

#pragma once
#include <gui/view.h>
#include "widget_elements/widget_element.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Widget Widget;
typedef struct WidgetElement WidgetElement;

/** Allocate Widget that holds Widget Elements
 *
 * @return     Widget instance
 */
Widget* widget_alloc(void);

/** Free Widget
 * @note       this function free allocated Widget Elements
 *
 * @param      widget  Widget instance
 */
void widget_free(Widget* widget);

/** Reset Widget
 *
 * @param      widget  Widget instance
 */
void widget_reset(Widget* widget);

/** Get Widget view
 *
 * @param      widget  Widget instance
 *
 * @return     View instance
 */
View* widget_get_view(Widget* widget);

/** Add Multi String Element
 *
 * @param      widget      Widget instance
 * @param      x           x coordinate
 * @param      y           y coordinate
 * @param      horizontal  Align instance
 * @param      vertical    Align instance
 * @param      font        Font instance
 * @param[in]  text        The text
 */
void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add String Element
 *
 * @param      widget      Widget instance
 * @param      x           x coordinate
 * @param      y           y coordinate
 * @param      horizontal  Align instance
 * @param      vertical    Align instance
 * @param      font        Font instance
 * @param[in]  text        The text
 */
void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Add Text Box Element
 *
 * @param      widget           Widget instance
 * @param      x                x coordinate
 * @param      y                y coordinate
 * @param      width            width to fit text
 * @param      height           height to fit text
 * @param      horizontal       Align instance
 * @param      vertical         Align instance
 * @param[in]  text             Formatted text. The following formats are available:
 *                               "\e#Bold text\e#" - bold font is used
 *                               "\e*Monospaced text\e*" - monospaced font is used
 *                               "\e!Inversed text\e!" - white text on black background
 * @param      strip_to_dots    Strip text to ... if does not fit to width
 */
void widget_add_text_box_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots);

/** Add Text Scroll Element
 *
 * @param      widget           Widget instance
 * @param      x                x coordinate
 * @param      y                y coordinate
 * @param      width            width to fit text
 * @param      height           height to fit text
 * @param[in]  text             Formatted text. Default format: align left, Secondary font.
 *                              The following formats are available:
 *                               "\e#Bold text" - sets bold font before until next '\n' symbol
 *                               "\e*Monospaced text\e*" - sets monospaced font before until next '\n' symbol
 *                               "\ecCenter-aligned text" - sets center horizontal align until the next '\n' symbol
 *                               "\erRight-aligned text" - sets right horizontal align until the next '\n' symbol
 */
void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text);

/** Add Button Element
 *
 * @param      widget       Widget instance
 * @param      button_type  GuiButtonType instance
 * @param      text         text on allocated button
 * @param      callback     ButtonCallback instance
 * @param      context      pointer to context
 */
void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

/** Add Icon Element
 *
 * @param      widget  Widget instance
 * @param      x       top left x coordinate
 * @param      y       top left y coordinate
 * @param      icon    Icon instance
 */
void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon);

/** Add Frame Element
 *
 * @param      widget  Widget instance
 * @param      x       top left x coordinate
 * @param      y       top left y coordinate
 * @param      width   frame width
 * @param      height  frame height
 * @param      radius  frame radius
 */
void widget_add_frame_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/widget.h ===

=== BEGIN: applications/services/gui/modules/widget_elements/widget_element.h ===
/**
 * @file widget_element_i.h
 * GUI: internal Widget Element API
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GuiButtonTypeLeft,
    GuiButtonTypeCenter,
    GuiButtonTypeRight,
} GuiButtonType;

typedef void (*ButtonCallback)(GuiButtonType result, InputType type, void* context);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/widget_elements/widget_element.h ===

=== BEGIN: applications/services/gui/modules/widget_elements/widget_element_i.h ===
/**
 * @file widget_element_i.h
 * GUI: internal Widget Element API
 */

#pragma once

#include "../widget.h"
#include "widget_element.h"
#include <furi.h>
#include <gui/view.h>
#include <input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WidgetElement WidgetElement;
typedef struct Widget Widget;

struct WidgetElement {
    // generic draw and input callbacks
    void (*draw)(Canvas* canvas, WidgetElement* element);
    bool (*input)(InputEvent* event, WidgetElement* element);

    // free callback
    void (*free)(WidgetElement* element);

    // generic model holder
    void* model;
    FuriMutex* model_mutex;

    // pointer to widget that hold our element
    Widget* parent;
};

/** Create multi string element */
WidgetElement* widget_element_string_multiline_create(
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Create string element */
WidgetElement* widget_element_string_create(
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text);

/** Create text box element */
WidgetElement* widget_element_text_box_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots);

/** Create button element */
WidgetElement* widget_element_button_create(
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context);

/** Create icon element */
WidgetElement* widget_element_icon_create(uint8_t x, uint8_t y, const Icon* icon);

/** Create frame element */
WidgetElement* widget_element_frame_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius);

WidgetElement* widget_element_text_scroll_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/modules/widget_elements/widget_element_i.h ===

=== BEGIN: applications/services/gui/scene_manager.h ===
/**
 * @file scene_manager.h
 * GUI: SceneManager API
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Scene Manager events type */
typedef enum {
    SceneManagerEventTypeCustom,
    SceneManagerEventTypeBack,
    SceneManagerEventTypeTick,
} SceneManagerEventType;

/** Scene Manager event
 */
typedef struct {
    SceneManagerEventType type;
    uint32_t event;
} SceneManagerEvent;

/** Prototype for Scene on_enter handler */
typedef void (*AppSceneOnEnterCallback)(void* context);

/** Prototype for Scene on_event handler */
typedef bool (*AppSceneOnEventCallback)(void* context, SceneManagerEvent event);

/** Prototype for Scene on_exit handler */
typedef void (*AppSceneOnExitCallback)(void* context);

/** Scene Manager configuration structure
 * Contains array of Scene handlers
 */
typedef struct {
    const AppSceneOnEnterCallback* on_enter_handlers;
    const AppSceneOnEventCallback* on_event_handlers;
    const AppSceneOnExitCallback* on_exit_handlers;
    const uint32_t scene_num;
} SceneManagerHandlers;

typedef struct SceneManager SceneManager;

/** Set Scene state
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 * @param      state          Scene new state
 */
void scene_manager_set_scene_state(SceneManager* scene_manager, uint32_t scene_id, uint32_t state);

/** Get Scene state
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     Scene state
 */
uint32_t scene_manager_get_scene_state(const SceneManager* scene_manager, uint32_t scene_id);

/** Scene Manager allocation and configuration
 *
 * Scene Manager allocates all scenes internally
 *
 * @param      app_scene_handlers  SceneManagerHandlers instance
 * @param      context             context to be set on Scene handlers calls
 *
 * @return     SceneManager instance
 */
SceneManager* scene_manager_alloc(const SceneManagerHandlers* app_scene_handlers, void* context);

/** Free Scene Manager with allocated Scenes
 *
 * @param      scene_manager  SceneManager instance
 */
void scene_manager_free(SceneManager* scene_manager);

/** Custom event handler
 *
 * Calls Scene event handler with Custom event parameter
 *
 * @param      scene_manager  SceneManager instance
 * @param      custom_event   Custom event code
 *
 * @return     true if event was consumed, false otherwise
 */
bool scene_manager_handle_custom_event(SceneManager* scene_manager, uint32_t custom_event);

/** Back event handler
 *
 * Calls Scene event handler with Back event parameter
 *
 * @param      scene_manager  SceneManager instance
 *
 * @return     true if event was consumed, false otherwise
 */
bool scene_manager_handle_back_event(SceneManager* scene_manager);

/** Tick event handler
 *
 * Calls Scene event handler with Tick event parameter
 *
 * @param      scene_manager  SceneManager instance
 */
void scene_manager_handle_tick_event(SceneManager* scene_manager);

/** Add and run next Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      next_scene_id  next Scene ID
 */
void scene_manager_next_scene(SceneManager* scene_manager, uint32_t next_scene_id);

/** Run previous Scene
 *
 * @param      scene_manager  SceneManager instance
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_previous_scene(SceneManager* scene_manager);

/** Search previous Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_has_previous_scene(const SceneManager* scene_manager, uint32_t scene_id);

/** Search and switch to previous Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_search_and_switch_to_previous_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);

/** Search and switch to previous Scene, multiple choice
 *
 * @param      scene_manager    SceneManager instance
 * @param      scene_ids        Array of scene IDs
 * @param      scene_ids_size   Array of scene IDs size
 *
 * @return     true if one of previous scenes was found, false otherwise
 */
bool scene_manager_search_and_switch_to_previous_scene_one_of(
    SceneManager* scene_manager,
    const uint32_t* scene_ids,
    size_t scene_ids_size);

/** Clear Scene stack and switch to another Scene
 *
 * @param      scene_manager  SceneManager instance
 * @param      scene_id       Scene ID
 *
 * @return     true if previous scene was found, false otherwise
 */
bool scene_manager_search_and_switch_to_another_scene(
    SceneManager* scene_manager,
    uint32_t scene_id);

/** Exit from current scene
 *
 * @param      scene_manager  SceneManager instance
 */
void scene_manager_stop(SceneManager* scene_manager);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/scene_manager.h ===

=== BEGIN: applications/services/gui/scene_manager_i.h ===
/**
 * @file scene_manager_i.h
 * GUI: internal SceneManager API
 */

#pragma once

#include "scene_manager.h"
#include <m-array.h>

ARRAY_DEF(SceneManagerIdStack, uint32_t, M_DEFAULT_OPLIST);

typedef struct {
    uint32_t state;
} AppScene;

struct SceneManager {
    SceneManagerIdStack_t scene_id_stack;
    const SceneManagerHandlers* scene_handlers;
    void* context;
    AppScene scene[];
};

=== END: applications/services/gui/scene_manager_i.h ===

=== BEGIN: applications/services/gui/view.h ===
/**
 * @file view.h
 * GUI: View API
 */

#pragma once

#include <input/input.h>

#include "icon_animation.h"
#include "canvas.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Hides drawing view_port */
#define VIEW_NONE 0xFFFFFFFF

/** Ignore navigation event */
#define VIEW_IGNORE 0xFFFFFFFE

typedef enum {
    ViewOrientationHorizontal,
    ViewOrientationHorizontalFlip,
    ViewOrientationVertical,
    ViewOrientationVerticalFlip,
} ViewOrientation;

/** View, anonymous type */
typedef struct View View;

/** View Draw callback
 * @param      canvas      pointer to canvas
 * @param      model       pointer to model
 * @warning    called from GUI thread
 */
typedef void (*ViewDrawCallback)(Canvas* canvas, void* model);

/** View Input callback
 * @param      event    pointer to input event data
 * @param      context  pointer to context
 * @return     true if event handled, false if event ignored
 * @warning    called from GUI thread
 */
typedef bool (*ViewInputCallback)(InputEvent* event, void* context);

/** View Custom callback
 * @param      event    number of custom event
 * @param      context  pointer to context
 * @return     true if event handled, false if event ignored
 */
typedef bool (*ViewCustomCallback)(uint32_t event, void* context);

/** View navigation callback
 * @param      context  pointer to context
 * @return     next view id
 * @warning    called from GUI thread
 */
typedef uint32_t (*ViewNavigationCallback)(void* context);

/** View callback
 * @param      context  pointer to context
 * @warning    called from GUI thread
 */
typedef void (*ViewCallback)(void* context);

/** View Update Callback Called upon model change, need to be propagated to GUI
 * throw ViewPort update
 * @param      view     pointer to view
 * @param      context  pointer to context
 * @warning    called from GUI thread
 */
typedef void (*ViewUpdateCallback)(View* view, void* context);

/** View model types */
typedef enum {
    /** Model is not allocated */
    ViewModelTypeNone,
    /** Model consist of atomic types and/or partial update is not critical for rendering.
     * Lock free.
     */
    ViewModelTypeLockFree,
    /** Model access is guarded with mutex.
     * Locking gui thread.
     */
    ViewModelTypeLocking,
} ViewModelType;

/** Allocate and init View
 * @return View instance
 */
View* view_alloc(void);

/** Free View
 *
 * @param      view  instance
 */
void view_free(View* view);

/** Tie IconAnimation with View
 *
 * @param      view            View instance
 * @param      icon_animation  IconAnimation instance
 */
void view_tie_icon_animation(View* view, IconAnimation* icon_animation);

/** Set View Draw callback
 *
 * @param      view      View instance
 * @param      callback  draw callback
 */
void view_set_draw_callback(View* view, ViewDrawCallback callback);

/** Set View Input callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_input_callback(View* view, ViewInputCallback callback);

/** Set View Custom callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_custom_callback(View* view, ViewCustomCallback callback);

/** Set Navigation Previous callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_previous_callback(View* view, ViewNavigationCallback callback);

/** Set Enter callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_enter_callback(View* view, ViewCallback callback);

/** Set Exit callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_exit_callback(View* view, ViewCallback callback);

/** Set Update callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_update_callback(View* view, ViewUpdateCallback callback);

/** Set View Draw callback
 *
 * @param      view     View instance
 * @param      context  context for callbacks
 */
void view_set_update_callback_context(View* view, void* context);

/** Set View Draw callback
 *
 * @param      view     View instance
 * @param      context  context for callbacks
 */
void view_set_context(View* view, void* context);

/** Set View Orientation
 *
 * @param      view         View instance
 * @param      orientation  either vertical or horizontal
 */
void view_set_orientation(View* view, ViewOrientation orientation);

/** Allocate view model.
 *
 * @param      view  View instance
 * @param      type  View Model Type
 * @param      size  size
 */
void view_allocate_model(View* view, ViewModelType type, size_t size);

/** Free view model data memory.
 *
 * @param      view  View instance
 */
void view_free_model(View* view);

/** Get view model data
 *
 * @param      view  View instance
 *
 * @return     pointer to model data
 * @warning    Don't forget to commit model changes
 */
void* view_get_model(View* view);

/** Commit view model
 *
 * @param      view    View instance
 * @param      update  true if you want to emit view update, false otherwise
 */
void view_commit_model(View* view, bool update);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define with_view_model_cpp(view, type, var, code, update)  \
    {                                                       \
        type var = static_cast<type>(view_get_model(view)); \
        {code};                                             \
        view_commit_model(view, update);                    \
    }
#else
/** With clause for view model
 *
 * @param      view           View instance pointer
 * @param      type           View model type
 * @param      code           Code block that will be executed between model lock and unlock
 * @param      update         Bool flag, if true, view will be updated after code block. Can be variable, so code block can decide if update is needed.
 *
 */
#define with_view_model(view, type, code, update) \
    {                                             \
        type = view_get_model(view);              \
        {code};                                   \
        view_commit_model(view, update);          \
    }
#endif

=== END: applications/services/gui/view.h ===

=== BEGIN: applications/services/gui/view_dispatcher.h ===
/**
 * @file view_dispatcher.h
 * @brief GUI: ViewDispatcher API
 *
 * ViewDispatcher is used to connect several Views to a Gui instance, switch between them and handle various events.
 * This is useful in applications featuring an advanced graphical user interface.
 *
 * Internally, ViewDispatcher employs a FuriEventLoop instance together with two separate
 * message queues for input and custom event handling. See FuriEventLoop for more information.
 *
 * If no multi-view or complex event handling capabilities are required, consider using ViewHolder instead.
 *
 * @warning Views added to a ViewDispatcher MUST NOT be in a ViewStack at the same time.
 */

#pragma once

#include "view.h"
#include "gui.h"
#include "scene_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ViewDispatcher view_port placement */
typedef enum {
    ViewDispatcherTypeDesktop, /**< Desktop layer: fullscreen with status bar on top of it. For internal usage. */
    ViewDispatcherTypeWindow, /**< Window layer: with status bar  */
    ViewDispatcherTypeFullscreen /**< Fullscreen layer: without status bar */
} ViewDispatcherType;

typedef struct ViewDispatcher ViewDispatcher;

/** Prototype for custom event callback */
typedef bool (*ViewDispatcherCustomEventCallback)(void* context, uint32_t event);

/** Prototype for navigation event callback */
typedef bool (*ViewDispatcherNavigationEventCallback)(void* context);

/** Prototype for tick event callback */
typedef void (*ViewDispatcherTickEventCallback)(void* context);

/** Allocate ViewDispatcher instance
 *
 * @return     pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc(void);

/** Allocate ViewDispatcher instance with an externally owned event loop. If
 * this constructor is used instead of `view_dispatcher_alloc`, the burden of
 * freeing the event loop is placed on the caller.
 *
 * @param loop pointer to FuriEventLoop instance
 * @return     pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc_ex(FuriEventLoop* loop);

/** Free ViewDispatcher instance
 *
 * @warning All added views MUST be removed using view_dispatcher_remove_view()
 *          before calling this function.
 *
 * @param      view_dispatcher  pointer to ViewDispatcher
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/** Enable queue support
 *
 * @deprecated Do NOT use in new code and remove all calls to it from existing code.
 *             The queue support is now always enabled during construction. If no queue support
 *             is required, consider using ViewHolder instead.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
FURI_DEPRECATED void view_dispatcher_enable_queue(ViewDispatcher* view_dispatcher);

/** Send custom event
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param[in]  event            The event
 */
void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);

/** Set custom event handler
 *
 * Called on Custom Event, if it is not consumed by view
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherCustomEventCallback instance
 */
void view_dispatcher_set_custom_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherCustomEventCallback callback);

/** Set navigation event handler
 *
 * Called on Input Short Back Event, if it is not consumed by view
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherNavigationEventCallback instance
 */
void view_dispatcher_set_navigation_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherNavigationEventCallback callback);

/** Set tick event handler
 *
 * @warning Requires the event loop to be owned by the view dispatcher, i.e.
 * it should have been instantiated with `view_dispatcher_alloc`, not
 * `view_dispatcher_alloc_ex`.
 * 
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherTickEventCallback
 * @param      tick_period      callback call period
 */
void view_dispatcher_set_tick_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherTickEventCallback callback,
    uint32_t tick_period);

/** Set event callback context
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      context          pointer to context
 */
void view_dispatcher_set_event_callback_context(ViewDispatcher* view_dispatcher, void* context);

/** Get event_loop instance
 *
 * Use the return value to connect additional supported primitives (message queues, timers, etc)
 * to this ViewDispatcher instance's event loop.
 *
 * @warning Do NOT call furi_event_loop_run() on the returned instance, it is done internally
 *          in the view_dispatcher_run() call.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 *
 * @return     The event_loop instance.
 */
FuriEventLoop* view_dispatcher_get_event_loop(ViewDispatcher* view_dispatcher);

/** Run ViewDispatcher
 *
 * This function will start the event loop and block until view_dispatcher_stop() is called
 * or the current thread receives a FuriSignalExit signal.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_run(ViewDispatcher* view_dispatcher);

/** Stop ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_stop(ViewDispatcher* view_dispatcher);

/** Add view to ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to register
 * @param      view             View instance
 */
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);

/** Remove view from ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to remove
 */
void view_dispatcher_remove_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Switch to View
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to register
 * @warning    switching may be delayed till input events complementarity
 *             reached
 */
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Send ViewPort of this ViewDispatcher instance to front
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_send_to_front(ViewDispatcher* view_dispatcher);

/** Send ViewPort of this ViewDispatcher instance to back
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_send_to_back(ViewDispatcher* view_dispatcher);

/** Attach ViewDispatcher to GUI
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      gui              GUI instance to attach to
 * @param[in]  type             The type
 */
void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/view_dispatcher.h ===

=== BEGIN: applications/services/gui/view_dispatcher_i.h ===
/**
 * @file view_dispatcher_i.h
 * GUI: ViewDispatcher API
 */

#pragma once

#include <m-dict.h>

#include "view_dispatcher.h"
#include "view_i.h"
#include "gui_i.h"

DICT_DEF2(ViewDict, uint32_t, M_DEFAULT_OPLIST, View*, M_PTR_OPLIST) // NOLINT

struct ViewDispatcher {
    bool is_event_loop_owned;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;

    Gui* gui;
    ViewPort* view_port;
    ViewDict_t views;

    View* current_view;

    View* ongoing_input_view;
    uint8_t ongoing_input;

    ViewDispatcherCustomEventCallback custom_event_callback;
    ViewDispatcherNavigationEventCallback navigation_event_callback;
    ViewDispatcherTickEventCallback tick_event_callback;
    uint32_t tick_period;
    void* event_context;
};

/** ViewPort Draw Callback */
void view_dispatcher_draw_callback(Canvas* canvas, void* context);

/** ViewPort Input Callback */
void view_dispatcher_input_callback(InputEvent* event, void* context);

/** Input handler */
void view_dispatcher_handle_input(ViewDispatcher* view_dispatcher, InputEvent* event);

/** Tick handler */
void view_dispatcher_handle_tick_event(void* context);

/** Custom event handler */
void view_dispatcher_handle_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);

/** Set current view, dispatches view enter and exit */
void view_dispatcher_set_current_view(ViewDispatcher* view_dispatcher, View* view);

/** ViewDispatcher update event */
void view_dispatcher_update(View* view, void* context);

/** ViewDispatcher run event loop event callback */
void view_dispatcher_run_event_callback(FuriEventLoopObject* object, void* context);

/** ViewDispatcher run event loop input callback */
void view_dispatcher_run_input_callback(FuriEventLoopObject* object, void* context);

=== END: applications/services/gui/view_dispatcher_i.h ===

=== BEGIN: applications/services/gui/view_holder.h ===
/**
 * @file view_holder.h
 * @brief GUI: ViewHolder API
 *
 * ViewHolder is used to connect a single View to a Gui instance. This is useful in smaller applications
 * with a simple user interface. If advanced view switching capabilites are required, consider using ViewDispatcher instead.
 *
 * @warning Views added to a ViewHolder MUST NOT be in a ViewStack at the same time.
 */
#pragma once

#include <gui/view.h>
#include <gui/gui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ViewHolder ViewHolder;

/** 
 * @brief Free callback type
 */
typedef void (*FreeCallback)(void* free_context);

/** 
 * @brief Back callback type
 *
 * @warning Will be called from the GUI thread
 */
typedef void (*BackCallback)(void* back_context);

/**
 * @brief Allocate ViewHolder
 * @return pointer to ViewHolder instance
 */
ViewHolder* view_holder_alloc(void);

/**
 * @brief Free ViewHolder and call Free callback
 *
 * @warning The current view must be unset prior to freeing a ViewHolder instance.
 *
 * @param view_holder pointer to ViewHolder
 */
void view_holder_free(ViewHolder* view_holder);

/**
 * @brief Set view for ViewHolder
 *
 * Pass NULL as the view parameter to unset the current view.
 * 
 * @param view_holder ViewHolder instance
 * @param view View instance
 */
void view_holder_set_view(ViewHolder* view_holder, View* view);

/**
 * @brief Set Free callback
 * 
 * @param view_holder ViewHolder instance
 * @param free_callback callback pointer
 * @param free_context callback context
 */
void view_holder_set_free_callback(
    ViewHolder* view_holder,
    FreeCallback free_callback,
    void* free_context);

/**
 * @brief Free callback context getter.
 *
 * Useful if your Free callback is a module destructor, so you can get an instance of the module using this method.
 * 
 * @param view_holder ViewHolder instance
 * @return void* free callback context
 */
void* view_holder_get_free_context(ViewHolder* view_holder);

/**
 * @brief Set the back key callback.
 *
 * The callback function will be called if the user has pressed the Back key
 * and the current view did not handle this event.
 *
 * @param view_holder ViewHolder instance
 * @param back_callback pointer to the callback function
 * @param back_context pointer to a user-specific object, can be NULL
 */
void view_holder_set_back_callback(
    ViewHolder* view_holder,
    BackCallback back_callback,
    void* back_context);

/**
 * @brief Attach ViewHolder to GUI
 * 
 * @param view_holder ViewHolder instance
 * @param gui GUI instance to attach to
 */
void view_holder_attach_to_gui(ViewHolder* view_holder, Gui* gui);

/**
 * @brief View Update Handler
 *
 * @param view View Instance
 * @param context ViewHolder instance
 */
void view_holder_update(View* view, void* context);

/**
 * @brief Send ViewPort of this ViewHolder instance to front
 *
 * @param view_holder ViewHolder instance
 */
void view_holder_send_to_front(ViewHolder* view_holder);

/**
 * @brief Send ViewPort of this ViewHolder instance to back
 *
 * @param view_holder ViewHolder instance
 */
void view_holder_send_to_back(ViewHolder* view_holder);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/view_holder.h ===

=== BEGIN: applications/services/gui/view_i.h ===
/**
 * @file view_i.h
 * GUI: internal View API
 */

#pragma once

#include "view.h"
#include <furi.h>

typedef struct {
    FuriMutex* mutex;
    uint8_t data[];
} ViewModelLocking;

struct View {
    ViewDrawCallback draw_callback;
    ViewInputCallback input_callback;
    ViewCustomCallback custom_callback;

    ViewModelType model_type;
    ViewNavigationCallback previous_callback;
    ViewCallback enter_callback;
    ViewCallback exit_callback;
    ViewOrientation orientation;

    ViewUpdateCallback update_callback;
    void* update_callback_context;

    void* model;
    void* context;
};

/** IconAnimation tie callback */
void view_icon_animation_callback(IconAnimation* instance, void* context);

/** Unlock model */
void view_unlock_model(View* view);

/** Draw Callback for View dispatcher */
void view_draw(View* view, Canvas* canvas);

/** Input Callback for View dispatcher */
bool view_input(View* view, InputEvent* event);

/** Custom Callback for View dispatcher */
bool view_custom(View* view, uint32_t event);

/** Previous Callback for View dispatcher */
uint32_t view_previous(View* view);

/** Enter Callback for View dispatcher */
void view_enter(View* view);

/** Exit Callback for View dispatcher */
void view_exit(View* view);

=== END: applications/services/gui/view_i.h ===

=== BEGIN: applications/services/gui/view_port.h ===
/**
 * @file view_port.h
 * GUI: ViewPort API
 */

#pragma once

#include <input/input.h>
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ViewPort ViewPort;

typedef enum {
    ViewPortOrientationHorizontal,
    ViewPortOrientationHorizontalFlip,
    ViewPortOrientationVertical,
    ViewPortOrientationVerticalFlip,
    ViewPortOrientationMAX, /**< Special value, don't use it */
} ViewPortOrientation;

/** ViewPort Draw callback
 * @warning    called from GUI thread
 */
typedef void (*ViewPortDrawCallback)(Canvas* canvas, void* context);

/** ViewPort Input callback
 * @warning    called from GUI thread
 */
typedef void (*ViewPortInputCallback)(InputEvent* event, void* context);

/** ViewPort allocator
 *
 * always returns view_port or stops system if not enough memory.
 *
 * @return     ViewPort instance
 */
ViewPort* view_port_alloc(void);

/** ViewPort deallocator
 *
 * Ensure that view_port was unregistered in GUI system before use.
 *
 * @param      view_port  ViewPort instance
 */
void view_port_free(ViewPort* view_port);

/** Set view_port width.
 *
 * Will be used to limit canvas drawing area and autolayout feature.
 *
 * @param      view_port  ViewPort instance
 * @param      width      wanted width, 0 - auto.
 */
void view_port_set_width(ViewPort* view_port, uint8_t width);
uint8_t view_port_get_width(const ViewPort* view_port);

/** Set view_port height.
 *
 * Will be used to limit canvas drawing area and autolayout feature.
 *
 * @param      view_port  ViewPort instance
 * @param      height     wanted height, 0 - auto.
 */
void view_port_set_height(ViewPort* view_port, uint8_t height);
uint8_t view_port_get_height(const ViewPort* view_port);

/** Enable or disable view_port rendering.
 *
 * @param      view_port  ViewPort instance
 * @param      enabled    Indicates if enabled
 * @warning    automatically dispatches update event
 */
void view_port_enabled_set(ViewPort* view_port, bool enabled);
bool view_port_is_enabled(const ViewPort* view_port);

/** ViewPort event callbacks
 *
 * @param      view_port  ViewPort instance
 * @param      callback   appropriate callback function
 * @param      context    context to pass to callback
 */
void view_port_draw_callback_set(ViewPort* view_port, ViewPortDrawCallback callback, void* context);
void view_port_input_callback_set(
    ViewPort* view_port,
    ViewPortInputCallback callback,
    void* context);

/** Emit update signal to GUI system.
 *
 * Rendering will happen later after GUI system process signal.
 *
 * @param      view_port  ViewPort instance
 */
void view_port_update(ViewPort* view_port);

/** Set ViewPort orientation.
 *
 * @param      view_port    ViewPort instance
 * @param      orientation  display orientation, horizontal or vertical.
 */
void view_port_set_orientation(ViewPort* view_port, ViewPortOrientation orientation);
ViewPortOrientation view_port_get_orientation(const ViewPort* view_port);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/view_port.h ===

=== BEGIN: applications/services/gui/view_port_i.h ===
/**
 * @file view_port_i.h
 * GUI: internal ViewPort API
 */

#pragma once

#include "gui_i.h"
#include "view_port.h"

struct ViewPort {
    Gui* gui;
    FuriMutex* mutex;
    bool is_enabled;
    ViewPortOrientation orientation;

    uint8_t width;
    uint8_t height;

    ViewPortDrawCallback draw_callback;
    void* draw_callback_context;

    ViewPortInputCallback input_callback;
    void* input_callback_context;
};

/** Set GUI reference.
 *
 * To be used by GUI, called upon view_port tree insert
 *
 * @param      view_port  ViewPort instance
 * @param      gui        gui instance pointer
 */
void view_port_gui_set(ViewPort* view_port, Gui* gui);

/** Process draw call. Calls draw callback.
 *
 * To be used by GUI, called on tree redraw.
 *
 * @param      view_port  ViewPort instance
 * @param      canvas     canvas to draw at
 */
void view_port_draw(ViewPort* view_port, Canvas* canvas);

/** Process input. Calls input callback.
 *
 * To be used by GUI, called on input dispatch.
 *
 * @param      view_port  ViewPort instance
 * @param      event      pointer to input event
 */
void view_port_input(ViewPort* view_port, InputEvent* event);

=== END: applications/services/gui/view_port_i.h ===

=== BEGIN: applications/services/gui/view_stack.h ===
/**
 * @file view_stack.h
 * @brief GUI: ViewStack API
 *
 * ViewStack accumulates several Views in one stack.
 * Draw callbacks are called sequenctially starting from
 * first added. Input callbacks are called in reverse order.
 * Consumed input is not passed on underlying layers.
 *
 * @warning Views added to a ViewStack MUST NOT be in a ViewDispatcher or a ViewHolder at the same time.
 */

#pragma once

#include <stdbool.h>
#include "view.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ViewStack, anonymous type. */
typedef struct ViewStack ViewStack;

/** Allocate and init ViewStack
 *
 * @return      ViewStack instance
 */
ViewStack* view_stack_alloc(void);

/** Free ViewStack instance
 *
 * @param       view_stack  instance
 */
void view_stack_free(ViewStack* view_stack);

/** Get View of ViewStack.
 * Should this View to any view manager such as
 * ViewDispatcher or ViewHolder.
 *
 * @param       view_stack  instance
 */
View* view_stack_get_view(ViewStack* view_stack);

/** Add View to ViewStack.
 * Adds View on top of ViewStack.
 *
 * @param       view_stack  instance
 * @param       view        view to add
 */
void view_stack_add_view(ViewStack* view_stack, View* view);

/** Remove any View in ViewStack.
 * If no View to remove found - ignore.
 *
 * @param       view_stack  instance
 * @param       view        view to remove
 */
void view_stack_remove_view(ViewStack* view_stack, View* view);

#ifdef __cplusplus
}
#endif

=== END: applications/services/gui/view_stack.h ===


