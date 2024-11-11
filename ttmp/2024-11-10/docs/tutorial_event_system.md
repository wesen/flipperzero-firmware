# FURI Event System Tutorial

## Introduction
This step-by-step tutorial will guide you through building applications using FURI's event system. You'll learn how to handle events, timers, and inter-thread communication in a clean, non-blocking way.

## Prerequisites
- Basic understanding of C programming
- Familiarity with FURI basics and threading concepts
- Basic understanding of event-driven programming

## Part 1: Understanding Event System Fundamentals

### Core Concepts
Before writing code, let's understand the key components:

1. **Event Loop**: Central processor that handles:
   - Timer events (periodic/one-shot)
   - Message queue events
   - Stream buffer events
   - Mutex/semaphore events
   - Custom events

2. **Event Types**:
```c
typedef enum {
    // Direction flags
    FuriEventLoopEventIn = 0x00000001U,  // Input events (data available) 
    FuriEventLoopEventOut = 0x00000002U, // Output events (space available)
    
    // Behavior flags
    FuriEventLoopEventFlagEdge = 0x00000004U, // Only trigger on changes
    FuriEventLoopEventFlagOnce = 0x00000008U  // One-shot event
} FuriEventLoopEvent;
```

3. **Event Loop States**:
```c
typedef enum {
    FuriEventLoopStateStopped,    // Not running
    FuriEventLoopStateIdle,       // Waiting for events
    FuriEventLoopStateProcessing  // Processing events
} FuriEventLoopState;
```

### Step 1: Creating Your First Event Loop

Let's start with a basic event loop structure:

```c
typedef struct {
    FuriEventLoop* event_loop;
    bool running;
} BasicEventApp;

// Allocate and initialize
static BasicEventApp* basic_event_app_alloc(void) {
    BasicEventApp* app = malloc(sizeof(BasicEventApp));
    
    // Create event loop instance
    app->event_loop = furi_event_loop_alloc();
    app->running = true;
    
    return app;
}

// Cleanup resources
static void basic_event_app_free(BasicEventApp* app) {
    furi_event_loop_free(app->event_loop);
    free(app);
}

// Run the event loop
static void basic_event_app_run(BasicEventApp* app) {
    // This will block until furi_event_loop_stop() is called
    furi_event_loop_run(app->event_loop);
}
```

### Step 2: Adding Timer Events

Now let's add a periodic timer to our application:

```c
typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t counter;
} TimerEventApp;

// Timer callback function
static void timer_callback(void* context) {
    TimerEventApp* app = context;
    FURI_LOG_I("Timer", "Tick: %lu", app->counter++);
    
    // Stop after 10 ticks
    if(app->counter >= 10) {
        furi_event_loop_stop(app->event_loop);
    }
}

// Initialize with timer
static TimerEventApp* timer_event_app_alloc(void) {
    TimerEventApp* app = malloc(sizeof(TimerEventApp));
    
    app->event_loop = furi_event_loop_alloc();
    app->counter = 0;
    
    // Create periodic timer
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop,
        timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app
    );
    
    return app;
}

// Run with timer
static void timer_event_app_run(TimerEventApp* app) {
    // Start timer with 1 second interval
    furi_event_loop_timer_start(app->timer, 1000);
    furi_event_loop_run(app->event_loop);
}

// Cleanup timer resources
static void timer_event_app_free(TimerEventApp* app) {
    // IMPORTANT: Free timer before event loop
    furi_event_loop_timer_free(app->timer);
    furi_event_loop_free(app->event_loop);
    free(app);
}
```

### Exercise 1: Timer Control
Modify the timer example to:
1. Add pause/resume functionality
2. Change interval dynamically
3. Switch between periodic and one-shot modes

## Part 2: Message Queue Events

### Understanding Message Queues
Message queues enable thread-safe communication between different parts of your application. Let's build a producer-consumer example:

```c
typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    bool running;
} QueueEventApp;

// Process queue messages
static bool queue_callback(FuriEventLoopObject* object, void* context) {
    QueueEventApp* app = context;
    uint32_t message;
    
    if(furi_message_queue_get(app->queue, &message, 0) == FuriStatusOk) {
        FURI_LOG_I("Queue", "Received: %lu", message);
        return true;
    }
    return false;
}

// Initialize queue handling
static QueueEventApp* queue_event_app_alloc(void) {
    QueueEventApp* app = malloc(sizeof(QueueEventApp));
    
    app->event_loop = furi_event_loop_alloc();
    app->queue = furi_message_queue_alloc(10, sizeof(uint32_t));
    app->running = true;
    
    // Subscribe to queue events
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->queue,
        FuriEventLoopEventIn,  // Monitor for incoming messages
        queue_callback,
        app
    );
    
    return app;
}

// Cleanup queue resources
static void queue_event_app_free(QueueEventApp* app) {
    // IMPORTANT: Unsubscribe before cleanup
    furi_event_loop_unsubscribe(app->event_loop, app->queue);
    furi_message_queue_free(app->queue);
    furi_event_loop_free(app->event_loop);
    free(app);
}
```

