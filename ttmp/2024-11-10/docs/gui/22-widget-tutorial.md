# Widget System Tutorial

## Part 1: Understanding Widgets in Real Applications

### 1. Introduction to Widgets

The Widget system is one of the fundamental building blocks for creating user interfaces in Flipper Zero applications. Before diving into creating our own widget-based application, let's examine how widgets are used in real applications.

### 2. Case Study: File Browser Test Application

The File Browser Test application demonstrates how widgets integrate with other GUI components in a full application:

```c
// Application structure
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;                 // Widget instance
    FileBrowser* file_browser;      // Specialized browser component
    // ... other components
} FileBrowserApp;
```

Key points from the implementation:

1. **Widget Lifecycle**:
   ```c
   // Allocation
   app->widget = widget_alloc();
   
   // View Integration
   view_dispatcher_add_view(
       app->view_dispatcher, 
       FileBrowserAppViewStart, 
       widget_get_view(app->widget));
   
   // Cleanup
   widget_free(app->widget);
   ```

2. **View Management**:
   - Multiple views can use the same widget
   - Views are switched using the view dispatcher
   - Widgets integrate seamlessly with other view types

### 2.3 Widget Usage in File Browser Test

The File Browser Test application demonstrates three key widget use cases:

1. **Start Screen Widget**:
   ```c
   // Display welcome message and button
   widget_add_string_multiline_element(
       app->widget, 64, 20, AlignCenter, AlignTop, 
       FontSecondary, "Press OK to start");
   widget_add_button_element(
       app->widget, GuiButtonTypeCenter, "Ok", 
       file_browser_scene_start_ok_callback, app);
   ```
   - Shows centered welcome text
   - Adds interactive OK button
   - Handles button press to start browsing

2. **Result Screen Widget**:
   ```c
   // Display selected file path
   widget_add_string_multiline_element(
       app->widget, 64, 10, AlignCenter, AlignTop,
       FontSecondary, furi_string_get_cstr(app->file_path));
   ```
   - Shows selected file path
   - Centered text alignment
   - Multi-line support for long paths

3. **Memory Management**:
   ```c
   // Reset widget between scenes
   widget_reset(app->widget);
   ```
   - Cleans up widget elements between scenes
   - Prevents memory leaks
   - Ensures clean state for next scene

### 3. Case Study: SubGHz Test Application

The SubGHz Test application shows how widgets can be used alongside other UI components:

```c
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;         // For static content
    Submenu* submenu;      // For menus
    Popup* popup;          // For notifications
    // ... other components
} SubGhzTestApp;
```

Notable patterns:

1. **Component Integration**:
   - Widgets used for static content display
   - Combined with specialized components (Submenu, Popup)
   - Each component serves a specific UI purpose

2. **View Switching**:
   ```c
   view_dispatcher_add_view(
       app->view_dispatcher, 
       SubGhzTestViewWidget, 
       widget_get_view(app->widget));
   ```

### 2.4 Widget Usage in SubGHz Test

The SubGHz Test application shows more advanced widget usage:

1. **About Screen Widget**:
   ```c
   // Title box with styling
   widget_add_text_box_element(
       app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom,
       "\e#\e!         Sub-Ghz Test          \e!\n",
       false);
   
   // Scrollable content
   widget_add_text_scroll_element(
       app->widget, 0, 16, 128, 50, 
       furi_string_get_cstr(temp_str));
   ```
   - Uses styled text box for title
   - Implements scrollable text area
   - Combines multiple widget elements
   - Handles rich text formatting

2. **Information Display**:
   - Version information
   - Developer credits
   - GitHub repository link
   - Technical description
   - All formatted with proper alignment and styling

3. **Widget Integration**:
   - Seamless integration with other views (Submenu, Popup)
   - Scene-based navigation
   - Proper cleanup on exit

These real-world examples demonstrate:
- Complex layout composition
- Multiple element types
- Event handling
- Memory management
- Scene integration

