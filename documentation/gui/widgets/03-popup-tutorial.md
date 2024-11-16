# Tutorial: Using the Popup Widget in Flipper Zero

## Overview

The Popup widget provides a way to display temporary messages, alerts, or confirmations to the user. It's particularly useful for:

- Showing status messages
- Displaying alerts and warnings
- Providing feedback for user actions
- Creating confirmation dialogs
- Showing loading states

## Prerequisites

- Basic C programming knowledge
- Flipper Zero firmware development environment set up
- Understanding of basic GUI concepts

## Step 1: Include Required Headers

```c
#include <gui/modules/popup.h>
#include <gui/view_dispatcher.h>
```

## Step 2: Define Popup Structure

```c
typedef struct {
    Popup* popup;
    ViewDispatcher* view_dispatcher;
    // Add your application-specific state here
} MyApp;

// Callback function for popup buttons
static void popup_callback(void* context) {
    MyApp* app = context;
    // Handle popup callback
    view_dispatcher_switch_to_view(app->view_dispatcher, PreviousViewId);
}
```

## Step 3: Initialize the Popup

```c
MyApp* app = malloc(sizeof(MyApp));

// Allocate the popup
app->popup = popup_alloc();

// Configure the popup
popup_set_header(
    app->popup,
    "Title",             // Header text
    64,                  // Header x position
    5,                   // Header y position
    AlignCenter,         // Text alignment
    AlignTop            // Vertical alignment
);

popup_set_text(
    app->popup,
    "Message text",      // Body text
    64,                  // Text x position
    32,                  // Text y position
    AlignCenter,         // Text alignment
    AlignTop            // Vertical alignment
);

// Optional: Set icon
popup_set_icon(app->popup, 0, 0, &I_icon_48x48);

// Set timeout (optional)
popup_set_timeout(app->popup, 3000);  // 3 seconds

// Set callback
popup_set_callback(app->popup, popup_callback);
popup_set_context(app->popup, app);
```

## Step 4: Integration with ViewDispatcher

```c
View* view = popup_get_view(app->popup);
view_dispatcher_add_view(view_dispatcher, MyPopupView, view);
```

## Example Application

Here's a complete example that demonstrates different types of popups:

```c
typedef struct {
    Popup* popup;
    ViewDispatcher* view_dispatcher;
} PopupDemo;

static void popup_callback(void* context) {
    PopupDemo* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);  // Switch back to main view
}

static void show_success_popup(PopupDemo* app) {
    popup_reset(app->popup);
    
    popup_set_header(app->popup, "Success!", 64, 5, AlignCenter, AlignTop);
    popup_set_text(app->popup, "Operation completed", 64, 32, AlignCenter, AlignTop);
    popup_set_icon(app->popup, 0, 0, &I_Success_48x48);
    popup_set_timeout(app->popup, 2000);  // Auto-hide after 2 seconds
    popup_set_callback(app->popup, popup_callback);
    popup_set_context(app->popup, app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, PopupViewId);
}

static void show_error_popup(PopupDemo* app) {
    popup_reset(app->popup);
    
    popup_set_header(app->popup, "Error", 64, 5, AlignCenter, AlignTop);
    popup_set_text(
        app->popup,
        "Something went wrong!\nPlease try again.",
        64,
        32,
        AlignCenter,
        AlignTop);
    popup_set_icon(app->popup, 0, 0, &I_Error_48x48);
    popup_set_callback(app->popup, popup_callback);
    popup_set_context(app->popup, app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, PopupViewId);
}

static void show_loading_popup(PopupDemo* app) {
    popup_reset(app->popup);
    
    popup_set_header(app->popup, "Loading", 64, 5, AlignCenter, AlignTop);
    popup_set_text(app->popup, "Please wait...", 64, 32, AlignCenter, AlignTop);
    popup_set_icon(app->popup, 0, 0, &I_Loading_24x24);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, PopupViewId);
}

int32_t popup_demo_app(void* p) {
    UNUSED(p);
    PopupDemo* app = malloc(sizeof(PopupDemo));

    // Allocate view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    
    // Allocate popup
    app->popup = popup_alloc();
    
    // Add popup view to dispatcher
    View* popup_view = popup_get_view(app->popup);
    view_dispatcher_add_view(app->view_dispatcher, PopupViewId, popup_view);
    
    // Show different types of popups
    show_loading_popup(app);
    furi_delay_ms(2000);  // Simulate loading
    
    show_success_popup(app);
    furi_delay_ms(2000);  // Wait for success popup to auto-hide
    
    show_error_popup(app);  // This one requires user interaction
    
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, PopupViewId);
    popup_free(app->popup);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
    
    return 0;
}
```

## Key Features

- Customizable header and body text
- Optional icon display
- Auto-hide timeout support
- Callback support for user interaction
- Flexible text alignment options
- Support for multiline text

## Common Customization Options

1. **Set Header:**
```c
popup_set_header(popup, "Title", x, y, AlignCenter, AlignTop);
```

2. **Set Body Text:**
```c
popup_set_text(popup, "Message", x, y, AlignCenter, AlignTop);
```

3. **Set Icon:**
```c
popup_set_icon(popup, x, y, &I_icon_48x48);
```

4. **Set Timeout:**
```c
popup_set_timeout(popup, milliseconds);
```

5. **Reset Popup:**
```c
popup_reset(popup);
```

## Best Practices

1. Keep messages clear and concise
2. Use appropriate icons for different message types
3. Set reasonable timeout durations
4. Clean up resources when done
5. Use consistent styling across your application
6. Consider screen space when laying out text and icons
7. Provide feedback for user interactions

## Common Use Cases

1. **Success Message:**
```c
popup_set_header(popup, "Success", 64, 5, AlignCenter, AlignTop);
popup_set_icon(popup, 0, 0, &I_Success_48x48);
popup_set_timeout(popup, 2000);
```

2. **Error Message:**
```c
popup_set_header(popup, "Error", 64, 5, AlignCenter, AlignTop);
popup_set_text(popup, "Operation failed", 64, 32, AlignCenter, AlignTop);
popup_set_icon(popup, 0, 0, &I_Error_48x48);
```

3. **Loading State:**
```c
popup_set_header(popup, "Loading", 64, 5, AlignCenter, AlignTop);
popup_set_text(popup, "Please wait...", 64, 32, AlignCenter, AlignTop);
popup_set_icon(popup, 0, 0, &I_Loading_24x24);
```

4. **Confirmation Dialog:**
```c
popup_set_header(popup, "Confirm", 64, 5, AlignCenter, AlignTop);
popup_set_text(popup, "Delete item?", 64, 32, AlignCenter, AlignTop);
popup_set_icon(popup, 0, 0, &I_Question_48x48);
popup_set_callback(popup, confirm_callback);
```

## Common Issues and Solutions

1. **Popup not showing:**
   - Verify view is added to view dispatcher
   - Check if view dispatcher is running
   - Ensure popup is properly allocated

2. **Text alignment issues:**
   - Verify x and y coordinates
   - Check alignment parameters
   - Consider text length and screen boundaries

3. **Icons not displaying:**
   - Verify icon resource exists
   - Check icon coordinates
   - Ensure icon size is appropriate

4. **Timeout not working:**
   - Verify timeout value is in milliseconds
   - Check if timeout is set after other popup configurations
   - Ensure view dispatcher is running

## Exercises

1. Create a popup with custom icon and timeout
2. Implement a confirmation dialog with Yes/No options
3. Create a loading popup with animation
4. Build a popup that displays multiline text
5. Implement a popup with dynamic content
