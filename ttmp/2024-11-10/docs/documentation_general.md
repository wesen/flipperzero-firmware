# FURI System Documentation

## Overview
FURI (Flipper Universal Registry Interface) is the core operating system layer of the Flipper Zero device. This document covers the fundamental services and APIs, excluding thread management and memory management which are covered in separate documents.

## Core Components

### Error Handling System

FURI provides a comprehensive error handling system with status codes and assertions:

```c
typedef enum {
    FuriStatusOk = 0,           // Operation completed successfully
    FuriStatusError = -1,       // Unspecified error
    FuriStatusTimeout = -2,     // Operation timed out
    FuriStatusNotFound = -3,    // Resource not found
    FuriStatusNoMemory = -4,    // Out of memory
    FuriStatusBusy = -5,        // Resource busy
    FuriStatusInvalidParameter = -6,  // Invalid parameter
} FuriStatus;
```

#### Assertion Mechanisms
```c
// Debug-only assertion
furi_assert(condition);  // Only checked in debug builds

// Runtime check (always enabled)
furi_check(condition);   // Always checked, crashes if false

// Crash system with message
furi_crash("Error message");  // Immediately crashes with message
```

### Logging System

FURI provides a hierarchical logging system with different severity levels:

```c
// Different log levels
FURI_LOG_E(tag, fmt, ...);  // Error level
FURI_LOG_W(tag, fmt, ...);  // Warning level
FURI_LOG_I(tag, fmt, ...);  // Info level
FURI_LOG_D(tag, fmt, ...);  // Debug level

// Example usage
FURI_LOG_I("MyApp", "Starting application v%d.%d", VERSION_MAJOR, VERSION_MINOR);
```

### Timer System

The FURI timer system provides high-precision timing capabilities:

```c
// Delay operations
furi_delay_ms(1000);    // Delay for 1000 milliseconds
furi_delay_us(100);     // Delay for 100 microseconds

// Get system time
uint32_t ticks = furi_get_tick();        // Get current system ticks
uint32_t ms = furi_ms_since_boot();      // Milliseconds since boot
```

### Event System

FURI provides an event system for asynchronous operations:

```c
// Event flags
typedef enum {
    MyEventFlag1 = (1 << 0),
    MyEventFlag2 = (1 << 1),
} MyEventFlags;

// Event flag operations
uint32_t flags = event_flags_wait(flags, wait_mode, timeout);
event_flags_set(flags);
event_flags_clear(flags);
```

### Record Storage

FURI includes a key-value storage system for persistent data:

```c
// Open storage
Storage* storage = furi_record_open(RECORD_STORAGE);

// Read/Write operations
File* file = storage_file_alloc(storage);
storage_file_open(file, "path/to/file.txt", FSAM_WRITE);
storage_file_write(file, data, size);
storage_file_close(file);
storage_file_free(file);

// Close storage
furi_record_close(RECORD_STORAGE);
```

### Notification System

FURI provides a system for user notifications:

```c
// Get notification system
NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

// Simple notifications
notification_message(notifications, &sequence_success);
notification_message(notifications, &sequence_error);

// Custom notifications
notification_message_block(notifications, &sequence_blink_blue_100);
```

### GUI System

Basic GUI primitives and operations:

```c
// Get GUI system
Gui* gui = furi_record_open(RECORD_GUI);

// Create view
View* view = view_alloc();
view_set_draw_callback(view, draw_callback);
view_set_input_callback(view, input_callback);

// Draw callback example
void draw_callback(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Hello World");
}

// Input callback example
bool input_callback(InputEvent* event, void* context) {
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            return true;
        }
    }
    return false;
}
```

## Best Practices

### Error Handling
```c
// Always check return values
FuriStatus status = some_furi_operation();
if(status != FuriStatusOk) {
    FURI_LOG_E(TAG, "Operation failed: %d", status);
    return status;
}

// Use furi_check for critical conditions
furi_check(important_pointer != NULL);
```

### Resource Management
```c
// Always pair open/close operations
SomeResource* resource = furi_record_open(RECORD_TYPE);
// ... use resource
furi_record_close(RECORD_TYPE);

// Handle cleanup in error cases
if(error_condition) {
    cleanup_resources();
    return FuriStatusError;
}
```

### Logging Guidelines
```c
// Use appropriate log levels
FURI_LOG_E(TAG, "Fatal error: %d", error_code);      // For errors
FURI_LOG_W(TAG, "Resource running low: %d", level);  // For warnings
FURI_LOG_I(TAG, "Operation completed");              // For general info
FURI_LOG_D(TAG, "Debug value: %d", debug_val);      // For debug info

// Include relevant context
FURI_LOG_I(TAG, "%s: Processing item %d of %d", __func__, current, total);
```

## Common Pitfalls

1. **Incorrect Error Handling**
   ```c
   // WRONG: Ignoring return values
   some_furi_operation();

   // RIGHT: Check return values
   if(some_furi_operation() != FuriStatusOk) {
       // Handle error
   }
   ```

2. **Resource Leaks**
   ```c
   // WRONG: Not closing records
   Resource* res = furi_record_open(RECORD_TYPE);
   if(error) return;  // Resource leak!

   // RIGHT: Ensure cleanup
   Resource* res = furi_record_open(RECORD_TYPE);
   if(error) {
       furi_record_close(RECORD_TYPE);
       return;
   }
   ```

3. **Inappropriate Log Levels**
   ```c
   // WRONG: Using debug for important errors
   FURI_LOG_D(TAG, "Critical system error!");

   // RIGHT: Use appropriate level
   FURI_LOG_E(TAG, "Critical system error!");
   ```

## API Usage Examples

### Basic Application Structure
```c
#include <furi.h>

typedef struct {
    // Application state
    bool running;
    // Other state variables
} MyApp;

int32_t my_app_main(void* p) {
    UNUSED(p);
    
    MyApp* app = malloc(sizeof(MyApp));
    app->running = true;

    FURI_LOG_I("MyApp", "Application started");

    while(app->running) {
        // Main application loop
        furi_delay_ms(100);
    }

    free(app);
    return 0;
}
```

### Storage Operations
```c
bool save_data(Storage* storage, const char* filename, const void* data, size_t size) {
    File* file = storage_file_alloc(storage);
    bool success = false;
    
    do {
        if(!storage_file_open(file, filename, FSAM_WRITE)) break;
        if(storage_file_write(file, data, size) != size) break;
        success = true;
    } while(0);
    
    storage_file_close(file);
    storage_file_free(file);
    return success;
}
```
