# Event Loop API Reference

## Core Event Loop API
```c
FuriEventLoop* furi_event_loop_alloc(void);
void furi_event_loop_free(FuriEventLoop* instance);
void furi_event_loop_run(FuriEventLoop* instance);
void furi_event_loop_stop(FuriEventLoop* instance);
```

| Method | Description |
|--------|-------------|
| `furi_event_loop_alloc` | Creates a new event loop instance for the current thread |
| `furi_event_loop_free` | Destroys an event loop instance. Must be stopped first |
| `furi_event_loop_run` | Starts the event loop processing cycle |
| `furi_event_loop_stop` | Signals the event loop to stop processing |

## Timer API
```c
FuriEventLoopTimer* furi_event_loop_timer_alloc(FuriEventLoop* instance, 
    FuriEventLoopTimerCallback callback, FuriEventLoopTimerType type, void* context);
void furi_event_loop_timer_free(FuriEventLoopTimer* timer);
void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval);
void furi_event_loop_timer_restart(FuriEventLoopTimer* timer);
void furi_event_loop_timer_stop(FuriEventLoopTimer* timer);
uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer);
uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer);
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer);
```

| Method | Description |
|--------|-------------|
| `timer_alloc` | Creates a new timer (one-shot or periodic) |
| `timer_free` | Destroys a timer instance |
| `timer_start` | Starts or restarts a timer with a new interval |
| `timer_restart` | Restarts a timer with its existing interval |
| `timer_stop` | Stops a running timer |
| `timer_get_remaining_time` | Returns ticks until timer expiration |
| `timer_get_interval` | Returns the timer's current interval |
| `timer_is_running` | Checks if timer is currently active |

## Event Subscription API
```c
void furi_event_loop_subscribe_event_flag(FuriEventLoop* instance, FuriEventFlag* event_flag,
    FuriEventLoopEvent event, FuriEventLoopEventCallback callback, void* context);
void furi_event_loop_subscribe_message_queue(...);
void furi_event_loop_subscribe_stream_buffer(...);
void furi_event_loop_subscribe_semaphore(...);
void furi_event_loop_subscribe_mutex(...);
void furi_event_loop_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object);
bool furi_event_loop_is_subscribed(FuriEventLoop* instance, FuriEventLoopObject* object);
```

| Method | Description |
|--------|-------------|
| `subscribe_event_flag` | Subscribe to event flag state changes |
| `subscribe_message_queue` | Subscribe to message queue events |
| `subscribe_stream_buffer` | Subscribe to stream buffer events |
| `subscribe_semaphore` | Subscribe to semaphore events |
| `subscribe_mutex` | Subscribe to mutex events |
| `unsubscribe` | Remove an event subscription |
| `is_subscribed` | Check if object has active subscription |

## Tick API
```c
void furi_event_loop_tick_set(FuriEventLoop* instance, uint32_t interval,
    FuriEventLoopTickCallback callback, void* context);
```

| Method | Description |
|--------|-------------|
| `tick_set` | Configure background tick handler |

## Pending Callback API
```c
void furi_event_loop_pend_callback(FuriEventLoop* instance,
    FuriEventLoopPendingCallback callback, void* context);
```

| Method | Description |
|--------|-------------|
| `pend_callback` | Schedule callback for execution in event loop context |

## Event Types and Flags
```c
typedef enum {
    FuriEventLoopEventIn = 0x00000001U,
    FuriEventLoopEventOut = 0x00000002U,
    FuriEventLoopEventFlagEdge = 0x00000004U,
    FuriEventLoopEventFlagOnce = 0x00000008U
} FuriEventLoopEvent;
```

| Event/Flag | Description |
|------------|-------------|
| `EventIn` | Subscribe to input/receive events |
| `EventOut` | Subscribe to output/send events |
| `FlagEdge` | Use edge-triggered instead of level-triggered |
| `FlagOnce` | Auto-unsubscribe after first event |

## Common Callback Types
```c
typedef void (*FuriEventLoopEventCallback)(FuriEventLoopObject* object, void* context);
typedef void (*FuriEventLoopTimerCallback)(void* context);
typedef void (*FuriEventLoopTickCallback)(void* context);
typedef void (*FuriEventLoopPendingCallback)(void* context);
```

| Callback Type | Description |
|---------------|-------------|
| `EventCallback` | Called when subscribed event occurs |
| `TimerCallback` | Called when timer expires |
| `TickCallback` | Called on tick interval |
| `PendingCallback` | Called when pending callback processed |

All API functions must be called from the same thread that owns the event loop instance, except for event notifications which can come from any context including interrupts.
