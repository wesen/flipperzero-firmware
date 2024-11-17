# Widget Technical Documentation

## For Flipper Zero Application Development

### Document Planning

#### Audience Analysis

- Primary: Flipper Zero application developers
- Expected Knowledge: C programming, basic GUI concepts
- Technical Level: Intermediate

#### Key Components

1. Widget Architecture
2. Widget Elements
3. Layout Management
4. Event Handling
5. Best Practices

### 1. Introduction

The Widget system is a fundamental UI component in the Flipper Zero application framework, designed to simplify the creation of complex user interfaces. It provides a collection of ready-to-use UI elements that can be composed to create rich, interactive displays while maintaining a consistent look and feel across applications.

At its core, the Widget system implements a composite pattern, allowing developers to build interfaces by combining multiple widget elements such as text, buttons, icons, and frames. This architecture enables rapid UI development while ensuring consistency and maintainability.

### 2. Architectural Overview

#### 2.1 Core Components

The Widget architecture consists of several key components:

1. **Widget**: The main container that manages a collection of widget elements
2. **Widget Elements**: Individual UI components that can be added to a widget:
   - Text elements (single-line, multi-line, text box)
   - Buttons
   - Icons
   - Frames
3. **View System Integration**: Widgets integrate with Flipper's View system for rendering and input handling

#### 2.2 Widget Lifecycle

Each widget follows a defined lifecycle:

1. **Creation**: Widget is allocated and initialized
2. **Element Addition**: UI elements are added to the widget
3. **Active**: Widget processes input events and renders elements
4. **Destruction**: Widget and its elements are cleaned up

### 3. Implementation Guide

#### 3.1 Widget Creation and Lifecycle

Creating and managing a widget involves several key steps:

```c
// 1. Allocation
Widget* widget = widget_alloc();

// 2. View Integration
View* view = widget_get_view(widget);
view_dispatcher_add_view(dispatcher, view_id, view);

// 3. Element Addition and Usage
// ... add elements and handle events ...

// 4. Cleanup
widget_reset(widget);  // Clear elements but keep widget
widget_free(widget);   // Full cleanup
```

#### 3.2 Adding Elements

The Widget system provides several methods to add UI elements:

```c
// Text Elements
widget_add_string_element(widget, x, y, AlignLeft, AlignTop, FontPrimary, "Text");
widget_add_string_multiline_element(widget, x, y, AlignCenter, AlignCenter, FontSecondary, "Multi\nLine");
widget_add_text_box_element(widget, x, y, width, height, AlignLeft, AlignTop, "Rich \e#Bold\e# Text", false);
widget_add_text_scroll_element(widget, x, y, width, height, "Scrollable Content");

// Interactive Elements
widget_add_button_element(widget, GuiButtonTypeLeft, "OK", button_callback, context);

// Visual Elements
widget_add_icon_element(widget, x, y, &I_icon_name);
widget_add_frame_element(widget, x, y, width, height, radius);
```

#### 3.3 Text Formatting Options

Text elements support various formatting features:

1. **String Element**: Single line text with alignment control
2. **Multiline Element**: Multiple lines with automatic wrapping
3. **Text Box**: Rich text formatting support:
   - `\e#Text\e#`: Bold text
   - `\e*Text\e*`: Monospaced text
   - `\e!Text\e!`: Inverted colors
4. **Text Scroll**: Scrollable content with formatting:
   - `\ec`: Center align line
   - `\er`: Right align line
   - Automatic scroll bar indicator

### 4. Internal Architecture

#### 4.1 Core Components

The Widget system is built on these key structures:

```c
struct Widget {
    View* view;           // Underlying view for rendering
    void* context;        // Application context
};

typedef struct {
    ElementArray_t element;  // Dynamic array of widget elements
} GuiWidgetModel;
```

#### 4.2 Memory Management

The Widget system employs a hierarchical memory management approach:

1. **Allocation**: Widgets and elements are individually allocated
2. **Registration**: Elements are stored in a dynamic array
3. **Cleanup**: Proper resource management through:
   - `widget_reset()`: Clears elements but keeps widget
   - `widget_free()`: Complete cleanup of widget and elements

### 5. Real-World Examples

#### 5.1 File Browser Implementation

```c
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    FileBrowser* file_browser;
} FileBrowserApp;

// Initialize widget
app->widget = widget_alloc();
view_dispatcher_add_view(
    app->view_dispatcher,
    FileBrowserAppViewStart,
    widget_get_view(app->widget));

// Create start screen
widget_add_string_multiline_element(
    app->widget, 64, 20, AlignCenter, AlignTop,
    FontSecondary, "Press OK to start");
widget_add_button_element(
    app->widget, GuiButtonTypeCenter, "Ok",
    file_browser_scene_start_ok_callback, app);
```

#### 5.2 Complex Layout Example

```c
// Title with styling
widget_add_text_box_element(
    widget, 0, 0, 128, 14, AlignCenter, AlignBottom,
    "\e#\e!         Title          \e!\n",
    false);

// Scrollable content
widget_add_text_scroll_element(
    widget, 0, 16, 128, 50,
    content_string);

// Action buttons
widget_add_button_element(
    widget, GuiButtonTypeLeft, "Back",
    back_callback, context);
widget_add_button_element(
    widget, GuiButtonTypeRight, "Next",
    next_callback, context);
```

### 6. Best Practices

1. **Memory Management**:
   - Always free widgets when no longer needed
   - Use `widget_reset()` between scenes
   - Follow proper allocation/deallocation order

2. **Layout Design**:
   - Use consistent spacing and alignment
   - Consider screen boundaries (128x64 typical)
   - Combine elements for complex layouts

3. **Performance**:
   - Minimize allocations during drawing
   - Cache computed values when possible
   - Keep input handlers lightweight

4. **Error Handling**:
   - Validate all pointer parameters
   - Check memory allocation success
   - Implement proper cleanup in error cases

### 7. API Reference

For a complete reference of all Widget APIs and elements, please refer to the Widget Reference Documentation.

### 8. References

- Flipper Zero GUI Framework Documentation
- Widget Element Implementation Files
