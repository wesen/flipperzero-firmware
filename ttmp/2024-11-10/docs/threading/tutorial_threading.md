I'll revise the tutorial with a more step-by-step approach. Here's the first part (I'll break it into multiple messages for clarity):

```markdown
# FURI Thread Management Tutorial

## Introduction
This step-by-step tutorial will guide you through building threaded applications in FURI. We'll start with basic concepts and gradually build up to complex implementations.

## Prerequisites
- Basic C programming knowledge
- FURI development environment set up
- Basic understanding of concurrent programming

## Part 1: Building Your First Thread

### Step 1: Understanding Thread Components
Before writing code, let's understand the basic components we need:
1. Thread context structure - holds thread-specific data
2. Thread callback function - the code that runs in the thread
3. Thread creation and management code

### Step 1.1: Creating the Context Structure
First, let's create a simple structure to hold our thread's data:

```c
// Start with a basic context
typedef struct {
    bool should_exit;    // Control flag for clean shutdown
    uint32_t counter;    // Simple data to manipulate
} ThreadContext;
```

### Step 1.2: Creating the Thread Callback
Now, let's create a simple thread function that uses our context:

```c
static int32_t thread_callback(void* context) {
    // Cast the generic context pointer to our specific type
    ThreadContext* ctx = context;
    
    // Add a simple startup message
    FURI_LOG_I("Thread", "Thread starting");
    
    // Main thread loop
    while(!ctx->should_exit) {
        FURI_LOG_I("Thread", "Counter: %lu", ctx->counter++);
        furi_delay_ms(1000);
    }
    
    FURI_LOG_I("Thread", "Thread finishing");
    return 0;
}
```

### Step 1.3: Creating and Starting the Thread
Let's build the function that sets up and manages our thread:

```c
int32_t thread_example(void* p) {
    UNUSED(p);
    
    // Step 1: Create and initialize context
    ThreadContext* context = malloc(sizeof(ThreadContext));
    if(!context) {
        FURI_LOG_E("Thread", "Failed to allocate context");
        return -1;
    }
    
    context->should_exit = false;
    context->counter = 0;
    
    // Step 2: Create thread object
    FuriThread* thread = furi_thread_alloc();
    if(!thread) {
        FURI_LOG_E("Thread", "Failed to allocate thread");
        free(context);
        return -1;
    }
    
    // Step 3: Configure thread
    furi_thread_set_name(thread, "Example");
    furi_thread_set_stack_size(thread, 1024);
    furi_thread_set_callback(thread, thread_callback);
    furi_thread_set_context(thread, context);
    
    // Step 4: Start thread
    FuriStatus status = furi_thread_start(thread);
    if(status != FuriStatusOk) {
        FURI_LOG_E("Thread", "Failed to start thread");
        furi_thread_free(thread);
        free(context);
        return -1;
    }
    
    // Let it run for 5 seconds
    furi_delay_ms(5000);
    
    // Step 5: Clean shutdown
    context->should_exit = true;
    furi_thread_join(thread);
    furi_thread_free(thread);
    free(context);
    
    return 0;
}
```

### Step 1.4: Testing Your Thread
Let's add some test code:

```c
void run_thread_test(void) {
    FURI_LOG_I("Test", "Starting thread test");
    thread_example(NULL);
    FURI_LOG_I("Test", "Thread test complete");
}
```

### Exercise 1: Enhancing the Basic Thread
Try these modifications to better understand thread behavior:

1. Add a state callback:
```c
void state_callback(FuriThreadState state, void* context) {
    UNUSED(context);
    FURI_LOG_I("Thread", "State changed to: %d", state);
}

// Add to thread configuration:
furi_thread_set_state_callback(thread, state_callback, NULL);
```

2. Add stack monitoring:
```c
void monitor_stack(FuriThread* thread) {
    size_t free_stack = furi_thread_get_stack_space(thread);
    FURI_LOG_I("Thread", "Free stack: %d bytes", free_stack);
}

// Call periodically in your thread:
monitor_stack(furi_thread_get_current());
```

### Understanding What We Built
Let's break down the key concepts:

1. **Thread Context**
   - Why we need it: Holds thread-specific data
   - How it works: Passed to thread via void pointer
   - Best practice: Always initialize all fields

2. **Thread Callback**
   - Purpose: Contains the actual thread code
   - Structure: Initialize → Work Loop → Cleanup
   - Safety: Check for exit condition regularly

3. **Thread Management**
   - Creation: Allocate → Configure → Start
   - Monitoring: State callbacks, stack checking
   - Cleanup: Signal exit → Join → Free resources

### Common Issues and Solutions

1. **Stack Size**
```c
// Too small - might crash:
furi_thread_set_stack_size(thread, 256);  // DON'T DO THIS

// Better - includes safety margin:
furi_thread_set_stack_size(thread, 1024 + 512);
```

2. **Resource Leaks**
```c
// Wrong - resources not freed on error:
if(error_condition) {
    return -1;  // DON'T DO THIS
}

// Correct - proper cleanup:
if(error_condition) {
    furi_thread_free(thread);
    free(context);
    return -1;
}
```

3. **Thread Synchronization**
```c
// Wrong - no synchronization:
thread->should_exit = true;  // DON'T DO THIS

// Correct - proper shutdown sequence:
context->should_exit = true;
furi_thread_join(thread);  // Wait for thread to finish
```

### Next Steps
Try these exercises to reinforce your understanding:

1. Modify the counter to count down instead of up
2. Add error handling for memory allocation
3. Implement a maximum count limit
4. Add more detailed state logging

# Part 2: Thread Communication

## Understanding Thread Communication
Before we dive in, let's understand why and how threads need to communicate:
- Threads often need to share data
- Communication must be synchronized to prevent data corruption
- FURI provides several mechanisms for safe communication

## Step 2.1: Basic Mutex Usage

### First, let's create a simple shared resource example:

```c
// Step 1: Define the shared structure
typedef struct {
    FuriMutex* mutex;           // Protects our data
    uint32_t shared_counter;    // The shared resource
} SharedResource;

// Step 2: Create initialization function
SharedResource* shared_resource_alloc(void) {
    SharedResource* resource = malloc(sizeof(SharedResource));
    if(!resource) return NULL;
    
    // Create mutex
    resource->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!resource->mutex) {
        free(resource);
        return NULL;
    }
    
    resource->shared_counter = 0;
    return resource;
}

