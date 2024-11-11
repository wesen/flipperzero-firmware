# FURI System Tutorial for New Developers

## Introduction
This tutorial will guide you through the basics of using FURI (Flipper Universal Registry Interface). We'll focus on the core functionality including error handling, logging, event system, and basic GUI operations.

## Prerequisites
- Basic C programming knowledge
- Flipper Zero development environment set up
- Understanding of basic embedded systems concepts

## Part 1: Error Handling and Logging

### Key Concepts
- **Error Handling**: 
  - Status codes for operation results
  - Assertion mechanisms for validation
  - Proper error propagation
  
- **Logging System**:
  - Different severity levels (Error, Warning, Info, Debug)
  - Tagged messages for categorization
  - Formatted output support

### Basic Error Handling Example
```c
#include <furi.h>

FuriStatus process_value(int32_t value) {
    // Parameter validation
    if(value < 0) {
        FURI_LOG_W("MyApp", "Invalid value: %ld", value);
        return FuriStatusInvalidParameter;
    }
    
    // Business logic check
    if(value > 100) {
        FURI_LOG_E("MyApp", "Value out of range: %ld", value);
        return FuriStatusError;
    }
    
    return FuriStatusOk;
}

void error_handling_example(void) {
    // Using status returns
    FuriStatus status = process_value(150);
    if(status != FuriStatusOk) {
        FURI_LOG_E("MyApp", "Processing failed: %d", status);
        return;
    }
    
    // Using assertions
    int* ptr = malloc(sizeof(int));
    furi_check(ptr != NULL);  // Runtime check
    
    // Debug assertion
    furi_assert(value > 0);  // Only checked in debug builds
}
```

**Exercise 1**: Create a program that:
1. Implements error handling for different scenarios
2. Uses all log levels appropriately
3. Demonstrates both assertions and status returns

### Part 2: Event System

### Key Concepts
- **Event Flags**: Binary indicators for state/actions
- **Event Operations**: Setting, clearing, waiting for events
- **Event Combinations**: Using multiple flags together

### Event System Example
```c
#include <furi.h>

#define MY_EVENT_FLAG1 (1 << 0)
#define MY_EVENT_FLAG2 (1 << 1)

void event_example(void) {
    // Wait for events with timeout
    uint32_t flags = event_flags_wait(
        flags,
        MY_EVENT_FLAG1 | MY_EVENT_FLAG2,
        FuriFlagWaitAny,
        1000  // 1 second timeout
    );
    
    if(flags & MY_EVENT_FLAG1) {
        FURI_LOG_I("Events", "Flag 1 received");
    }
    if(flags & MY_EVENT_FLAG2) {
        FURI_LOG_I("Events", "Flag 2 received");
    }
}
```

**Exercise 2**: Create an event-based program that:
1. Defines multiple event flags
2. Implements different wait strategies
3. Handles timeouts appropriately

### Part 3: Storage Operations

### Key Concepts
- **File Operations**: Opening, reading, writing files
- **Storage Access**: Using the storage subsystem
- **Resource Management**: Proper cleanup of resources

### Storage Example
```c
void storage_example(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    if(storage_file_open(file, "test.txt", FSAM_WRITE_CREATE)) {
        const char* text = "Hello, Storage!";
        if(storage_file_write(file, text, strlen(text)) != strlen(text)) {
            FURI_LOG_E("Storage", "Write failed");
        }
        storage_file_close(file);
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}
```

**Exercise 3**: Create a storage program that:
1. Implements basic file operations
2. Handles errors properly
3. Ensures proper resource cleanup

## Best Practices
- Always check return values from API calls
- Use appropriate log levels for different situations
- Clean up resources in reverse order of allocation
- Use assertions for programming errors
- Use status returns for runtime errors

## Final Project
Create an application that combines all learned concepts:
1. Proper error handling and logging
2. Event-based operations
3. File operations with proper resource management
4. Clean shutdown and cleanup

```c
#include <furi.h>

#define APP_TAG "FinalProject"

int32_t final_project(void* p) {
    UNUSED(p);
    FURI_LOG_I(APP_TAG, "Starting application");
    
    // Your implementation combining all concepts
    
    return 0;
}
```

Remember to:
- Handle all error cases
- Use appropriate logging
- Clean up resources
- Follow the patterns shown in examples