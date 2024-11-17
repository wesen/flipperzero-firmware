# Widget Element Tutorial

This tutorial provides a comprehensive guide to understanding and implementing widget elements in the Flipper Zero GUI framework.

## 1. Widget Element Architecture

### 1.1 Core Interface

Every widget element implements the `WidgetElement` interface:

```c
struct WidgetElement {
    // Drawing callback - renders the element
    void (*draw)(Canvas* canvas, WidgetElement* element);

    // Input handling callback - processes user input
    bool (*input)(InputEvent* event, WidgetElement* element);

    // Cleanup callback - frees resources
    void (*free)(WidgetElement* element);

    // Element-specific data
    void* model;
    FuriMutex* model_mutex;

    // Parent widget reference
    Widget* parent;
};
```

### 1.2 Element Lifecycle

1. **Creation**: Element is allocated and initialized with specific parameters
2. **Registration**: Element is added to a widget via `widget_add_element()`
3. **Usage**: Element's draw and input callbacks are invoked by the widget
4. **Cleanup**: Element's free callback is called during widget reset/destruction

## 2. Understanding Existing Elements

### 2.1 String Element (Simple)

The string element demonstrates basic element implementation:

```c
typedef struct {
    uint8_t x;
    uint8_t y;
    Align horizontal;
    Align vertical;
    Font font;
    char* text;
} StringElementModel;

static void string_draw_callback(Canvas* canvas, WidgetElement* element) {
    StringElementModel* model = element->model;
    canvas_set_font(canvas, model->font);
    canvas_draw_str_aligned(
        canvas,
        model->x,
        model->y,
        model->horizontal,
        model->vertical,
        model->text);
}
```

### 2.2 Button Element (Interactive)

The button element shows how to handle user input:

```c
typedef struct {
    GuiButtonType button_type;
    char* text;
    ButtonCallback callback;
    void* context;
} ButtonElementModel;

static bool button_input_callback(InputEvent* event, WidgetElement* element) {
    ButtonElementModel* model = element->model;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(model->callback) {
            model->callback(model->context);
            consumed = true;
        }
    }

    return consumed;
}
```

### 2.3 Text Scroll Element (Complex)

The text scroll element demonstrates advanced features:

- Stateful behavior (scroll position)
- Complex input handling (up/down navigation)
- Dynamic rendering (scroll indicators)
- Rich text formatting

## 3. Creating Custom Elements

### 3.1 Basic Element Template

Here's a template for creating a new widget element:

```c
// 1. Define the element's model
typedef struct {
    // Element-specific data
    uint8_t x;
    uint8_t y;
    // ... other parameters
} CustomElementModel;

// 2. Implement callbacks
static void custom_draw(Canvas* canvas, WidgetElement* element) {
    CustomElementModel* model = element->model;
    // Drawing logic here
}

static bool custom_input(InputEvent* event, WidgetElement* element) {
    CustomElementModel* model = element->model;
    bool consumed = false;
    // Input handling logic here
    return consumed;
}

static void custom_free(WidgetElement* element) {
    CustomElementModel* model = element->model;
    // Free any allocated resources
    free(model);
    free(element);
}

// 3. Create element constructor
WidgetElement* widget_element_custom_create(uint8_t x, uint8_t y) {
    WidgetElement* element = malloc(sizeof(WidgetElement));

    // Initialize callbacks
    element->draw = custom_draw;
    element->input = custom_input;
    element->free = custom_free;

    // Initialize model
    CustomElementModel* model = malloc(sizeof(CustomElementModel));
    model->x = x;
    model->y = y;
    element->model = model;

    return element;
}
```

### 3.2 Best Practices

1. **Memory Management**:

   - Free all allocated resources in the free callback
   - Use model mutex for thread safety if needed
   - Clear all pointers after freeing

2. **Drawing**:

   - Respect canvas state (font, color)
   - Use canvas helper functions
   - Consider screen boundaries

3. **Input Handling**:

   - Return true only if input was consumed
   - Handle relevant input types only
   - Keep handlers lightweight

4. **Thread Safety**:
   - Use model mutex when accessing shared data
   - Keep critical sections short
   - Don't block in callbacks

## 4. Advanced Topics

### 4.1 Element Composition

Elements can be composed to create complex widgets:

```c
// Create a labeled button
void widget_add_labeled_button(
    Widget* widget,
    const char* label,
    const char* button_text,
    ButtonCallback callback) {

    // Add label
    widget_add_string_element(
        widget, 0, 0, AlignLeft, AlignTop,
        FontPrimary, label);

    // Add button below
    widget_add_button_element(
        widget,
        GuiButtonTypeCenter,
        button_text,
        callback,
        NULL);
}
```

### 4.2 State Management

For elements with internal state:

