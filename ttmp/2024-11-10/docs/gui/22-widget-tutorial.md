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

### 4. Common Implementation Patterns

1. **Application Structure**:
   - Widget is typically a member of the app struct
   - Allocated early in app lifecycle
   - Integrated with ViewDispatcher
   - Properly freed during cleanup

2. **View Management**:
   - Widgets provide views via `widget_get_view()`
   - Multiple views can share a widget
   - Views are switched based on application state

3. **Memory Management**:
   - Clear ownership and lifecycle
   - Proper cleanup in reverse order
   - No memory leaks through systematic freeing

## Part 2: Building Your First Widget Application

### 1. Basic Concepts

Let's build a simple "Hello Widget" application that introduces core concepts.

#### Exercise 1.1: Application Structure
Create the basic application structure with a widget.

**Hints:**
- Start with a minimal app struct
- Include necessary headers
- Think about component lifecycle

#### Exercise 1.2: Widget Allocation
Implement widget allocation and view setup.

**Hints:**
- Consider when to allocate
- Remember view dispatcher integration
- Think about cleanup order

### 2. Adding Content

#### Exercise 2.1: Static Text
Add a simple text element to your widget.

**Hints:**
- Look at string element functions
- Consider text positioning
- Think about alignment options

#### Exercise 2.2: Multiple Elements
Combine text with a frame element.

**Hints:**
- Order of element addition matters
- Consider element layering
- Think about visual hierarchy

### 3. Input Handling

#### Exercise 3.1: Button Elements
Add an interactive button to your widget.

**Hints:**
- Button callback structure
- Event handling
- State management

#### Exercise 3.2: Complex Interactions
Create a counter with increment/decrement buttons.

**Hints:**
- State storage
- Multiple button coordination
- Update triggering

### 4. Advanced Layouts

#### Exercise 4.1: Multi-line Text
Create a scrollable text display.

**Hints:**
- Text box vs scroll elements
- Content overflow handling
- Update mechanisms

#### Exercise 4.2: Dynamic Content
Build a simple file viewer with dynamic content.

**Hints:**
- Content update strategy
- Memory management
- Refresh mechanism

### 5. Integration Challenges

#### Exercise 5.1: Widget with Scene Manager
Integrate your widget into a scene-based application.

**Hints:**
- Scene transitions
- State preservation
- View management

#### Exercise 5.2: Multiple Widgets
Create an application using multiple widgets for different views.

**Hints:**
- Widget state management
- View switching
- Memory considerations

### 6. Best Practices Challenge

#### Exercise 6.1: Code Organization
Refactor your application following best practices.

**Hints:**
- Separation of concerns
- Error handling
- Resource management

#### Exercise 6.2: Performance Optimization
Optimize your widget application.

**Hints:**
- Drawing efficiency
- Memory usage
- Event handling performance

## References

- Widget API Documentation
- Example Applications:
  - File Browser Test
  - SubGHz Test
- Flipper Zero GUI Framework Documentation
