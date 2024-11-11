---
source_url: https://claude.ai/chat/473fe884-0dd5-4b33-ad26-96a8f9ba269f
---

# Furi System Interfaces and Functionality

## Core System (furi.h)
- `furi_assert(condition)` - Runtime assertion checking
- `furi_check(condition)` - Validation checking with program termination
- `furi_delay_ms(ms)` - System delay function
- `FURI_LOG_D(tag, format, ...)` - Debug level logging
- `FURI_LOG_I(tag, format, ...)` - Info level logging
- `FURI_LOG_W(tag, format, ...)` - Warning level logging
- `FURI_LOG_E(tag, format, ...)` - Error level logging

## Record System (furi_record)
- `furi_record_open(record_id)` - Open/acquire system service
- `furi_record_close(record_id)` - Close/release system service

Common record IDs:
- `RECORD_GUI` - GUI system
- `RECORD_NOTIFICATION` - Notification system
- `RECORD_STORAGE` - Storage system

## Threading (furi_thread)
- `furi_thread_alloc()` - Allocate new thread
- `furi_thread_free(thread)` - Free thread resources
- `furi_thread_start(thread)` - Start thread execution
- `furi_thread_join(thread)` - Wait for thread completion
- `furi_thread_set_name(thread, name)` - Set thread name
- `furi_thread_set_stack_size(thread, size)` - Configure stack size
- `furi_thread_set_callback(thread, callback)` - Set thread function
- `furi_thread_set_context(thread, context)` - Set thread context data

## Message Queue (furi_message_queue)
- `furi_message_queue_alloc(size, item_size)` - Create message queue
- `furi_message_queue_free(queue)` - Free queue resources
- `furi_message_queue_put(queue, item, timeout)` - Send message to queue
- `furi_message_queue_get(queue, item, timeout)` - Receive message from queue

## Mutex (furi_mutex)
- `furi_mutex_alloc(mutex_type)` - Create mutex
- `furi_mutex_free(mutex)` - Free mutex
- `furi_mutex_acquire(mutex, timeout)` - Lock mutex
- `furi_mutex_release(mutex)` - Unlock mutex

## Timer (furi_timer)
- `furi_timer_alloc(callback, type, context)` - Create timer
- `furi_timer_free(timer)` - Free timer resources
- `furi_timer_start(timer, period_ms)` - Start timer
- `furi_timer_stop(timer)` - Stop timer

## Hardware Abstraction (furi_hal)
- `furi_hal_vibro_on(on)` - Control vibration motor
- `furi_hal_power_get_usb_voltage()` - Get USB voltage
- `furi_hal_rtc_get_datetime()` - Get current date/time
- `furi_hal_version_get_name_ptr()` - Get firmware version

## Constants and Types

### Wait Types
```c
FuriWaitForever    // Wait indefinitely
FuriStatusOk       // Operation success status
```

### Timer Types
```c
FuriTimerTypeOnce       // One-shot timer
FuriTimerTypePeriodic   // Repeating timer
```

### Thread Priorities
```c
FuriThreadPriorityNone
FuriThreadPriorityIdle
FuriThreadPriorityLowest
FuriThreadPriorityLow
FuriThreadPriorityNormal
FuriThreadPriorityHigh
FuriThreadPriorityHighest
```

## Common Patterns

### Service Acquisition
```c
ServiceType* service = furi_record_open(RECORD_SERVICE);
// Use service
furi_record_close(RECORD_SERVICE);
```

### Thread Creation
```c
FuriThread* thread = furi_thread_alloc();
furi_thread_set_name(thread, "ThreadName");
furi_thread_set_stack_size(thread, 1024);
furi_thread_set_callback(thread, callback_function);
furi_thread_start(thread);
```

### Message Queue Usage
```c
FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(MessageType));
furi_message_queue_put(queue, &message, FuriWaitForever);
furi_message_queue_get(queue, &message, FuriWaitForever);
```

### Timer Usage
```c
FuriTimer* timer = furi_timer_alloc(callback, FuriTimerTypePeriodic, context);
furi_timer_start(timer, period_ms);
```

## Error Handling
The Furi system generally uses return codes and the `furi_check()` macro for error handling. Critical errors trigger system halt or restart.

## Memory Management
Furi follows standard allocation/deallocation patterns with explicit resource management. All allocated resources must be freed when no longer needed.

## Threading Model
Furi uses a cooperative threading model with message passing for inter-thread communication. Threads can have different priorities and must yield regularly.

## Notes
- All timeouts are in milliseconds
- Services are singleton instances managed by the record system
- Resources must be freed in reverse order of allocation
- System services should be released before application exit
- Thread stacks should be sized appropriately for their workload