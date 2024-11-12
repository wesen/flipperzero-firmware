# Flipper Zero Record System Technical Documentation

## Introduction

The Record System is a fundamental component of the Flipper Zero firmware architecture that provides a thread-safe mechanism for managing system-wide services and shared resources. It implements a key-value store pattern combined with service lifecycle management capabilities, enabling orderly access to singleton services while maintaining thread safety and proper resource cleanup.

This documentation provides a comprehensive overview of the Record System's architecture, usage patterns, and best practices for developers working with the Flipper Zero firmware.

## Core Concepts

### Records and Services

A Record in the Flipper Zero firmware context represents either a named data entry or a system service. The Record system maintains these entries in a thread-safe manner, ensuring proper synchronization between multiple threads accessing the same resources.

Key characteristics of Records include:

1. Unique string identifiers (e.g., "gui", "storage", "notifications")
2. Thread-safe access control
3. Reference counting for proper cleanup
4. Support for both data and service objects

### Service Lifecycle

The Record system manages the complete lifecycle of system services:

1. Registration: Services register themselves with a unique identifier
2. Acquisition: Threads request access to services
3. Usage: Multiple threads can safely access services
4. Release: Threads signal completion of service usage
5. Cleanup: Services are properly destroyed when no longer needed

## Architecture

### Component Overview

The Record system consists of several key components working together:

1. **Record Manager**

   - Maintains the global record registry
   - Handles thread synchronization
   - Manages service lifecycles

2. **Record Data**

   - Stores actual service/data objects
   - Tracks holder count
   - Maintains state information

3. **Event Flags**
   - Provides synchronization primitives
   - Enables thread-safe state changes
   - Coordinates service availability

### Thread Safety

The Record system employs multiple synchronization mechanisms to ensure thread safety:

```c
void furi_record_create(const char* name, void* data) {
    furi_check(furi_record);
    furi_check(name);

    furi_record_lock();

    // Get record data and fill it
    FuriRecordData* record_data = furi_record_data_get_or_create(name);
    furi_check(record_data->data == NULL);
    record_data->data = data;
    furi_event_flag_set(record_data->flags, FURI_RECORD_FLAG_READY);

    furi_record_unlock();
}
```

This example demonstrates the core principles:

- Mutex-based access control
- Event flags for state signaling
- Atomic operations for reference counting

## Usage Patterns

### Service Registration

Services typically register themselves during system initialization:

```c
void my_service_start() {
    MyService* service = my_service_alloc();
    // Register service with the record system
    furi_record_create("my_service", service);
}
```

### Service Acquisition

Threads acquire services using the following pattern:

```c
void my_thread_function() {
    // Access the service through the record system
    MyService* service = furi_record_open("my_service");

    // Use the service...

    // Release the service when done
    furi_record_close("my_service");
}
```

### Error Handling

The Record system provides robust error checking:

1. Invalid record names
2. Duplicate service registration
3. Access to unregistered services
4. Resource leaks detection

## Best Practices

### Service Implementation

When implementing a new service:

1. Always use unique, descriptive record names
2. Implement proper cleanup in service destructors
3. Document service dependencies
4. Handle initialization failures gracefully

### Resource Management

To prevent resource leaks:

1. Always pair `furi_record_open()` with `furi_record_close()`
2. Use RAII-style patterns when possible
3. Implement proper error handling
4. Avoid holding records longer than necessary

## Advanced Topics

### Service Dependencies

Services often depend on other services. Handle dependencies by:

1. Documenting required services
2. Opening dependencies in correct order
3. Implementing graceful degradation
4. Avoiding circular dependencies

### Performance Considerations

The Record system is optimized for:

1. Fast service lookup
2. Minimal lock contention
3. Efficient thread synchronization
4. Low memory overhead

## Conclusion

The Record system provides a robust foundation for service management in the Flipper Zero firmware. By following the patterns and practices outlined in this documentation, developers can effectively utilize this system while maintaining thread safety and proper resource management.

Key takeaways:

1. Use records for system-wide singleton services
2. Follow proper lifecycle management patterns
3. Implement robust error handling
4. Consider thread safety implications
5. Document service dependencies clearly

The Record system's design enables scalable and maintainable service management while enforcing thread safety and proper resource cleanup.