### 4. Case Study: BLE Beacon Example

The BLE Beacon example demonstrates how to use widgets in a more complex application with dynamic content updates:

```c
// Dynamic Status Updates
static void update_status_text(BleBeaconApp* ble_beacon) {
    DialogEx* dialog_ex = ble_beacon->dialog_ex;

    // Add header with centered text
    dialog_ex_set_header(dialog_ex, "BLE Beacon Demo", 64, 0, AlignCenter, AlignTop);

    // Build dynamic status string
    FuriString* status = ble_beacon->status_string;
    furi_string_reset(status);
    furi_string_cat_str(status, "Status: ");
    furi_string_cat_str(status, ble_beacon->is_beacon_active ? "Running\n" : "Stopped\n");

    // Add MAC address and data length info
    dialog_ex_set_text(dialog_ex, furi_string_get_cstr(status), 0, 29, AlignLeft, AlignCenter);

    // Add icon and interactive buttons
    dialog_ex_set_icon(dialog_ex, 93, 20, &I_lighthouse_35x44);
    dialog_ex_set_left_button_text(dialog_ex, "Config");
    dialog_ex_set_center_button_text(dialog_ex, ble_beacon->is_beacon_active ? "Stop" : "Start");
}
```

Key patterns demonstrated:
1. Dynamic text updates based on application state
2. Mixing text, icons, and buttons in the same view
3. Alignment control for different UI elements
4. Interactive button state management

### 5. Widget Best Practices

Based on the examples we've seen, here are some best practices for working with widgets:

1. **Clean Widget State**:
   ```c
   // Always reset widget between scenes
   void scene_exit(void* context) {
       MyApp* app = context;
       widget_reset(app->widget);
   }
   ```

2. **Consistent Layout**:
   ```c
   // Center important messages
   widget_add_string_multiline_element(
       app->widget, 64, 20, AlignCenter, AlignTop, 
       FontSecondary, "Press OK to start");

   // Left-align detailed information
   widget_add_string_element(
       app->widget, 0, 29, AlignLeft, AlignCenter,
       "Status: Active");
   ```

3. **Interactive Elements**:
   ```c
   // Add buttons with callbacks
   widget_add_button_element(
       app->widget,
       GuiButtonTypeCenter,
       "Ok",
       button_callback,
       context);
   ```

4. **Resource Management**:
   - Always pair `widget_alloc()` with `widget_free()`
   - Reset widgets between scenes
   - Use string buffers for dynamic content

### 6. Common Widget Patterns

Here are some common patterns seen across applications:

1. **Status Display**:
   ```c
   // Add multiple lines of status info
   widget_add_string_element(
       widget, 64, 10, AlignCenter, AlignTop, "Status");
   widget_add_string_multiline_element(
       widget, 64, 30, AlignCenter, AlignTop, status_text);
   ```

2. **Button Row**:
   ```c
   // Add multiple interactive buttons
   widget_add_button_element(
       widget, GuiButtonTypeLeft, "Back", back_cb, context);
   widget_add_button_element(
       widget, GuiButtonTypeRight, "Next", next_cb, context);
   ```

3. **Mixed Content**:
   ```c
   // Combine text, buttons, and frame
   widget_add_frame_element(widget, 0, 0, 128, 64, 0);
   widget_add_string_element(
       widget, 64, 10, AlignCenter, AlignTop, "Title");
   widget_add_button_element(
       widget, GuiButtonTypeCenter, "Action", action_cb, context);
   ```

## Part 2: Building Your First Widget Application

### 1. Basic Concepts

Let's build a simple "Hello Widget" application that introduces core concepts.

#### Exercise 1.1: Application Structure
Create the basic application structure with a widget.

```c
// hello_widget.h
#pragma once
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;  // Main widget instance
} HelloWidgetApp;

typedef enum {
    HelloWidgetViewWidget,  // View ID for our widget
    // Add more views here as needed
} HelloWidgetView;
```