```c
typedef struct {
    uint32_t state;
    bool need_update;
} StatefulElementModel;

static void stateful_draw(Canvas* canvas, WidgetElement* element) {
    StatefulElementModel* model = element->model;

    furi_mutex_acquire(element->model_mutex, FuriWaitForever);
    if(model->need_update) {
        // Update state
        model->need_update = false;
    }
    // Draw based on state
    furi_mutex_release(element->model_mutex);
}
```

### 4.3 Animation Support

Elements can implement animations:

```c
typedef struct {
    float progress;
    FuriTimer* timer;
} AnimatedElementModel;

static void animation_timer_callback(void* context) {
    WidgetElement* element = context;
    AnimatedElementModel* model = element->model;

    furi_mutex_acquire(element->model_mutex, FuriWaitForever);
    model->progress += 0.1f;
    if(model->progress > 1.0f) model->progress = 0.0f;
    furi_mutex_release(element->model_mutex);
}
```

## 5. Debugging Tips

1. **Visual Debugging**:

   ```c
   // Draw element bounds
   canvas_draw_frame(
       canvas,
       model->x,
       model->y,
       model->width,
       model->height);
   ```

2. **State Verification**:

   ```c
   static void element_draw(Canvas* canvas, WidgetElement* element) {
       furi_assert(element);
       furi_assert(element->model);
       // ... drawing code
   }
   ```

3. **Input Verification**:
   ```c
   static bool element_input(InputEvent* event, WidgetElement* element) {
       furi_assert(event);
       // Log input events for debugging
       FURI_LOG_D("Element", "Input: type=%d key=%d", event->type, event->key);
       // ... input handling
   }
   ```

## 6. Practical Tutorial: Creating a Progress Bar Element

In this tutorial, we'll create a custom progress bar element for the Flipper Zero GUI framework. A progress bar is a common UI element that visually represents the completion status of an operation, making it an excellent example for learning widget element creation.

### Understanding Widget Elements

Before we start, let's understand what makes up a widget element:

1. **Interface**: The public API that other code uses to interact with our element
2. **Model**: The internal data structure that holds the element's state
3. **Callbacks**: Functions that handle drawing, input, and cleanup
4. **Lifecycle Management**: How the element is created, used, and destroyed

### Step 1: Define the Element Interface

The first step is defining how other code will interact with our progress bar. This is done through a header file that declares the element's public API.

**Key Concepts:**

- **API Design**: How to create a clean, intuitive interface
- **Encapsulation**: Hiding implementation details from users
- **Type Safety**: Using appropriate data types for parameters

```c
#pragma once

#include <gui/gui.h>
#include "widget_element.h"

// Create element constructor declaration
WidgetElement* widget_element_progress_bar_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t progress);

// Add function to update progress
void widget_element_progress_bar_set_progress(
    WidgetElement* element,
    uint8_t progress);
```

**Why This Approach?**

- Separates interface from implementation
- Makes the element easy to use without knowing internals
- Provides clear documentation of available functions
- Enables compile-time type checking

### Step 2: Implement the Element

Next, we implement the element's functionality. This involves creating the model structure and implementing the required callbacks.

**Key Concepts:**

- **Model Design**: How to structure internal data
- **Drawing**: Using the Canvas API effectively
- **Memory Management**: Proper allocation and cleanup
- **State Management**: Handling updates and redraws

```c
#include "widget_element_progress_bar.h"
#include <gui/elements.h>
#include <gui/gui.h>
#include <furi.h>

// Define the model structure
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t progress;  // 0-100
} ProgressBarModel;

// Drawing callback - Renders the element
static void progress_bar_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);

    ProgressBarModel* model = element->model;

    // Draw outline - creates visual boundary
    canvas_draw_frame(
        canvas,
        model->x,
        model->y,
        model->width,
        model->height);

    // Calculate and draw fill - shows progress
    uint8_t fill_width = (model->width - 2) * model->progress / 100;
    canvas_draw_box(
        canvas,
        model->x + 1,
        model->y + 1,
        fill_width,
        model->height - 2);
}

// Input callback - No input handling needed for progress bar
static bool progress_bar_input(InputEvent* event, WidgetElement* element) {
    UNUSED(event);
    UNUSED(element);
    return false;
}

// Cleanup callback - Frees allocated resources
static void progress_bar_free(WidgetElement* element) {
    furi_assert(element);
    free(element->model);
    free(element);
}

// Constructor - Creates and initializes the element
WidgetElement* widget_element_progress_bar_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t progress) {

    furi_assert(width > 2);
    furi_assert(height > 2);
    furi_assert(progress <= 100);

    WidgetElement* element = malloc(sizeof(WidgetElement));

    // Set callbacks
    element->draw = progress_bar_draw;
    element->input = progress_bar_input;
    element->free = progress_bar_free;

    // Initialize model
    ProgressBarModel* model = malloc(sizeof(ProgressBarModel));
    model->x = x;
    model->y = y;
    model->width = width;
    model->height = height;
    model->progress = progress;
    element->model = model;

    return element;
}

// Update function - Changes progress and triggers redraw
void widget_element_progress_bar_set_progress(
    WidgetElement* element,
    uint8_t progress) {

    furi_assert(element);
    furi_assert(progress <= 100);

    ProgressBarModel* model = element->model;
    model->progress = progress;

}
```

