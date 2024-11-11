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