```c
// hello_widget.c
#include "hello_widget.h"

// Allocate application
HelloWidgetApp* hello_widget_app_alloc() {
    HelloWidgetApp* app = malloc(sizeof(HelloWidgetApp));
    
    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    
    // Create view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    
    // Create widget
    app->widget = widget_alloc();
    
    // Add widget to view dispatcher
    view_dispatcher_add_view(
        app->view_dispatcher,
        HelloWidgetViewWidget,
        widget_get_view(app->widget));
    
    return app;
}

// Free application
void hello_widget_app_free(HelloWidgetApp* app) {
    // Remove and free views
    view_dispatcher_remove_view(app->view_dispatcher, HelloWidgetViewWidget);
    widget_free(app->widget);
    
    // Free view dispatcher
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_GUI);
    
    free(app);
}
```

**Key APIs Used:**
- `widget_alloc()`: Creates a new widget instance
- `widget_get_view()`: Gets the view for view dispatcher
- `view_dispatcher_add_view()`: Registers widget view

#### Exercise 1.2: Widget Setup
Add content and show the widget.

```c
// Initialize widget content
void hello_widget_app_init(HelloWidgetApp* app) {
    // Add a frame around the screen
    widget_add_frame_element(app->widget, 0, 0, 128, 64, 0);
    
    // Add centered title text
    widget_add_string_element(
        app->widget,
        64, 10,         // x, y coordinates
        AlignCenter,    // horizontal alignment
        AlignTop,       // vertical alignment
        FontPrimary,    // font choice
        "Hello Widget!"
    );
    
    // Add description text
    widget_add_string_multiline_element(
        app->widget,
        64, 32,         // x, y coordinates
        AlignCenter,    // horizontal alignment
        AlignCenter,    // vertical alignment
        FontSecondary,  // font choice
        "This is my first\nwidget application!"
    );
    
    // Add button with callback
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,  // button position
        "OK",                 // button text
        hello_widget_ok_callback,
        app
    );
}

// Button callback
static void hello_widget_ok_callback(GuiButtonType result, InputType type, void* context) {
    HelloWidgetApp* app = context;
    if(type == InputTypeShort) {
        // Handle button press
        view_dispatcher_send_custom_event(app->view_dispatcher, 0);
    }
}
```

**Key Widget Element APIs:**
- Text Elements:
  ```c
  widget_add_string_element(
      Widget* widget,
      uint8_t x,
      uint8_t y,
      Align horizontal,
      Align vertical,
      Font font,
      const char* text);
  
  widget_add_string_multiline_element(
      Widget* widget,
      uint8_t x,
      uint8_t y,
      Align horizontal,
      Align vertical,
      Font font,
      const char* text);
  ```

- Frame Element:
  ```c
  widget_add_frame_element(
      Widget* widget,
      uint8_t x,
      uint8_t y,
      uint8_t width,
      uint8_t height,
      uint8_t radius);
  ```

- Button Element:
  ```c
  widget_add_button_element(
      Widget* widget,
      GuiButtonType button_type,  // Left/Right/Center
      const char* text,
      ButtonCallback callback,
      void* context);
  ```

### 2. Adding Dynamic Content

#### Exercise 2.1: State Management
Create a widget that updates its content based on state.

```c
typedef struct {
    HelloWidgetApp* app;
    uint32_t counter;
    FuriString* display_text;
} HelloWidgetState;

void hello_widget_update_display(HelloWidgetState* state) {
    // Clear previous elements
    widget_reset(state->app->widget);
    
    // Format display text
    furi_string_printf(
        state->display_text,
        "Counter: %lu",
        state->counter);
    
    // Add updated elements
    widget_add_string_element(
        state->app->widget,
        64, 10,
        AlignCenter,
        AlignTop,
        FontPrimary,
        "Dynamic Widget");
        
    widget_add_string_element(
        state->app->widget,
        64, 32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(state->display_text));
        
    // Add increment/decrement buttons
    widget_add_button_element(
        state->app->widget,
        GuiButtonTypeLeft,
        "-",
        decrement_callback,
        state);
        
    widget_add_button_element(
        state->app->widget,
        GuiButtonTypeRight,
        "+",
        increment_callback,
        state);
}
```