### Exercise 2: Enhanced Queue System
Build a message processing system that:
1. Handles multiple message types
2. Implements message priorities
3. Provides message filtering

## Part 3: Stream Buffer Events

Stream buffers are ideal for continuous data flows. Let's implement a data streaming example:

```c
#define STREAM_BUFFER_SIZE 128
#define STREAM_BUFFER_TRIGGER 32

typedef struct {
    FuriEventLoop* event_loop;
    FuriStreamBuffer* stream;
} StreamEventApp;

// Process incoming stream data
static bool stream_callback(FuriEventLoopObject* object, void* context) {
    StreamEventApp* app = context;
    uint8_t buffer[64];
    size_t received;
    
    received = furi_stream_buffer_receive(
        app->stream,
        buffer,
        sizeof(buffer),
        0
    );
    
    if(received > 0) {
        FURI_LOG_I("Stream", "Received %d bytes", received);
        return true;
    }
    return false;
}

// Initialize stream handling
static StreamEventApp* stream_event_app_alloc(void) {
    StreamEventApp* app = malloc(sizeof(StreamEventApp));
    
    app->event_loop = furi_event_loop_alloc();
    app->stream = furi_stream_buffer_alloc(
        STREAM_BUFFER_SIZE,
        STREAM_BUFFER_TRIGGER
    );
    
    // Subscribe to stream events
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        stream_callback,
        app
    );
    
    return app;
}
```

### Exercise 3: Stream Processing
Implement a streaming system that:
1. Handles continuous data flow
2. Implements buffer overflow protection
3. Processes data in chunks

## Best Practices

### 1. Thread Safety
Event loops are thread-bound:
```c
// CORRECT: Run in owning thread
FuriEventLoop* loop = furi_event_loop_alloc();
furi_event_loop_run(loop);

// WRONG: Running in different thread
some_other_thread_run_loop(loop); // Will crash!
```

### 2. Non-blocking Callbacks
Keep callbacks quick and non-blocking:
```c
// WRONG: Blocking callback
bool bad_callback(FuriEventLoopObject* obj, void* ctx) {
    furi_delay_ms(1000); // Never block!
    return true;
}

// CORRECT: Quick processing
bool good_callback(FuriEventLoopObject* obj, void* ctx) {
    process_event_data(obj); // Quick processing
    return true;
}
```

### 3. Resource Cleanup
Always clean up in the correct order:
```c
void cleanup_example(void) {
    // 1. Unsubscribe from events
    furi_event_loop_unsubscribe(loop, object);
    
    // 2. Stop all timers
    furi_event_loop_timer_stop(timer);
    
    // 3. Free timers
    furi_event_loop_timer_free(timer);
    
    // 4. Free event loop
    furi_event_loop_free(loop);
}
```

## Advanced Topics

### 1. Event Loop Monitoring
```c
typedef struct {
    uint32_t events_processed;
    uint32_t avg_processing_time;
    uint32_t max_queue_length;
} EventLoopStats;

void monitor_event_loop(FuriEventLoop* loop, EventLoopStats* stats) {
    // Implement monitoring logic
}
```

### 2. Custom Event Types
```c
typedef struct {
    uint32_t type;
    void* data;
    size_t data_size;
} CustomEvent;

bool handle_custom_event(FuriEventLoopObject* obj, void* ctx) {
    CustomEvent* event = get_custom_event(obj);
    return process_custom_event(event);
}
```

### 3. Error Recovery
```c
bool submit_with_retry(FuriEventLoop* loop, FuriEventLoopObject* obj) {
    const int max_retries = 3;
    int retry_count = 0;
    
    while(retry_count < max_retries) {
        if(submit_event(loop, obj)) {
            return true;
        }
        retry_count++;
        furi_delay_ms(100 * retry_count);
    }
    return false;
}
```

## Common Pitfalls and Solutions

1. **Resource Leaks**
   - Always unsubscribe from events before cleanup
   - Free timers before event loop
   - Check return values from event operations

2. **Callback Safety**
   - Keep callbacks quick and non-blocking
   - Use message queues for long operations
   - Handle errors gracefully

3. **Event Loop Performance**
   - Use edge-triggered events when possible
   - Implement proper error handling
   - Monitor event processing times