# Widget Reference Documentation

This document provides a comprehensive reference for the Flipper Zero Widget system, including all available APIs, elements, and their usage.

## Core Widget APIs

### Widget Lifecycle Management

#### `Widget* widget_alloc(void)`
Allocates a new Widget instance that can hold Widget Elements.
- **Returns**: A new Widget instance
- **Usage**: Call this when you need to create a new widget view

#### `void widget_free(Widget* widget)`
Frees a Widget instance and all its elements.
- **Parameters**:
  - `widget`: The Widget instance to free
- **Note**: This automatically frees all allocated Widget Elements

#### `void widget_reset(Widget* widget)`
Resets a Widget by removing all elements while keeping the widget itself.
- **Parameters**:
  - `widget`: The Widget instance to reset

#### `View* widget_get_view(Widget* widget)`
Gets the underlying View instance of a Widget.
- **Parameters**:
  - `widget`: The Widget instance
- **Returns**: The View instance associated with the widget
- **Usage**: Use when you need to add the widget to a ViewDispatcher

## Widget Elements

### Text Elements

#### `void widget_add_string_element(Widget* widget, uint8_t x, uint8_t y, Align horizontal, Align vertical, Font font, const char* text)`
Adds a single-line string element to the widget.
- **Parameters**:
  - `widget`: Target Widget instance
  - `x`: X coordinate
  - `y`: Y coordinate
  - `horizontal`: Horizontal alignment (Left, Right, Center)
  - `vertical`: Vertical alignment (Top, Center, Bottom)
  - `font`: Font to use
  - `text`: Text string to display

#### `void widget_add_string_multiline_element(Widget* widget, uint8_t x, uint8_t y, Align horizontal, Align vertical, Font font, const char* text)`
Adds a multi-line string element to the widget.
- **Parameters**: Same as string_element
- **Note**: Automatically handles text wrapping

#### `void widget_add_text_box_element(Widget* widget, uint8_t x, uint8_t y, uint8_t width, uint8_t height, Align horizontal, Align vertical, const char* text, bool strip_to_dots)`
Adds a text box with formatting support.
- **Parameters**:
  - Standard positioning parameters (x, y, width, height, alignments)
  - `text`: Formatted text supporting special markup:
    - `\e#Bold text\e#`: Uses bold font
    - `\e*Monospaced text\e*`: Uses monospaced font
    - `\e!Inversed text\e!`: White text on black background
  - `strip_to_dots`: If true, truncates text with "..." when it doesn't fit

#### `void widget_add_text_scroll_element(Widget* widget, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char* text)`
Adds a scrollable text element.
- **Parameters**:
  - Standard positioning parameters
  - `text`: Text content that can scroll if it exceeds the width

### Interactive Elements

#### `void widget_add_button_element(Widget* widget, GuiButtonType button_type, const char* text, ButtonCallback callback, void* context)`
Adds an interactive button element.
- **Parameters**:
  - `button_type`: Type of button (Left, Right, Center)
  - `text`: Button label
  - `callback`: Function to call when button is pressed
  - `context`: User data passed to callback

### Visual Elements

#### `void widget_add_icon_element(Widget* widget, uint8_t x, uint8_t y, const Icon* icon)`
Adds an icon element to the widget.
- **Parameters**:
  - Standard positioning parameters
  - `icon`: Icon to display

#### `void widget_add_frame_element(Widget* widget, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius)`
Adds a frame (border) element.
- **Parameters**:
  - Standard positioning parameters
  - `radius`: Corner radius of the frame

## Widget Elements in Detail

### Text Elements

#### String Element (`widget_add_string_element`)
Basic single-line text element with positioning and alignment control.
```c
void widget_add_string_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text
);
```
- **Features**:
  - Single-line text display
  - Flexible positioning (x, y coordinates)
  - Horizontal alignment (Left, Right, Center)
  - Vertical alignment (Top, Center, Bottom)
  - Font selection
- **Best Used For**:
  - Labels
  - Short text displays
  - Status indicators

#### Multiline String Element (`widget_add_string_multiline_element`)
Text element that automatically handles line wrapping.
```c
void widget_add_string_multiline_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    Font font,
    const char* text
);
```
- **Features**:
  - Automatic line wrapping
  - Maintains alignment across lines
  - Preserves whitespace
- **Best Used For**:
  - Paragraphs
  - Multi-line messages
  - Descriptions

#### Text Box Element (`widget_add_text_box_element`)
Rich text container with formatting support.
```c
void widget_add_text_box_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots
);
```
- **Features**:
  - Defined boundaries (width, height)
  - Rich text formatting:
    - `\e#Text\e#`: Bold text
    - `\e*Text\e*`: Monospaced text
    - `\e!Text\e!`: Inverted colors
  - Optional text truncation with ellipsis
- **Best Used For**:
  - Formatted text displays
  - Bounded text areas
  - Mixed-format text

#### Text Scroll Element (`widget_add_text_scroll_element`)
Scrollable text container with rich formatting support.
```c
void widget_add_text_scroll_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const char* text
);
```
- **Features**:
  - Vertical scrolling with Up/Down buttons
  - Automatic line breaking
  - Format control characters:
    - `\ec`: Center align line
    - `\er`: Right align line
    - `\e#`: Switch to primary font
    - `\e*`: Switch to monospace font
  - Scroll bar indicator
  - Maintains scroll position