**Important Concepts:**
- Use `widget_reset()` before updating elements
- Manage state in a separate structure
- Use `FuriString` for dynamic text
- Update display when state changes

### 3. Input Handling

#### Exercise 3.1: Button Elements
Add an interactive button to your widget.

```c
// Button callback
static void hello_widget_ok_callback(GuiButtonType result, InputType type, void* context) {
    HelloWidgetApp* app = context;
    if(type == InputTypeShort) {
        // Handle button press
        view_dispatcher_send_custom_event(app->view_dispatcher, 0);
    }
}
```

**Key APIs Used:**
- `widget_add_button_element()`: Adds a button to the widget
- `ButtonCallback`: Callback function for button press

#### Exercise 3.2: Complex Interactions
Create a counter with increment/decrement buttons.

```c
typedef struct {
    HelloWidgetApp* app;
    uint32_t counter;
    FuriString* display_text;
} HelloWidgetState;

void hello_widget_update_display(HelloWidgetState* state) {
    // Clear previous elements
    widget_reset(state->app->widget);
    
    // Format display text
    furi_string_printf(
        state->display_text,
        "Counter: %lu",
        state->counter);
    
    // Add updated elements
    widget_add_string_element(
        state->app->widget,
        64, 10,
        AlignCenter,
        AlignTop,
        FontPrimary,
        "Dynamic Widget");
        
    widget_add_string_element(
        state->app->widget,
        64, 32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(state->display_text));
        
    // Add increment/decrement buttons
    widget_add_button_element(
        state->app->widget,
        GuiButtonTypeLeft,
        "-",
        decrement_callback,
        state);
        
    widget_add_button_element(
        state->app->widget,
        GuiButtonTypeRight,
        "+",
        increment_callback,
        state);
}

static void decrement_callback(GuiButtonType result, InputType type, void* context) {
    HelloWidgetState* state = context;
    if(type == InputTypeShort) {
        state->counter--;
        hello_widget_update_display(state);
    }
}

static void increment_callback(GuiButtonType result, InputType type, void* context) {
    HelloWidgetState* state = context;
    if(type == InputTypeShort) {
        state->counter++;
        hello_widget_update_display(state);
    }
}
```

**Key APIs Used:**
- `widget_add_button_element()`: Adds a button to the widget
- `ButtonCallback`: Callback function for button press

### 4. Advanced Layouts

#### Exercise 4.1: Multi-line Text
Create a scrollable text display.

```c
// Initialize widget content
void hello_widget_app_init(HelloWidgetApp* app) {
    // Add a frame around the screen
    widget_add_frame_element(app->widget, 0, 0, 128, 64, 0);
    
    // Add centered title text
    widget_add_string_element(
        app->widget,
        64, 10,         // x, y coordinates
        AlignCenter,    // horizontal alignment
        AlignTop,       // vertical alignment
        FontPrimary,    // font choice
        "Hello Widget!"
    );
    
    // Add scrollable text area
    widget_add_text_scroll_element(
        app->widget,
        0, 32,         // x, y coordinates
        128, 32,       // width, height
        "This is a long text that will be scrolled.");
}
```

**Key APIs Used:**
- `widget_add_text_scroll_element()`: Adds a scrollable text area to the widget

#### Exercise 4.2: Dynamic Content
Build a simple file viewer with dynamic content.