// Step 3: Create cleanup function
void shared_resource_free(SharedResource* resource) {
    furi_mutex_free(resource->mutex);
    free(resource);
}
```

### Now, let's use this shared resource safely:

```c
// Step 4: Create function to modify shared data
bool increment_counter(SharedResource* resource) {
    bool success = false;
    
    // Try to acquire mutex with timeout
    if(furi_mutex_acquire(resource->mutex, 100) == FuriStatusOk) {
        resource->shared_counter++;
        success = true;
        furi_mutex_release(resource->mutex);
    }
    
    return success;
}

// Step 5: Create function to read shared data
uint32_t get_counter(SharedResource* resource) {
    uint32_t value = 0;
    
    if(furi_mutex_acquire(resource->mutex, 100) == FuriStatusOk) {
        value = resource->shared_counter;
        furi_mutex_release(resource->mutex);
    }
    
    return value;
}
```

## Step 2.2: Producer-Consumer Pattern

Let's build a more complex example using producer and consumer threads:

### First, create the shared context:

```c
typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    bool should_exit;
} ProducerConsumerContext;

// Step 1: Initialize context
ProducerConsumerContext* context_alloc(void) {
    ProducerConsumerContext* ctx = malloc(sizeof(ProducerConsumerContext));
    if(!ctx) return NULL;
    
    ctx->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    ctx->queue = furi_message_queue_alloc(10, sizeof(uint32_t));
    ctx->should_exit = false;
    
    if(!ctx->mutex || !ctx->queue) {
        context_free(ctx);
        return NULL;
    }
    
    return ctx;
}