- **Best Used For**:
  - Long text content
  - Documentation displays
  - Logs or message history

### Interactive Elements

#### Button Element (`widget_add_button_element`)
Interactive button with callback support.
```c
void widget_add_button_element(
    Widget* widget,
    GuiButtonType button_type,
    const char* text,
    ButtonCallback callback,
    void* context
);
```
- **Features**:
  - Three button types:
    - `GuiButtonTypeLeft`: Left button (Back)
    - `GuiButtonTypeRight`: Right button (Forward)
    - `GuiButtonTypeCenter`: Center button (OK)
  - Automatic input handling
  - Custom callback function
  - Context data passing
  - Visual feedback
- **Input Handling**:
  - Left button responds to InputKeyLeft
  - Right button responds to InputKeyRight
  - Center button responds to InputKeyOk
- **Best Used For**:
  - Navigation controls
  - Action triggers
  - Menu selections

### Visual Elements

#### Icon Element (`widget_add_icon_element`)
Display icon graphics.
```c
void widget_add_icon_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    const Icon* icon
);
```
- **Features**:
  - Precise positioning
  - Supports built-in icon set
  - Custom icon support
- **Best Used For**:
  - Status indicators
  - Visual markers
  - Application icons

#### Frame Element (`widget_add_frame_element`)
Draw bordered rectangles.
```c
void widget_add_frame_element(
    Widget* widget,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius
);
```
- **Features**:
  - Customizable dimensions
  - Rounded corners with adjustable radius
  - Single pixel border
- **Best Used For**:
  - Content grouping
  - Visual separation
  - Highlighting areas

## Element Composition Patterns

### Combining Text and Frame
```c
// Create a framed text box
widget_add_frame_element(widget, 2, 2, 124, 60, 3);
widget_add_text_box_element(
    widget, 4, 4, 120, 56,
    AlignLeft, AlignTop,
    "Content with \e#bold\e# and \e!inverse\e! text",
    false
);
```

### Interactive List Item
```c
// Create a selectable list item with icon
widget_add_icon_element(widget, 2, y, &my_icon);
widget_add_string_element(
    widget, 15, y, AlignLeft, AlignCenter,
    FontPrimary, "List Item"
);
widget_add_button_element(
    widget, GuiButtonTypeRight, "Select",
    my_callback, context
);
```

### Scrollable Documentation
```c
// Create a scrollable documentation view
widget_add_frame_element(widget, 0, 0, 128, 64, 0);
widget_add_text_scroll_element(
    widget, 2, 2, 124, 60,
    "\e#Section Title\e#\n"
    "Regular text content\n"
    "\ec\e*Centered monospace\e*\n"
    "\erRight aligned"
);
```

## Performance Considerations

1. **Element Order**:
   - Add elements in Z-order (background to foreground)
   - Frames and backgrounds first
   - Text and interactive elements last

2. **Memory Usage**:
   - Text elements create copies of strings
   - Free widgets when no longer needed
   - Use `widget_reset()` to clear elements

3. **Rendering Efficiency**:
   - Minimize overlapping elements
   - Use appropriate element types
   - Consider using string elements instead of text boxes for simple text

4. **Input Handling**:
   - Button elements automatically handle input
   - Text scroll elements manage their own scroll state
   - Multiple buttons can coexist but should be logically arranged

## Best Practices

1. **Memory Management**:
   - Always pair `widget_alloc()` with `widget_free()`
   - Use `widget_reset()` when you want to clear elements but keep the widget

2. **Element Positioning**:
   - Coordinates (0,0) start at the top-left corner
   - Use alignment options for automatic positioning
   - Consider screen boundaries (128x64 pixels)

3. **Text Formatting**:
   - Use text_box for rich text formatting
   - Consider using scroll elements for long text
   - Use multiline elements for automatic wrapping

4. **Performance**:
   - Add elements in order of background to foreground
   - Minimize the number of elements for better performance
   - Use frame elements sparingly as they impact render time

## Common Patterns

### Basic Widget Setup
```c
Widget* widget = widget_alloc();
View* view = widget_get_view(widget);
view_dispatcher_add_view(view_dispatcher, MyViewId, view);
```

### Text Display with Frame
```c
Widget* widget = widget_alloc();
widget_add_frame_element(widget, 0, 0, 128, 64, 3);
widget_add_string_element(widget, 64, 32, AlignCenter, AlignCenter, FontPrimary, "Hello World");
```

### Interactive Button
```c
Widget* widget = widget_alloc();
widget_add_button_element(widget, GuiButtonTypeLeft, "Press", button_callback, context);
```

## Related Components

- View System
- ViewDispatcher
- Scene Manager

For practical examples and tutorials, refer to the Widget Tutorial document.

## Common Issues and Solutions

1. **Text Truncation**:
   - Use text_scroll for long content
   - Enable strip_to_dots in text_box for graceful overflow
   - Consider multiline elements for automatic wrapping

2. **Layout Problems**:
   - Remember screen size is 128x64 pixels
   - Account for element padding and margins
   - Use alignment options for automatic positioning

3. **Memory Management**:
   - Always pair widget_alloc() with widget_free()
   - Clear elements with widget_reset() when reusing widgets
   - Be cautious with string lengths in text elements

4. **Input Conflicts**:
   - Organize button elements logically
   - Consider input focus when combining interactive elements
   - Use consistent navigation patterns
