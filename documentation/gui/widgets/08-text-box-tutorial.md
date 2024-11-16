# Text Box Widget Tutorial

## Overview
The Text Box widget provides a scrollable text display area with support for multiple lines of text. It's ideal for displaying large amounts of text content, logs, or detailed information that requires scrolling. The widget supports different font styles and automatic text wrapping.

## Prerequisites
- Basic understanding of Flipper Zero's GUI framework
- Familiarity with view management and callbacks

## Header Inclusion
```c
#include <gui/modules/text_box.h>
```

## Struct Definition
```c
typedef struct {
    // Your application state variables
    TextBox* text_box;
    ViewDispatcher* view_dispatcher;
} YourApp;
```

## Initialization
```c
// Allocate text box
app->text_box = text_box_alloc();

// Set text content
text_box_set_text(app->text_box, "Your multi-line\ntext content here");

// Optional: Set font style
text_box_set_font(app->text_box, TextBoxFontText);  // or TextBoxFontHex
```

## ViewDispatcher Integration
```c
// Add view to ViewDispatcher
view_dispatcher_add_view(
    app->view_dispatcher,
    YourAppViewTextBox,  // your view ID
    text_box_get_view(app->text_box));
```

## Complete Example Application
```c
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_box.h>

typedef struct {
    TextBox* text_box;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
} ExampleApp;

typedef enum {
    ExampleAppViewTextBox,
    // Add other views here
} ExampleAppView;

static ExampleApp* example_app_alloc() {
    ExampleApp* app = malloc(sizeof(ExampleApp));

    // Initialize GUI and ViewDispatcher
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize Text Box
    app->text_box = text_box_alloc();
    text_box_set_font(app->text_box, TextBoxFontText);
    
    const char* text_content = 
        "Welcome to Flipper Zero!\n\n"
        "This is a multi-line text box example.\n"
        "You can scroll through the content\n"
        "using UP and DOWN buttons.\n\n"
        "The text will automatically wrap\n"
        "if it exceeds the display width.\n\n"
        "You can display large amounts of\n"
        "text content in this widget.";
    
    text_box_set_text(app->text_box, text_content);

    // Add text box view
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleAppViewTextBox,
        text_box_get_view(app->text_box));

    return app;
}

static void example_app_free(ExampleApp* app) {
    // Remove and free views
    view_dispatcher_remove_view(app->view_dispatcher, ExampleAppViewTextBox);
    text_box_free(app->text_box);
    
    // Free ViewDispatcher
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_GUI);
    
    free(app);
}

int32_t example_app_main(void* p) {
    UNUSED(p);
    ExampleApp* app = example_app_alloc();

    // Set starting view
    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleAppViewTextBox);

    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);

    // Free resources
    example_app_free(app);
    return 0;
}
```

## Key Features
1. Multi-line text display
2. Automatic text wrapping
3. Vertical scrolling
4. Multiple font support
5. Text focus control
6. Automatic line breaking

## Customization Options
### Setting Font Style
```c
text_box_set_font(text_box, TextBoxFontText);  // Regular text font
text_box_set_font(text_box, TextBoxFontHex);   // Monospace font for hex display
```

### Setting Focus Position
```c
text_box_set_focus(text_box, TextBoxFocusStart);  // Focus at start
text_box_set_focus(text_box, TextBoxFocusEnd);    // Focus at end
```

### Resetting Content
```c
text_box_reset(text_box);  // Clear content and reset state
```

## Best Practices
1. Use appropriate line breaks for readability
2. Consider screen size when formatting text
3. Choose appropriate font for content type
4. Provide scroll indicators for long content
5. Handle text updates efficiently

## Common Use Cases
1. Help screens and documentation
2. Log displays
3. Message viewers
4. File content display
5. Status information
6. Debug output

## Common Issues and Solutions
1. **Issue**: Text not scrolling
   **Solution**: Ensure text content is longer than display area

2. **Issue**: Text not wrapping correctly
   **Solution**: Check text formatting and line break placement

3. **Issue**: Incorrect font display
   **Solution**: Verify font selection matches content type

## Exercises
1. Create a log viewer with automatic updates
2. Implement a help system with formatted text
3. Build a file viewer with scroll position tracking
4. Create a message display with dynamic content

## Advanced Usage
### Dynamic Content Updates
```c
void update_text_content(TextBox* text_box, const char* new_content) {
    text_box_reset(text_box);
    text_box_set_text(text_box, new_content);
}
```

### Formatted Text Display
```c
// Using text formatting for better presentation
const char* formatted_text =
    "Title: My Application\n"
    "-----------------\n\n"
    "Section 1:\n"
    "  • First item\n"
    "  • Second item\n\n"
    "Section 2:\n"
    "  1. Step one\n"
    "  2. Step two\n";

text_box_set_text(text_box, formatted_text);
```

## User Experience Tips
1. Use clear text formatting for readability
2. Include visual breaks between sections
3. Keep related information grouped together
4. Use consistent formatting patterns
5. Provide context through headers
6. Include scroll indicators for long content
