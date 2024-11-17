# Exercise Hints and API References

## Beginner Exercises

### 1. Change text position and content
**Hints:**
- Use `canvas_draw_str()` with different coordinates
- Try different font types with `canvas_set_font()`
- Consider screen boundaries (128x64 pixels)

**Useful APIs:**
```c
canvas_draw_str(canvas, x, y, text);
canvas_set_font(canvas, FontPrimary);  // or FontSecondary, FontKeyboard
canvas_draw_str_aligned(canvas, x, y, horizontal, vertical, text);  // For alignment options
```

### 2. Add a second line of text
**Hints:**
- Calculate vertical spacing between lines
- Consider font height when positioning
- Use font metrics APIs

**Useful APIs:**
```c
canvas_get_font_height(canvas);  // Get current font height
canvas_draw_str_aligned(canvas, x, y, AlignLeft, AlignTop, text);
```

### 3. Draw a simple shape
**Hints:**
- Start with basic shapes like rectangles
- Consider fill vs outline
- Try combining multiple shapes

**Useful APIs:**
```c
canvas_draw_rect(canvas, x, y, width, height);
canvas_draw_circle(canvas, x, y, radius);
canvas_draw_line(canvas, start_x, start_y, end_x, end_y);
canvas_draw_box(canvas, x, y, width, height);  // Filled rectangle
```

### 4. Add a border around text
**Hints:**
- Calculate text width using font metrics
- Add padding around text
- Consider different border styles

**Useful APIs:**
```c
canvas_draw_frame(canvas, x, y, width, height);  // Rectangle border
canvas_get_string_width(canvas, text);  // Get text width
```

## Intermediate Exercises

### 1. Add left/right movement
**Hints:**
- Handle additional input keys
- Consider movement speed
- Add acceleration for held buttons

**Useful APIs:**
```c
InputEvent.key;  // InputKeyLeft, InputKeyRight
InputEvent.type;  // InputTypeShort, InputTypeRepeat
view_set_input_callback(view, callback);
```

### 2. Implement long press behaviors
**Hints:**
- Track press duration
- Use different actions for short vs long press
- Consider feedback for long press

**Useful APIs:**
```c
InputEvent.type;  // InputTypeLong, InputTypeRepeat
InputEvent.key;
notification_message(notifications, &sequence_blink_blue_10);  // For feedback
```

### 3. Add screen boundaries
**Hints:**
- Define constants for screen edges
- Implement boundary checking logic
- Add visual feedback at boundaries

**Useful APIs:**
```c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
MIN(x, SCREEN_WIDTH);
MAX(x, 0);
```

### 4. Create movement animation
**Hints:**
- Use view_port_update() to trigger redraws
- Implement interpolation for smooth movement
- Consider frame timing

**Useful APIs:**
```c
view_port_update(view_port);
furi_delay_ms(frame_delay);
furi_timer_alloc(callback, FuriTimerTypePeriodic, context);
```

## Advanced Exercises

### 1. Add a third scene
**Hints:**
- Plan scene transitions
- Consider scene state management
- Implement proper cleanup

**Useful APIs:**
```c
scene_manager_next_scene(scene_manager, scene_id);
scene_manager_previous_scene(scene_manager);
scene_manager_set_scene_state(scene_manager, scene_id, state);
```

### 2. Implement menu system
**Hints:**
- Use Submenu view for navigation
- Handle menu callbacks
- Maintain menu state

**Useful APIs:**
```c
submenu_add_item(submenu, label, index, callback, context);
submenu_set_selected_item(submenu, index);
view_dispatcher_switch_to_view(dispatcher, view_id);
```

### 3. Add transition effects
**Hints:**
- Use canvas transformations
- Implement animation frames
- Consider timing and smoothness

**Useful APIs:**
```c
canvas_set_orientation(canvas, orientation);
canvas_draw_box(canvas, x, y, width, height);
view_port_enabled_set(view_port, enabled);
```

### 4. Save/restore scene state
**Hints:**
- Define state structure
- Implement serialization
- Handle memory management

**Useful APIs:**
```c
scene_manager_set_scene_state(scene_manager, scene_id, state);
scene_manager_get_scene_state(scene_manager, scene_id);
storage_file_write(file, data, size);
```

## Expert Exercises

### 1. Implement scene state preservation
**Hints:**
- Create state container structure
- Handle memory allocation/deallocation
- Consider persistence needs

**Useful APIs:**
```c
storage_file_alloc(storage);
storage_file_write(file, &state, sizeof(state));
storage_file_read(file, &state, sizeof(state));
```

### 2. Add complex navigation patterns
**Hints:**
- Implement navigation stack
- Handle back navigation
- Consider modal dialogs

**Useful APIs:**
```c
scene_manager_search_and_switch_to_previous_scene(scene_manager, scene_id);
scene_manager_has_previous_scene(scene_manager, scene_id);
dialog_ex_set_result_callback(dialog, callback);
```

### 3. Multi-view scene
**Hints:**
- Manage multiple view states
- Handle view switching
- Coordinate updates

**Useful APIs:**
```c
view_stack_add_view(stack, view);
view_stack_remove_view(stack, view);
view_dispatcher_send_custom_event(dispatcher, event);
```

## Master Exercises

### 1. Complex navigation flows
**Hints:**
- Implement scene graph
- Handle conditional transitions
- Maintain navigation history

**Useful APIs:**
```c
scene_manager_next_scene(scene_manager, scene_id);
scene_manager_search_and_switch_to_another_scene(scene_manager, scene_id);
view_dispatcher_send_custom_event(dispatcher, event);
```

### 2. Modal dialogs
**Hints:**
- Stack dialog views
- Handle result callbacks
- Manage dialog lifecycle

**Useful APIs:**
```c
dialog_ex_set_context(dialog, context);
dialog_ex_set_result_callback(dialog, callback);
view_dispatcher_send_custom_event(dispatcher, event);
```

### 3. Plugin system
**Hints:**
- Define plugin interface
- Implement dynamic loading
- Handle plugin lifecycle

**Useful APIs:**
```c
storage_file_open(storage, path, mode);
storage_dir_open(storage, path);
storage_dir_read(dir, info, path_size);
```

### 4. State persistence
**Hints:**
- Define serialization format
- Handle version compatibility
- Implement error recovery

**Useful APIs:**
```c
storage_file_write(file, data, size);
storage_file_read(file, data, size);
storage_common_stat(storage, path, info);
```

## Challenge Exercises

### 1. Pub/sub event system
**Hints:**
- Implement event dispatcher
- Handle subscriber registration
- Manage event routing

**Useful APIs:**
```c
furi_pubsub_alloc();
furi_pubsub_subscribe(pubsub, callback, context);
furi_message_queue_alloc(queue_size, event_size);
```

### 2. Undo/redo functionality
**Hints:**
- Implement command pattern
- Manage command history
- Handle state restoration

**Useful APIs:**
```c
linked_list_add(list, item);
linked_list_remove(list, item);
memcpy(&state_backup, &current_state, sizeof(state));
```

Remember:
- Start with simple implementations
- Add complexity gradually
- Test thoroughly at each step
- Consider memory constraints
- Handle error cases
- Document your code