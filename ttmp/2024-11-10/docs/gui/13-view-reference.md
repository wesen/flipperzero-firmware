# Flipper Zero View API Reference

## Core View Functions

### Allocation & Initialization
```c
// Allocate a new view
View* view_alloc(void);

// Free a view and its resources
void view_free(View* view);

// Reset the view to initial state 
void view_reset(View* view);
```

### Model Management
```c
// Allocate model storage for the view
void view_allocate_model(View* view, ViewModelType type, size_t size);
// type: ViewModelTypeLockFree or ViewModelTypeLocking
// size: Size of your model struct

// Free the view's model
void view_free_model(View* view);

// Access the view's model - prefer using with_view_model macro instead
void* view_get_model(View* view);

// Commit model changes
void view_commit_model(View* view, bool update);
```

### Callback Registration

```c
// Set drawing callback
void view_set_draw_callback(View* view, ViewDrawCallback callback);
// callback: void (*)(Canvas* canvas, void* model)

// Set input handling callback
void view_set_input_callback(View* view, ViewInputCallback callback);
// callback: bool (*)(InputEvent* event, void* context)

// Set custom event callback
void view_set_custom_callback(View* view, ViewCustomCallback callback);
// callback: bool (*)(uint32_t event, void* context)

// Set navigation callback
void view_set_previous_callback(View* view, ViewNavigationCallback callback);
// callback: uint32_t (*)(void* context)

// Set enter/exit callbacks
void view_set_enter_callback(View* view, ViewCallback callback);
void view_set_exit_callback(View* view, ViewCallback callback);
// callback: void (*)(void* context)

// Set update callback
void view_set_update_callback(View* view, ViewUpdateCallback callback);
// callback: void (*)(View* view, void* context)
```

### View Configuration

```c
// Set the view's orientation
void view_set_orientation(View* view, ViewOrientation orientation);
// orientation: ViewOrientationHorizontal, ViewOrientationHorizontalFlip,
//             ViewOrientationVertical, ViewOrientationVerticalFlip

// Set context for callbacks
void view_set_context(View* view, void* context);
```

## Common Usage Pattern

```c
// 1. Create view and model
View* view = view_alloc();
view_allocate_model(view, ViewModelTypeLocking, sizeof(MyModel));

// 2. Set up callbacks
view_set_draw_callback(view, my_draw_callback);
view_set_input_callback(view, my_input_callback);
view_set_enter_callback(view, my_enter_callback);
view_set_exit_callback(view, my_exit_callback);

// 3. Set context if needed
view_set_context(view, my_context);

// 4. Access model using with_view_model macro
with_view_model(
    view,
    MyModel* model,
    {
        // Update model here
    },
    true  // Update view after changes
);

// 5. Clean up when done
view_free(view);
```

## Model Types

- `ViewModelTypeLockFree`: For simple atomic data types or when partial updates are acceptable
- `ViewModelTypeLocking`: For complex data structures requiring mutex protection
- `ViewModelTypeNone`: When no model is needed (rare)

## Important Constants

- `VIEW_NONE`: Used to hide drawing viewport (0xFFFFFFFF)
- `VIEW_IGNORE`: Used to ignore navigation events (0xFFFFFFFE)

## Best Practices

1. Always pair `view_alloc()` with `view_free()`
2. Use `with_view_model` macro for model access
3. Return true from input callbacks when input is handled
4. Free resources in exit callback
5. Initialize resources in enter callback
6. Keep drawing callbacks efficient
7. Check for null pointers in callbacks

## Thread Safety Notes

- Model access is thread-safe when using `ViewModelTypeLocking`
- Callbacks are executed from GUI thread
- Update callbacks should be quick to maintain UI responsiveness
- Use message queues for long operations