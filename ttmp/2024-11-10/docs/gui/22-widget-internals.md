# Widget Internal Implementation

The Widget system is a foundational component of the Flipper Zero GUI framework that provides a flexible and efficient way to create composite user interfaces. This document details its internal implementation.

## Core Data Structures

### Widget Structure
The Widget maintains two key internal structures:

```c
struct Widget {
    View* view;           // Underlying view for rendering
    void* context;        // Application context
};

typedef struct {
    ElementArray_t element;  // Dynamic array of widget elements
} GuiWidgetModel;
```

- `view`: Points to the View system integration layer
- `context`: Application-specific context data
- `element`: M-Array based dynamic array storing widget elements

### Widget Elements
Each widget element is represented by the `WidgetElement` interface:

```c
typedef struct WidgetElement {
    void (*draw)(Canvas* canvas, struct WidgetElement* element);  // Draw callback
    bool (*input)(InputEvent* event, struct WidgetElement* element);  // Input handler
    void (*free)(struct WidgetElement* element);  // Cleanup function
    // Element-specific data follows
} WidgetElement;
```

## Memory Management

### Allocation Strategy
The Widget system uses a hierarchical allocation approach:

1. Widget Structure:
   - Allocated with `malloc(sizeof(Widget))`
   - View is allocated and associated
   - Model is allocated through view system

2. Widget Elements:
   - Each element is individually allocated
   - Elements are stored in a dynamic array
   - Memory is managed using M-Array macros

### Element Lifecycle
Elements follow a strict lifecycle:

1. **Allocation**: Element-specific allocation with type data
2. **Registration**: Added to widget's element array
3. **Usage**: Drawn and handling input while active
4. **Cleanup**: Free callback invoked during reset/destruction

## Rendering System

### Draw Pipeline
The widget rendering process follows these steps:

1. View system triggers `gui_widget_view_draw_callback`
2. Canvas is cleared for fresh drawing
3. Elements are drawn in registration order:
   - Each element's draw callback is invoked
   - Element handles its own rendering logic
   - Canvas state is preserved between elements

### Input Handling

The input system processes events through multiple layers:

1. View system delivers input events
2. Widget distributes events to all elements
3. Elements can consume events by returning true
4. Input propagation stops if event is consumed

## Element Implementation

### Element Types
The widget system includes several built-in element types:

1. **Text Elements**:
   - String Element: Single line text
   - Multiline Element: Multiple line text
   - Text Box: Contained text with scrolling
   - Text Scroll: Automated scrolling text

2. **Interactive Elements**:
   - Button Element: Clickable buttons
   - Frame Element: Visual containers
   - Icon Element: Image display

### Element Registration
Elements are added through specialized functions:

```c
void widget_add_element(Widget* widget, WidgetElement* element) {
    with_view_model(
        widget->view,
        GuiWidgetModel * model,
        { ElementArray_push_back(model->element, element); },
        true);
}
```

## Performance Considerations

1. **Memory Efficiency**:
   - Elements are allocated only when needed
   - M-Array provides efficient dynamic storage
   - View model ensures proper synchronization

2. **Rendering Optimization**:
   - Elements are rendered in sequence
   - No intermediate buffers needed
   - Canvas operations are direct

3. **Input Processing**:
   - Event distribution is O(n) with element count
   - Early exit on event consumption
   - Minimal event copying

## Error Handling

The Widget system implements several safety measures:

1. Null pointer validation using `furi_check()`
2. Memory allocation verification
3. Element callback validation
4. View system integration checks

## Best Practices for Extension

1. **Creating New Elements**:
   - Implement all required callbacks
   - Handle memory cleanup properly
   - Follow existing element patterns

2. **Memory Management**:
   - Free all allocated resources
   - Use `widget_reset()` for cleanup
   - Avoid resource leaks in callbacks

3. **Performance**:
   - Minimize allocations during drawing
   - Cache computed values when possible
   - Keep input handlers lightweight

## References

- Flipper Zero GUI Framework Source
- M-Array Documentation
- Widget Element Implementation Files