// Step 2: Cleanup function
void context_free(ProducerConsumerContext* ctx) {
    if(ctx->mutex) furi_mutex_free(ctx->mutex);
    if(ctx->queue) furi_message_queue_free(ctx->queue);
    free(ctx);
}
```

### Next, create the producer thread:

```c
// Step 3: Producer thread function
static int32_t producer_callback(void* context) {
    ProducerConsumerContext* ctx = context;
    uint32_t count = 0;
    
    FURI_LOG_I("Producer", "Starting");
    
    while(!ctx->should_exit) {
        // Create data
        count++;
        
        // Try to send to queue
        FuriStatus status = furi_message_queue_put(
            ctx->queue, 
            &count, 
            100  // timeout in ms
        );
        
        if(status == FuriStatusOk) {
            FURI_LOG_I("Producer", "Produced: %lu", count);
        } else {
            FURI_LOG_W("Producer", "Queue full!");
        }
        
        furi_delay_ms(1000);
    }
    
    FURI_LOG_I("Producer", "Stopping");
    return 0;
}
```

### Then, create the consumer thread:

```c
// Step 4: Consumer thread function
static int32_t consumer_callback(void* context) {
    ProducerConsumerContext* ctx = context;
    uint32_t received_data;
    
    FURI_LOG_I("Consumer", "Starting");
    
    while(!ctx->should_exit) {
        // Try to receive from queue
        FuriStatus status = furi_message_queue_get(
            ctx->queue,
            &received_data,
            100  // timeout in ms
        );
        
        if(status == FuriStatusOk) {
            FURI_LOG_I("Consumer", "Consumed: %lu", received_data);
        }
        
        furi_delay_ms(1500);  // Process slower than producer
    }
    
    FURI_LOG_I("Consumer", "Stopping");
    return 0;
}
```

### Finally, create the main function to run everything:

```c
// Step 5: Main function to run the example
void run_producer_consumer(void) {
    // Create context
    ProducerConsumerContext* ctx = context_alloc();
    if(!ctx) {
        FURI_LOG_E("Main", "Failed to create context");
        return;
    }
    
    // Create producer thread
    FuriThread* producer = furi_thread_alloc_ex(
        "Producer",
        1024,
        producer_callback,
        ctx
    );
    
    // Create consumer thread
    FuriThread* consumer = furi_thread_alloc_ex(
        "Consumer",
        1024,
        consumer_callback,
        ctx
    );
    
    // Start threads
    furi_thread_start(producer);
    furi_thread_start(consumer);
    
    // Let them run for a while
    furi_delay_ms(10000);
    
    // Clean shutdown
    ctx->should_exit = true;
    furi_thread_join(producer);
    furi_thread_join(consumer);
    
    // Cleanup
    furi_thread_free(producer);
    furi_thread_free(consumer);
    context_free(ctx);
}
```

## Exercises for Thread Communication

1. **Basic Mutex Exercise**
   - Modify the shared counter to keep track of minimum and maximum values
   - Add functions to safely reset the counter
   - Implement a thread-safe way to check if counter reached a threshold

2. **Producer-Consumer Exercise**
   - Add priority to messages in the queue
   - Implement multiple consumers
   - Add error recovery for full/empty queues

3. **Advanced Communication Exercise**
   - Implement a circular buffer protected by a mutex
   - Create a publish-subscribe pattern
   - Add timeout handling for all operations

## Common Pitfalls in Thread Communication

1. **Deadlock Prevention**
```c
// BAD: Nested mutex locks can deadlock
furi_mutex_acquire(mutex1, FuriWaitForever);
furi_mutex_acquire(mutex2, FuriWaitForever);  // Potential deadlock!

// GOOD: Use timeouts and error handling
if(furi_mutex_acquire(mutex1, 100) == FuriStatusOk) {
    if(furi_mutex_acquire(mutex2, 100) != FuriStatusOk) {
        furi_mutex_release(mutex1);
        return false;
    }
    // ... work with protected resources ...
    furi_mutex_release(mutex2);
    furi_mutex_release(mutex1);
}
```

2. **Resource Cleanup**
```c
// GOOD: Always clean up in reverse order of creation
void cleanup_resources(ProducerConsumerContext* ctx) {
    // First signal threads to stop
    ctx->should_exit = true;
    
    // Then wait for threads
    furi_thread_join(producer);
    furi_thread_join(consumer);
    
    // Finally clean up resources
    furi_thread_free(producer);
    furi_thread_free(consumer);
    context_free(ctx);
}
```

# Part 3: Advanced Thread Patterns

## Thread Pool Implementation

We'll build a reusable thread pool system that can handle multiple tasks efficiently. Let's break this down into manageable steps:

### Step 3.1: Basic Thread Pool Structure

First, let's define our core structures:

```c
// Step 1: Define the task structure
typedef void (*TaskFunction)(void* arg);

typedef struct {
    TaskFunction func;  // Function to execute
    void* arg;         // Arguments for the function
} Task;

