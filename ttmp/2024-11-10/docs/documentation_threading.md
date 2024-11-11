# FURI Thread Management Documentation

## Overview

The Furi threading system provides a cooperative multitasking environment built on FreeRTOS. It adds several key features on top of the base RTOS:

- Thread lifecycle management with clear states
- Priority-based scheduling
- Stack monitoring and memory tracking
- Inter-thread communication via signals and flags
- Standard output redirection
- CPU usage monitoring

## Core Architecture

### Thread States and Lifecycle

A Furi thread goes through several states during its lifetime:

```
[Stopped] -> [Starting] -> [Running] -> [Stopped]
```

- **Stopped**: Initial state and final state after completion
- **Starting**: Transitional state during thread initialization 
- **Running**: Thread is actively executing

The state transitions are managed internally by the `furi_thread_set_state()` function, which also triggers any registered state callbacks.

### Thread Types

The system supports two types of threads:

1. **Regular Threads**
   - Can be created/destroyed dynamically
   - Support full lifecycle management
   - Allow stack size changes
   - Can return from their main function

2. **Service Threads** 
   - Memory-efficient system threads
   - Fixed stack size
   - Cannot return from main function
   - Used for system services and drivers

### Priority System

Threads operate with a priority hierarchy from idle tasks to ISR-level tasks:

```c
typedef enum {
    FuriThreadPriorityIdle = 1,      // Background tasks
    FuriThreadPriorityLowest = 14,   // Low priority operations
    FuriThreadPriorityLow = 15,      // Non-time-critical tasks
    FuriThreadPriorityNormal = 16,   // Default application priority
    FuriThreadPriorityHigh = 17,     // Time-sensitive operations
    FuriThreadPriorityHighest = 18,  // Critical system tasks
    FuriThreadPriorityIsr = 31,      // Reserved for ISR callbacks
} FuriThreadPriority;
```

Higher priority threads preempt lower priority ones. The scheduler ensures fair execution within the same priority level.

## Key Features

### Memory Management

The system provides comprehensive memory tracking:

- Stack usage monitoring with high watermark tracking
- Heap allocation tracking per thread (optional)
- Memory leak detection during thread cleanup

Example of stack configuration:
```c
// Recommended pattern for stack allocation
size_t required_stack = 2048;  // Base requirement
size_t safety_margin = 512;    // Safety buffer
furi_thread_set_stack_size(thread, required_stack + safety_margin);
```

### Inter-thread Communication

Threads can communicate through:

1. **Signals**: Asynchronous messages between threads
```c
// Register signal handler
furi_thread_set_signal_callback(thread, signal_handler, context);

// Send signal
furi_thread_signal(thread, SignalType, data);
```

2. **Flags**: Bitwise event notifications
```c
// Wait for specific flags
uint32_t flags = furi_thread_flags_wait(FLAG_MASK, FuriFlagWaitAll, timeout);

// Set flags on another thread
furi_thread_flags_set(thread_id, FLAG_MASK);
```

### Performance Monitoring

The system tracks CPU usage and performance metrics:

- Per-thread CPU utilization percentage
- Stack usage statistics
- ISR (Interrupt Service Routine) time tracking

Example of monitoring:
```c
FuriThreadList* list = furi_thread_list_alloc();
furi_thread_enumerate(list);

for(size_t i = 0; i < furi_thread_list_size(list); i++) {
    FuriThreadListItem* item = furi_thread_list_get_at(list, i);
    printf("Thread %s: CPU %.1f%%, Stack Free: %lu\n", 
           item->name, item->cpu, item->stack_min_free);
}
```

## Best Practices

### Thread Creation

Follow this pattern for reliable thread creation:

```c
FuriThread* thread = furi_thread_alloc_ex(
    "MyThread",           // Descriptive name
    2048,                // Adequate stack size
    thread_callback,      // Main function
    context              // Thread context
);

// Configure before starting
furi_thread_set_priority(thread, FuriThreadPriorityNormal);
furi_thread_set_state_callback(thread, state_callback, state_context);

// Start thread
furi_thread_start(thread);
```

### Resource Management

Always implement proper cleanup:

```c
// Thread callback pattern
int32_t thread_callback(void* context) {
    // 1. Initialize resources
    if(!setup_resources()) {
        return -1;
    }
    
    // 2. Main work loop
    while(!should_exit) {
        process_work();
        furi_delay_ms(10);  // Prevent CPU hogging
    }
    
    // 3. Cleanup
    cleanup_resources();
    return 0;
}
```

### Error Handling

Use the built-in status checking:

```c
FuriStatus status = furi_thread_start(thread);
if(status != FuriStatusOk) {
    // Handle error
    furi_thread_free(thread);
    return false;
}
```

## Common Pitfalls

1. **Stack Overflow**
   - Always monitor stack usage with `furi_thread_get_stack_space()`
   - Add safety margins to stack allocations

2. **Priority Inversion**
   - Use appropriate priorities
   - Avoid long critical sections

3. **Resource Leaks**
   - Enable heap tracking for development
   - Properly free resources in cleanup

4. **Deadlocks**
   - Avoid nested mutex locks
   - Use timeouts with thread synchronization

## Advanced Topics

### Thread Local Storage

The system provides thread-local storage for thread-specific data:

```c
// Store thread instance in TLS
vTaskSetThreadLocalStoragePointer(NULL, 0, thread);

// Retrieve current thread
FuriThread* current = furi_thread_get_current();
```

### Service Thread Pattern

For system services:

```c
FuriThread* service = furi_thread_alloc_service(
    "SystemService",
    1024,
    service_callback,
    NULL
);
```

### CPU Usage Monitoring

Monitor thread performance:

```c
void monitor_thread_performance(FuriThread* thread) {
    FuriThreadListItem* stats = furi_thread_list_get_stats(thread);
    if(stats->cpu > 80.0f) {
        FURI_LOG_W("Thread", 
            "High CPU usage: %.1f%% in %s",
            stats->cpu, stats->name);
    }
}
```