```c
typedef struct {
    HelloWidgetApp* app;
    FuriString* file_name;
    FuriString* file_content;
} HelloWidgetState;

void hello_widget_update_display(HelloWidgetState* state) {
    // Clear previous elements
    widget_reset(state->app->widget);
    
    // Format display text
    furi_string_printf(
        state->file_name,
        "File: %s",
        "example.txt");
    
    // Add updated elements
    widget_add_string_element(
        state->app->widget,
        64, 10,
        AlignCenter,
        AlignTop,
        FontPrimary,
        furi_string_get_cstr(state->file_name));
        
    widget_add_string_multiline_element(
        state->app->widget,
        64, 32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        furi_string_get_cstr(state->file_content));
}
```

**Key APIs Used:**
- `widget_reset()`: Resets the widget to its initial state
- `widget_add_string_element()`: Adds a string element to the widget
- `widget_add_string_multiline_element()`: Adds a multi-line string element to the widget

### 5. Integration Challenges

#### Exercise 5.1: Widget with Scene Manager
Integrate your widget into a scene-based application.

```c
// Initialize scene manager
void hello_widget_app_init(HelloWidgetApp* app) {
    // Create scene manager
    app->scene_manager = scene_manager_alloc();
    
    // Add scenes to scene manager
    scene_manager_add_scene(
        app->scene_manager,
        HelloWidgetViewWidget,
        hello_widget_scene_enter,
        hello_widget_scene_exit);
}
```

**Key APIs Used:**
- `scene_manager_alloc()`: Allocates a new scene manager
- `scene_manager_add_scene()`: Adds a scene to the scene manager

#### Exercise 5.2: Multiple Widgets
Create an application using multiple widgets for different views.

```c
typedef struct {
    HelloWidgetApp* app;
    Widget* widget1;
    Widget* widget2;
} HelloWidgetState;

void hello_widget_update_display(HelloWidgetState* state) {
    // Clear previous elements
    widget_reset(state->app->widget);
    
    // Add updated elements
    widget_add_string_element(
        state->widget1,
        64, 10,
        AlignCenter,
        AlignTop,
        FontPrimary,
        "Widget 1");
        
    widget_add_string_element(
        state->widget2,
        64, 32,
        AlignCenter,
        AlignCenter,
        FontSecondary,
        "Widget 2");
}
```

**Key APIs Used:**
- `widget_alloc()`: Allocates a new widget
- `widget_add_string_element()`: Adds a string element to the widget

### 6. Best Practices Challenge

#### Exercise 6.1: Code Organization
Refactor your application following best practices.

```c
// hello_widget.h
#pragma once
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;  // Main widget instance
} HelloWidgetApp;

typedef enum {
    HelloWidgetViewWidget,  // View ID for our widget
    // Add more views here as needed
} HelloWidgetView;
```

**Key APIs Used:**
- `widget_alloc()`: Allocates a new widget
- `widget_get_view()`: Gets the view for view dispatcher
- `view_dispatcher_add_view()`: Registers widget view

#### Exercise 6.2: Performance Optimization
Optimize your widget application.

```c
// Initialize widget content
void hello_widget_app_init(HelloWidgetApp* app) {
    // Add a frame around the screen
    widget_add_frame_element(app->widget, 0, 0, 128, 64, 0);
    
    // Add centered title text
    widget_add_string_element(
        app->widget,
        64, 10,         // x, y coordinates
        AlignCenter,    // horizontal alignment
        AlignTop,       // vertical alignment
        FontPrimary,    // font choice
        "Hello Widget!"
    );
    
    // Add description text
    widget_add_string_multiline_element(
        app->widget,
        64, 32,         // x, y coordinates
        AlignCenter,    // horizontal alignment
        AlignCenter,    // vertical alignment
        FontSecondary,  // font choice
        "This is my first\nwidget application!"
    );
}
```

**Key APIs Used:**
- `widget_add_frame_element()`: Adds a frame element to the widget
- `widget_add_string_element()`: Adds a string element to the widget
- `widget_add_string_multiline_element()`: Adds a multi-line string element to the widget

## References

- Widget API Documentation
- Example Applications:
  - File Browser Test
  - SubGHz Test
  - BLE Beacon
- Flipper Zero GUI Framework Documentation