**Why These Implementation Choices?**

- **Simple Model**: Only stores essential data
- **Efficient Drawing**: Minimizes canvas operations
- **No Input Handling**: Progress bar is display-only
- **Safe Updates**: Validates input and handles edge cases

### Step 3: Using the Element

Now we'll look at how to integrate the progress bar into an application. This involves creating a widget and managing the element's lifecycle.

**Key Concepts:**

- **Application Structure**: How to organize GUI components
- **View Management**: Using ViewDispatcher
- **Widget Integration**: Adding elements to widgets
- **Resource Management**: Proper cleanup

```c
// Application structure
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    WidgetElement* progress_element;  // Store for updates
} YourApp;

// Widget setup
static void setup_widget(YourApp* app) {
    // Create widget container
    app->widget = widget_alloc();

    // Add descriptive title
    widget_add_string_element(
        app->widget,
        64, 10,
        AlignCenter, AlignTop,
        FontPrimary,
        "Progress Demo");

    // Create and add progress bar
    WidgetElement* progress = widget_element_progress_bar_create(
        10,    // x - left margin
        30,    // y - below title
        108,   // width - most of screen width
        8,     // height - reasonable for progress bar
        50);   // initial progress

    widget_add_element(app->widget, progress);
    app->progress_element = progress;

    // Add to view system
    view_dispatcher_add_view(
        app->view_dispatcher,
        YourAppViewProgress,
        widget_get_view(app->widget));
}

// Progress update function
void update_progress(YourApp* app, uint8_t new_progress) {
    widget_element_progress_bar_set_progress(
        app->progress_element,
        new_progress);
}

// Cleanup function
static void cleanup_widget(YourApp* app) {
    view_dispatcher_remove_view(
        app->view_dispatcher,
        YourAppViewProgress);
    widget_free(app->widget);  // Also frees elements
}
```

**Why This Integration Approach?**

- **Clean Organization**: Separates GUI setup from logic
- **Easy Updates**: Stores element for later access
- **Proper Cleanup**: Prevents memory leaks
- **View System Integration**: Works with Flipper's GUI framework

### Step 4: Testing the Element

Finally, we'll create a test function to verify our progress bar works correctly.

**Key Concepts:**

- **GUI Testing**: How to test visual elements
- **Animation**: Creating smooth visual updates
- **Resource Cleanup**: Proper teardown after testing
- **Error Checking**: Validating element behavior

```c
static void test_progress_bar() {
    // Setup GUI environment
    Widget* widget = widget_alloc();
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewDispatcher* view_dispatcher = view_dispatcher_alloc();

    // Create and add progress bar
    WidgetElement* progress = widget_element_progress_bar_create(
        10, 30, 108, 8, 0);
    widget_add_element(widget, progress);

    // Setup view
    view_dispatcher_add_view(
        view_dispatcher,
        0,
        widget_get_view(widget));

    view_dispatcher_attach_to_gui(
        view_dispatcher,
        gui,
        ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(view_dispatcher, 0);

    // Test progress updates
    for(uint8_t i = 0; i <= 100; i += 10) {
        widget_element_progress_bar_set_progress(progress, i);
        furi_delay_ms(500);  // Animate smoothly
    }

    // Cleanup in reverse order
    view_dispatcher_remove_view(view_dispatcher, 0);
    widget_free(widget);
    view_dispatcher_free(view_dispatcher);
    furi_record_close(RECORD_GUI);
}
```

**Why This Testing Approach?**

- **Visual Verification**: See the element in action
- **Animation Testing**: Verify smooth updates
- **Complete Setup**: Tests full GUI integration
- **Proper Cleanup**: Ensures no resource leaks

### Best Practices and Tips

1. **Memory Management**:

   - Always pair allocations with frees
   - Use furi_assert for pointer validation
   - Clean up resources in reverse order

2. **Drawing Performance**:

   - Minimize canvas operations
   - Only redraw when necessary
   - Use efficient drawing primitives

3. **Error Handling**:

   - Validate all input parameters
   - Handle edge cases gracefully
   - Use assertions for debugging

4. **Code Organization**:
   - Keep interface simple and focused
   - Document public functions
   - Follow Flipper Zero coding style

The progress bar element demonstrates these concepts while remaining simple enough to understand. It can be used as a template for creating more complex widget elements.

## References

- Widget System Documentation
- Flipper Zero GUI Framework
- Canvas API Reference
- Input System Documentation