// Step 2: Define the thread pool structure
typedef struct {
    FuriThread** threads;           // Array of worker threads
    size_t thread_count;            // Number of threads
    FuriMessageQueue* task_queue;   // Queue of pending tasks
    bool running;                   // Pool control flag
} ThreadPool;
```

### Step 3.2: Worker Thread Implementation

Now let's create the worker thread that will process tasks:

```c
// Step 3: Implement the worker thread function
static int32_t pool_worker(void* context) {
    ThreadPool* pool = context;
    Task task;
    
    FURI_LOG_I("Worker", "Worker thread starting");
    
    while(pool->running) {
        // Wait for tasks with timeout
        FuriStatus status = furi_message_queue_get(
            pool->task_queue, 
            &task, 
            1000  // 1 second timeout
        );
        
        if(status == FuriStatusOk) {
            // Execute the task
            FURI_LOG_D("Worker", "Executing task");
            task.func(task.arg);
            FURI_LOG_D("Worker", "Task completed");
        }
    }
    
    FURI_LOG_I("Worker", "Worker thread stopping");
    return 0;
}
```

### Step 3.3: Thread Pool Creation and Management

Let's implement the pool management functions:

```c
// Step 4: Create thread pool
ThreadPool* thread_pool_alloc(size_t num_threads) {
    // Allocate pool structure
    ThreadPool* pool = malloc(sizeof(ThreadPool));
    if(!pool) {
        FURI_LOG_E("Pool", "Failed to allocate pool structure");
        return NULL;
    }
    
    // Initialize pool members
    pool->threads = malloc(sizeof(FuriThread*) * num_threads);
    if(!pool->threads) {
        FURI_LOG_E("Pool", "Failed to allocate thread array");
        free(pool);
        return NULL;
    }
    
    // Create message queue for tasks
    pool->task_queue = furi_message_queue_alloc(20, sizeof(Task));
    if(!pool->task_queue) {
        FURI_LOG_E("Pool", "Failed to create task queue");
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    pool->thread_count = num_threads;
    pool->running = true;
    
    // Create and start worker threads
    for(size_t i = 0; i < num_threads; i++) {
        char thread_name[16];
        snprintf(thread_name, sizeof(thread_name), "Worker_%d", (int)i);
        
        pool->threads[i] = furi_thread_alloc_ex(
            thread_name,
            1024,
            pool_worker,
            pool
        );
        
        if(!pool->threads[i]) {
            FURI_LOG_E("Pool", "Failed to create worker thread %d", (int)i);
            thread_pool_free(pool);
            return NULL;
        }
        
        furi_thread_start(pool->threads[i]);
    }
    
    FURI_LOG_I("Pool", "Thread pool created with %d workers", (int)num_threads);
    return pool;
}

// Step 5: Implement task submission
bool thread_pool_submit(ThreadPool* pool, TaskFunction func, void* arg) {
    if(!pool || !func) return false;
    
    Task task = {
        .func = func,
        .arg = arg
    };
    
    FuriStatus status = furi_message_queue_put(
        pool->task_queue,
        &task,
        100  // 100ms timeout
    );
    
    if(status != FuriStatusOk) {
        FURI_LOG_W("Pool", "Failed to submit task - queue full");
        return false;
    }
    
    return true;
}

// Step 6: Implement pool cleanup
void thread_pool_free(ThreadPool* pool) {
    if(!pool) return;
    
    // Signal threads to stop
    pool->running = false;
    
    // Wait for all threads to complete
    for(size_t i = 0; i < pool->thread_count; i++) {
        if(pool->threads[i]) {
            furi_thread_join(pool->threads[i]);
            furi_thread_free(pool->threads[i]);
        }
    }
    
    // Clean up resources
    if(pool->task_queue) {
        furi_message_queue_free(pool->task_queue);
    }
    
    free(pool->threads);
    free(pool);
    
    FURI_LOG_I("Pool", "Thread pool destroyed");
}
```

### Step 3.4: Usage Example

Let's create a practical example of using the thread pool:

```c
// Step 7: Define example task
typedef struct {
    int task_id;
    int delay_ms;
} ExampleTaskData;

void example_task(void* arg) {
    ExampleTaskData* data = arg;
    FURI_LOG_I("Task", "Starting task %d", data->task_id);
    furi_delay_ms(data->delay_ms);
    FURI_LOG_I("Task", "Completed task %d", data->task_id);
    free(data);  // Free task data
}

// Step 8: Create test function
void test_thread_pool(void) {
    // Create pool with 3 workers
    ThreadPool* pool = thread_pool_alloc(3);
    if(!pool) {
        FURI_LOG_E("Test", "Failed to create thread pool");
        return;
    }
    
    // Submit several tasks
    for(int i = 0; i < 5; i++) {
        ExampleTaskData* data = malloc(sizeof(ExampleTaskData));
        data->task_id = i;
        data->delay_ms = 1000 + (i * 500);  // Varying delays
        
        if(!thread_pool_submit(pool, example_task, data)) {
            FURI_LOG_E("Test", "Failed to submit task %d", i);
            free(data);
        }
    }
    
    // Let tasks run for a while
    furi_delay_ms(10000);
    
    // Cleanup
    thread_pool_free(pool);
}
```

### Step 3.5: Advanced Features

Let's add some monitoring and management capabilities:

```c
// Step 9: Add pool statistics
typedef struct {
    size_t tasks_pending;
    size_t tasks_completed;
    uint32_t avg_processing_time;
} ThreadPoolStats;

void thread_pool_get_stats(ThreadPool* pool, ThreadPoolStats* stats) {
    if(!pool || !stats) return;
    
    stats->tasks_pending = furi_message_queue_get_count(pool->task_queue);
    // ... implement other statistics gathering
}

// Step 10: Add task prioritization
bool thread_pool_submit_priority(
    ThreadPool* pool,
    TaskFunction func,
    void* arg,
    FuriThreadPriority priority
) {
    // Implementation for priority-based task submission
    // ...
}
```

## Exercises for Advanced Patterns

1. **Thread Pool Enhancement**
   - Add task priorities
   - Implement task cancellation
   - Add worker thread recycling
   - Implement dynamic pool sizing

2. **Monitoring and Debugging**
```c
// Add performance monitoring
void monitor_pool_performance(ThreadPool* pool) {
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    
    FURI_LOG_I("Monitor", 
        "Tasks Pending: %d, Completed: %d, Avg Time: %lu ms",
        (int)stats.tasks_pending,
        (int)stats.tasks_completed,
        stats.avg_processing_time);
}
```

3. **Error Handling and Recovery**
```c
// Add robust error handling
bool submit_task_with_retry(ThreadPool* pool, TaskFunction func, void* arg) {
    const int max_retries = 3;
    int retry_count = 0;
    
    while(retry_count < max_retries) {
        if(thread_pool_submit(pool, func, arg)) {
            return true;
        }
        retry_count++;
        furi_delay_ms(100 * retry_count);  // Exponential backoff
    }
    
    return false;
}
```

## Best Practices Summary

1. **Resource Management**
```c
// Always implement proper cleanup
void cleanup_example(void) {
    // Allocate resources
    ThreadPool* pool = thread_pool_alloc(3);
    if(!pool) return;
    
    // Use try-finally pattern
    bool success = false;
    do {
        // Use resources
        if(!do_work(pool)) break;
        
        success = true;
    } while(0);
    
    // Always cleanup
    thread_pool_free(pool);
    
    if(!success) {
        FURI_LOG_E("Example", "Operation failed");
    }
}
```

2. **Error Handling**
```c
// Implement comprehensive error handling
void handle_errors(ThreadPool* pool) {
    // Check for null pointers
    if(!pool) {
        FURI_LOG_E("Pool", "Invalid pool pointer");
        return;
    }
    
    // Verify pool state
    if(!pool->running) {
        FURI_LOG_W("Pool", "Pool is not running");
        return;
    }
    
    // Handle queue overflow
    if(furi_message_queue_get_count(pool->task_queue) >= 
       furi_message_queue_get_capacity(pool->task_queue)) {
        FURI_LOG_W("Pool", "Task queue is full");
        // Implement recovery strategy
    }
}
```

3. **Performance Optimization**
```c
// Monitor and optimize performance
void optimize_pool(ThreadPool* pool) {
    ThreadPoolStats stats;
    thread_pool_get_stats(pool, &stats);
    
    // Adjust based on load
    if(stats.tasks_pending > pool->thread_count * 2) {
        FURI_LOG_W("Pool", "High task backlog - consider adding workers");
    }
    
    // Monitor processing times
    if(stats.avg_processing_time > 1000) {
        FURI_LOG_W("Pool", "Long task processing times detected");
    }
}
```
