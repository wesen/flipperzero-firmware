# Building Event Loop Applications Tutorial

## Introduction

Event loops are a fundamental pattern for handling asynchronous events in applications. They allow programs to respond to multiple input sources (timers, user input, data streams) without blocking or busy waiting. This tutorial will teach you how to build applications using the FURI Event Loop system.

## Prerequisites

- Basic C programming knowledge
- Understanding of threads and concurrency concepts
- Familiarity with callback functions

## Core Concepts

### What is an Event Loop?

An event loop is a programming pattern that:

1. Waits for events to occur
2. Dispatches those events to appropriate handlers
3. Returns to waiting for more events

This creates a responsive application that can handle multiple event sources efficiently.

### FURI Event Types

The FURI system supports several event types:

- Timers (periodic or one-shot)
- Message queues
- Stream buffers
- Mutex/semaphore events

## Basic Event Loop Application

Let's start with a simple timer-based application.

### Exercise 1: Hello Timer

Create an application that prints "Hello" every second for 5 seconds then exits:

```c
#include <furi.h>

#define TAG "TimerExample"

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t count;
} TimerApp;

static void timer_callback(void* context) {
    TimerApp* app = context;

    FURI_LOG_I(TAG, "Hello! Count: %lu", app->count);

    app->count++;
    if(app->count >= 5) {
        furi_event_loop_stop(app->event_loop);
    }
}

int32_t timer_app_main(void* arg) {
    UNUSED(arg);

    TimerApp* app = malloc(sizeof(TimerApp));
    app->event_loop = furi_event_loop_alloc();
    app->count = 0;

    // Create periodic timer
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop,
        timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app
    );

    // Start timer with 1 second interval
    furi_event_loop_timer_start(app->timer, 1000);

    // Run event loop
    furi_event_loop_run(app->event_loop);

    // Cleanup
    furi_event_loop_timer_free(app->timer);
    furi_event_loop_free(app->event_loop);
    free(app);

    return 0;
}
```

Key points:

1. Create event loop with `furi_event_loop_alloc()`
2. Create timer with `furi_event_loop_timer_alloc()`
3. Start timer with desired interval
4. Run event loop until stopped
5. Clean up resources

### Exercise 2: Message Queue Handler

Let's build an application that processes messages from a queue:

```c
typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
} QueueApp;

typedef struct {
    uint32_t id;
    char message[32];
} QueueMessage;

static bool queue_callback(FuriEventLoopObject* obj, void* context) {
    QueueApp* app = context;
    QueueMessage msg;

    if(furi_message_queue_get(app->queue, &msg, 0) == FuriStatusOk) {
        FURI_LOG_I(TAG, "Message %lu: %s", msg.id, msg.message);
        return true;
    }
    return false;
}

// Usage example in main:
QueueApp* app = malloc(sizeof(QueueApp));
app->event_loop = furi_event_loop_alloc();
app->queue = furi_message_queue_alloc(10, sizeof(QueueMessage));

furi_event_loop_subscribe_message_queue(
    app->event_loop,
    app->queue,
    FuriEventLoopEventIn,
    queue_callback,
    app
);
```

## Advanced Topics

### Multiple Event Sources

For more complex applications, you'll often need to handle multiple event sources. Let's look at the example_event_loop_multi.c file:

```c:applications/examples/example_event_loop/example_event_loop_multi.c
startLine: 33
endLine: 48
```

This structure shows how to combine:

- GUI input events
- Worker thread
- Stream buffer
- Exit timer

### Exercise 3: Combined Events

Create an application that:

1. Accepts user input through a message queue
2. Generates periodic data with a timer
3. Processes that data in a worker thread

Try implementing this using the patterns from the example files.

## Best Practices

### 1. Resource Management

Always clean up in the correct order:

1. Stop timers
2. Unsubscribe from events
3. Free timers
4. Free event loop
5. Free other resources

### 2. Thread Safety

- Event loops are thread-bound
- Create event loop in the thread that will run it
- Don't access event loop from other threads

### 3. Non-blocking Callbacks

```c
// BAD:
static bool blocking_callback(FuriEventLoopObject* obj, void* ctx) {
    furi_delay_ms(1000); // Never block!
    return true;
}

// GOOD:
static bool nonblocking_callback(FuriEventLoopObject* obj, void* ctx) {
    process_data_quickly();
    return true;
}
```

## Exercises

### Exercise 4: Data Processing Pipeline

Build an application that:

1. Generates random data every 500ms
2. Processes that data in a worker thread
3. Sends results back through a stream buffer
4. Displays results in main thread

### Exercise 5: Input Handler

Create an application that:

1. Accepts keyboard input
2. Has different behaviors for short vs long presses
3. Implements a timeout for user inactivity
4. Provides visual feedback through the GUI

## Further Reading

- Review the full event system documentation
- Study the example applications in the codebase
- Experiment with different event types and combinations
