# Core System Context
Foundational Furi system files and core definitions containing:
- Basic assertions (furi_assert, furi_check)
- System delays
- Logging functionality (FURI_LOG_*)
Forms the backbone of the Flipper system.

---

<file name="furi/flipper.c">
<content>
#include "flipper.h"
#include <applications.h>
#include <furi.h>
#include <furi_hal_version.h>
#include <furi_hal_memory.h>
#include <furi_hal_rtc.h>

#include <FreeRTOS.h>

#define TAG "Flipper"

static void flipper_print_version(const char* target, const Version* version) {
    if(version) {
        FURI_LOG_I(
            TAG,
            "\r\n\t%s version:\t%s\r\n"
            "\tBuild date:\t\t%s\r\n"
            "\tGit Commit:\t\t%s (%s)%s\r\n"
            "\tGit Branch:\t\t%s",
            target,
            version_get_version(version),
            version_get_builddate(version),
            version_get_githash(version),
            version_get_gitbranchnum(version),
            version_get_dirty_flag(version) ? " (dirty)" : "",
            version_get_gitbranch(version));
    } else {
        FURI_LOG_I(TAG, "No build info for %s", target);
    }
}

void flipper_init(void) {
    flipper_print_version("Firmware", furi_hal_version_get_firmware_version());

    FURI_LOG_I(TAG, "Boot mode %d, starting services", furi_hal_rtc_get_boot_mode());

    for(size_t i = 0; i < FLIPPER_SERVICES_COUNT; i++) {
        FURI_LOG_D(TAG, "Starting service %s", FLIPPER_SERVICES[i].name);

        FuriThread* thread = furi_thread_alloc_service(
            FLIPPER_SERVICES[i].name,
            FLIPPER_SERVICES[i].stack_size,
            FLIPPER_SERVICES[i].app,
            NULL);
        furi_thread_set_appid(thread, FLIPPER_SERVICES[i].appid);

        furi_thread_start(thread);
    }

    FURI_LOG_I(TAG, "Startup complete");
}

void vApplicationGetIdleTaskMemory(
    StaticTask_t** tcb_ptr,
    StackType_t** stack_ptr,
    uint32_t* stack_size) {
    *tcb_ptr = memmgr_alloc_from_pool(sizeof(StaticTask_t));
    *stack_ptr = memmgr_alloc_from_pool(sizeof(StackType_t) * configIDLE_TASK_STACK_DEPTH);
    *stack_size = configIDLE_TASK_STACK_DEPTH;
}

void vApplicationGetTimerTaskMemory(
    StaticTask_t** tcb_ptr,
    StackType_t** stack_ptr,
    uint32_t* stack_size) {
    *tcb_ptr = memmgr_alloc_from_pool(sizeof(StaticTask_t));
    *stack_ptr = memmgr_alloc_from_pool(sizeof(StackType_t) * configTIMER_TASK_STACK_DEPTH);
    *stack_size = configTIMER_TASK_STACK_DEPTH;
}

</content>
</file>
<file name="furi/flipper.h">
<content>
#pragma once

void flipper_init(void);

</content>
</file>
<file name="furi/furi.c">
<content>
#include "furi.h"

#include <FreeRTOS.h>
#include <queue.h>

void furi_init(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    furi_log_init();
    furi_record_init();
}

void furi_run(void) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);

    /* Start the kernel scheduler */
    vTaskStartScheduler();
}

</content>
</file>
<file name="furi/furi.h">
<content>
#pragma once

#include <stdlib.h>

#include "core/common_defines.h"
#include "core/check.h"
#include "core/event_loop.h"
#include "core/event_loop_timer.h"
#include "core/event_flag.h"
#include "core/kernel.h"
#include "core/log.h"
#include "core/memmgr.h"
#include "core/memmgr_heap.h"
#include "core/message_queue.h"
#include "core/mutex.h"
#include "core/pubsub.h"
#include "core/record.h"
#include "core/semaphore.h"
#include "core/thread.h"
#include "core/thread_list.h"
#include "core/timer.h"
#include "core/string.h"
#include "core/stream_buffer.h"

#include <furi_hal_gpio.h>

// Workaround for math.h leaking through HAL in older versions
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init(void);

void furi_run(void);

#ifdef __cplusplus
}
#endif

</content>
</file>

<file name="furi/core/base.h">
<content>
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <furi_config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriWaitForever = 0xFFFFFFFFU,
} FuriWait;

typedef enum {
    FuriFlagWaitAny = 0x00000000U, ///< Wait for any flag (default).
    FuriFlagWaitAll = 0x00000001U, ///< Wait for all flags.
    FuriFlagNoClear = 0x00000002U, ///< Do not clear flags which have been specified to wait for.

    FuriFlagError = 0x80000000U, ///< Error indicator.
    FuriFlagErrorUnknown = 0xFFFFFFFFU, ///< FuriStatusError (-1).
    FuriFlagErrorTimeout = 0xFFFFFFFEU, ///< FuriStatusErrorTimeout (-2).
    FuriFlagErrorResource = 0xFFFFFFFDU, ///< FuriStatusErrorResource (-3).
    FuriFlagErrorParameter = 0xFFFFFFFCU, ///< FuriStatusErrorParameter (-4).
    FuriFlagErrorISR = 0xFFFFFFFAU, ///< FuriStatusErrorISR (-6).
} FuriFlag;

typedef enum {
    FuriStatusOk = 0, ///< Operation completed successfully.
    FuriStatusError =
        -1, ///< Unspecified RTOS error: run-time error but no other error message fits.
    FuriStatusErrorTimeout = -2, ///< Operation not completed within the timeout period.
    FuriStatusErrorResource = -3, ///< Resource not available.
    FuriStatusErrorParameter = -4, ///< Parameter error.
    FuriStatusErrorNoMemory =
        -5, ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
    FuriStatusErrorISR =
        -6, ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
    FuriStatusReserved = 0x7FFFFFFF ///< Prevents enum down-size compiler optimization.
} FuriStatus;

typedef enum {
    FuriSignalExit, /**< Request (graceful) exit. */
    // Other standard signals may be added in the future
    FuriSignalCustom = 100, /**< Custom signal values start from here. */
} FuriSignal;

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/check.h">
<content>
/**
 * @file check.h
 * 
 * Furi crash and assert functions.
 * 
 * The main problem with crashing is that you can't do anything without disturbing registers,
 * and if you disturb registers, you won't be able to see the correct register values in the debugger.
 * 
 * Current solution works around it by passing the message through r12 and doing some magic with registers in crash function.
 * r0-r10 are stored in the ram2 on crash routine start and restored at the end.
 * The only register that is going to be lost is r11.
 * 
 */
#pragma once

#include <m-core.h>
#include "common_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// Flags instead of pointers will save ~4 bytes on furi_assert and furi_check calls.
#define __FURI_ASSERT_MESSAGE_FLAG (0x01)
#define __FURI_CHECK_MESSAGE_FLAG  (0x02)

/** Crash system */
FURI_NORETURN void __furi_crash_implementation(void);

/** Halt system */
FURI_NORETURN void __furi_halt_implementation(void);

/** Crash system with message. Show message after reboot. */
#define __furi_crash(message)                                 \
    do {                                                      \
        register const void* r12 asm("r12") = (void*)message; \
        asm volatile("sukima%=:" : : "r"(r12));               \
        __furi_crash_implementation();                        \
    } while(0)

/** Crash system
 *
 * @param      ... optional  message (const char*)
 */
#define furi_crash(...) M_APPLY(__furi_crash, M_IF_EMPTY(__VA_ARGS__)((NULL), (__VA_ARGS__)))

/** Halt system with message. */
#define __furi_halt(message)                                  \
    do {                                                      \
        register const void* r12 asm("r12") = (void*)message; \
        asm volatile("sukima%=:" : : "r"(r12));               \
        __furi_halt_implementation();                         \
    } while(0)

/** Halt system
 *
 * @param      ... optional  message (const char*)
 */
#define furi_halt(...) M_APPLY(__furi_halt, M_IF_EMPTY(__VA_ARGS__)((NULL), (__VA_ARGS__)))

/** Check condition and crash if check failed */
#define __furi_check(__e, __m) \
    do {                       \
        if(!(__e)) {           \
            __furi_crash(__m); \
        }                      \
    } while(0)

/** Check condition and crash if failed
 *
 * @param      ... condition to check and optional  message (const char*)
 */
#define furi_check(...) \
    M_APPLY(__furi_check, M_DEFAULT_ARGS(2, (__FURI_CHECK_MESSAGE_FLAG), __VA_ARGS__))

/** Only in debug build: Assert condition and crash if assert failed  */
#ifdef FURI_DEBUG
#define __furi_assert(__e, __m) \
    do {                        \
        if(!(__e)) {            \
            __furi_crash(__m);  \
        }                       \
    } while(0)
#else
#define __furi_assert(__e, __m) \
    do {                        \
        ((void)(__e));          \
        ((void)(__m));          \
    } while(0)
#endif

/** Assert condition and crash if failed
 *
 * @warning    only will do check if firmware compiled in debug mode
 *
 * @param      ... condition to check and optional  message (const char*)
 */
#define furi_assert(...) \
    M_APPLY(__furi_assert, M_DEFAULT_ARGS(2, (__FURI_ASSERT_MESSAGE_FLAG), __VA_ARGS__))

#define furi_break(__e)             \
    do {                            \
        if(!(__e)) {                \
            asm volatile("bkpt 0"); \
        }                           \
    } while(0)

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/common_defines.h">
<content>
#pragma once

#include "core_defines.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#define FURI_NORETURN [[noreturn]]
#else
#include <stdnoreturn.h>
#define FURI_NORETURN noreturn
#endif

#include <cmsis_compiler.h>

#ifndef FURI_WARN_UNUSED
#define FURI_WARN_UNUSED __attribute__((warn_unused_result))
#endif

#ifndef FURI_DEPRECATED
#define FURI_DEPRECATED __attribute__((deprecated))
#endif

#ifndef FURI_WEAK
#define FURI_WEAK __attribute__((weak))
#endif

#ifndef FURI_PACKED
#define FURI_PACKED __attribute__((packed))
#endif

#ifndef FURI_ALWAYS_INLINE
#define FURI_ALWAYS_INLINE __attribute__((always_inline)) inline
#endif

#ifndef FURI_IS_IRQ_MASKED
#define FURI_IS_IRQ_MASKED() (__get_PRIMASK() != 0U)
#endif

#ifndef FURI_IS_IRQ_MODE
#define FURI_IS_IRQ_MODE() (__get_IPSR() != 0U)
#endif

#ifndef FURI_IS_ISR
#define FURI_IS_ISR() (FURI_IS_IRQ_MODE() || FURI_IS_IRQ_MASKED())
#endif

typedef struct {
    uint32_t isrm;
    bool from_isr;
    bool kernel_running;
} __FuriCriticalInfo;

__FuriCriticalInfo __furi_critical_enter(void);

void __furi_critical_exit(__FuriCriticalInfo info);

#ifndef FURI_CRITICAL_ENTER
#define FURI_CRITICAL_ENTER() __FuriCriticalInfo __furi_critical_info = __furi_critical_enter();
#endif

#ifndef FURI_CRITICAL_EXIT
#define FURI_CRITICAL_EXIT() __furi_critical_exit(__furi_critical_info);
#endif

#ifndef FURI_CHECK_RETURN
#define FURI_CHECK_RETURN __attribute__((__warn_unused_result__))
#endif

#ifndef FURI_NAKED
#define FURI_NAKED __attribute__((naked))
#endif

#ifndef FURI_DEFAULT
#define FURI_DEFAULT(x) __attribute__((weak, alias(x)))
#endif

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/core_defines.h">
<content>
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_RETURNS_NONNULL __attribute__((returns_nonnull))

#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif

#ifndef ABS
#define ABS(a) ({ (a) < 0 ? -(a) : (a); })
#endif

#ifndef ROUND_UP_TO
#define ROUND_UP_TO(a, b)       \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a / _b + !!(_a % _b);  \
    })
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

#ifndef COUNT_OF
#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef FURI_SWAP
#define FURI_SWAP(x, y)     \
    do {                    \
        typeof(x) SWAP = x; \
        x = y;              \
        y = SWAP;           \
    } while(0)
#endif

#ifndef PLACE_IN_SECTION
#define PLACE_IN_SECTION(x) __attribute__((section(x)))
#endif

#ifndef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifndef UNUSED
#define UNUSED(X) (void)(X)
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#ifndef TOSTRING
#define TOSTRING(x) STRINGIFY(x)
#endif

#ifndef CONCATENATE
#define CONCATENATE(a, b)  CONCATENATE_(a, b)
#define CONCATENATE_(a, b) a##b
#endif

#ifndef REVERSE_BYTES_U32
#define REVERSE_BYTES_U32(x)                                                              \
    ((((x) & 0x000000FF) << 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8) | \
     (((x) & 0xFF000000) >> 24))
#endif

#ifndef FURI_BIT
#define FURI_BIT(x, n) (((x) >> (n)) & 1)
#endif

#ifndef FURI_BIT_SET
#define FURI_BIT_SET(x, n)      \
    ({                          \
        __typeof__(x) _x = (1); \
        (x) |= (_x << (n));     \
    })
#endif

#ifndef FURI_BIT_CLEAR
#define FURI_BIT_CLEAR(x, n)    \
    ({                          \
        __typeof__(x) _x = (1); \
        (x) &= ~(_x << (n));    \
    })
#endif

#define FURI_SW_MEMBARRIER() asm volatile("" : : : "memory")

#ifdef __cplusplus
}
#endif

</content>
</file>


# Thread Management Context
Threading subsystem containing:
- Thread creation and management (furi_thread_*)
- Thread priorities (FuriThreadPriority*)
- Thread list management
Implements the cooperative threading model of Flipper OS.

---

<file name="furi/core/thread.c">
<content>
#include "thread.h"
#include "thread_list.h"
#include "kernel.h"
#include "memmgr.h"
#include "memmgr_heap.h"
#include "check.h"
#include "common_defines.h"
#include "string.h"

#include "log.h"
#include <furi_hal_rtc.h>

#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>

#include <task_control_block.h>

#define TAG "FuriThread"

#define THREAD_NOTIFY_INDEX (1) // Index 0 is used for stream buffers

#define THREAD_MAX_STACK_SIZE (UINT16_MAX * sizeof(StackType_t))

typedef struct FuriThreadStdout FuriThreadStdout;

struct FuriThreadStdout {
    FuriThreadStdoutWriteCallback write_callback;
    FuriString* buffer;
};

struct FuriThread {
    StaticTask_t container;
    StackType_t* stack_buffer;

    FuriThreadState state;
    int32_t ret;

    FuriThreadCallback callback;
    void* context;

    FuriThreadStateCallback state_callback;
    void* state_context;

    FuriThreadSignalCallback signal_callback;
    void* signal_context;

    char* name;
    char* appid;

    FuriThreadPriority priority;

    size_t stack_size;
    size_t heap_size;

    FuriThreadStdout output;

    // Keep all non-alignable byte types in one place,
    // this ensures that the size of this structure is minimal
    bool is_service;
    bool heap_trace_enabled;
    volatile bool is_active;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriThread, container) == 0);

static size_t __furi_thread_stdout_write(FuriThread* thread, const char* data, size_t size);
static int32_t __furi_thread_stdout_flush(FuriThread* thread);

/** Catch threads that are trying to exit wrong way */
__attribute__((__noreturn__)) void furi_thread_catch(void) { //-V1082
    // If you're here it means you're probably doing something wrong
    // with critical sections or with scheduler state
    asm volatile("nop"); // extra magic
    furi_crash("You are doing it wrong"); //-V779
    __builtin_unreachable();
}

static void furi_thread_set_state(FuriThread* thread, FuriThreadState state) {
    furi_assert(thread);
    thread->state = state;
    if(thread->state_callback) {
        thread->state_callback(state, thread->state_context);
    }
}

static void furi_thread_body(void* context) {
    furi_check(context);
    FuriThread* thread = context;

    // store thread instance to thread local storage
    furi_check(pvTaskGetThreadLocalStoragePointer(NULL, 0) == NULL);
    vTaskSetThreadLocalStoragePointer(NULL, 0, thread);

    furi_check(thread->state == FuriThreadStateStarting);
    furi_thread_set_state(thread, FuriThreadStateRunning);

    if(thread->heap_trace_enabled == true) {
        memmgr_heap_enable_thread_trace((FuriThreadId)thread);
    }

    thread->ret = thread->callback(thread->context);

    furi_check(!thread->is_service, "Service threads MUST NOT return");

    if(thread->heap_trace_enabled == true) {
        furi_delay_ms(33);
        thread->heap_size = memmgr_heap_get_thread_memory((FuriThreadId)thread);
        furi_log_print_format(
            thread->heap_size ? FuriLogLevelError : FuriLogLevelInfo,
            TAG,
            "%s allocation balance: %zu",
            thread->name ? thread->name : "Thread",
            thread->heap_size);
        memmgr_heap_disable_thread_trace((FuriThreadId)thread);
    }

    furi_check(thread->state == FuriThreadStateRunning);

    // flush stdout
    __furi_thread_stdout_flush(thread);

    furi_thread_set_state(thread, FuriThreadStateStopped);

    vTaskDelete(NULL);
    furi_thread_catch();
}

static void furi_thread_init_common(FuriThread* thread) {
    thread->output.buffer = furi_string_alloc();

    FuriThread* parent = NULL;
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        // TLS is not available, if we called not from thread context
        parent = pvTaskGetThreadLocalStoragePointer(NULL, 0);

        if(parent && parent->appid) {
            furi_thread_set_appid(thread, parent->appid);
        } else {
            furi_thread_set_appid(thread, "unknown");
        }
    } else {
        // if scheduler is not started, we are starting driver thread
        furi_thread_set_appid(thread, "driver");
    }

    FuriHalRtcHeapTrackMode mode = furi_hal_rtc_get_heap_track_mode();
    if(mode == FuriHalRtcHeapTrackModeAll) {
        thread->heap_trace_enabled = true;
    } else if(mode == FuriHalRtcHeapTrackModeTree && furi_thread_get_current_id()) {
        if(parent) thread->heap_trace_enabled = parent->heap_trace_enabled;
    } else {
        thread->heap_trace_enabled = false;
    }
}

FuriThread* furi_thread_alloc(void) {
    FuriThread* thread = malloc(sizeof(FuriThread));

    furi_thread_init_common(thread);

    return thread;
}

FuriThread* furi_thread_alloc_service(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context) {
    FuriThread* thread = memmgr_alloc_from_pool(sizeof(FuriThread));

    furi_thread_init_common(thread);

    thread->stack_buffer = memmgr_alloc_from_pool(stack_size);
    thread->stack_size = stack_size;
    thread->is_service = true;

    furi_thread_set_name(thread, name);
    furi_thread_set_callback(thread, callback);
    furi_thread_set_context(thread, context);

    return thread;
}

FuriThread* furi_thread_alloc_ex(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context) {
    FuriThread* thread = furi_thread_alloc();
    furi_thread_set_name(thread, name);
    furi_thread_set_stack_size(thread, stack_size);
    furi_thread_set_callback(thread, callback);
    furi_thread_set_context(thread, context);
    return thread;
}

void furi_thread_free(FuriThread* thread) {
    furi_check(thread);
    // Cannot free a service thread
    furi_check(thread->is_service == false);
    // Cannot free a non-joined thread
    furi_check(thread->state == FuriThreadStateStopped);
    furi_check(!thread->is_active);

    furi_thread_set_name(thread, NULL);
    furi_thread_set_appid(thread, NULL);

    if(thread->stack_buffer) {
        free(thread->stack_buffer);
    }

    furi_string_free(thread->output.buffer);
    free(thread);
}

void furi_thread_set_name(FuriThread* thread, const char* name) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);

    if(thread->name) {
        free(thread->name);
    }

    thread->name = name ? strdup(name) : NULL;
}

void furi_thread_set_appid(FuriThread* thread, const char* appid) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);

    if(thread->appid) {
        free(thread->appid);
    }

    thread->appid = appid ? strdup(appid) : NULL;
}

void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    furi_check(stack_size);
    furi_check(stack_size <= THREAD_MAX_STACK_SIZE);
    furi_check(stack_size % sizeof(StackType_t) == 0);
    // Stack size cannot be configured for a thread that has been marked as a service
    furi_check(thread->is_service == false);

    if(thread->stack_buffer) {
        free(thread->stack_buffer);
    }

    thread->stack_buffer = malloc(stack_size);
    thread->stack_size = stack_size;
}

void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->callback = callback;
}

void furi_thread_set_context(FuriThread* thread, void* context) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->context = context;
}

void furi_thread_set_priority(FuriThread* thread, FuriThreadPriority priority) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    furi_check(priority >= FuriThreadPriorityIdle && priority <= FuriThreadPriorityIsr);
    thread->priority = priority;
}

FuriThreadPriority furi_thread_get_priority(FuriThread* thread) {
    furi_check(thread);
    TaskHandle_t hTask = (TaskHandle_t)thread;
    return (FuriThreadPriority)uxTaskPriorityGet(hTask);
}

void furi_thread_set_current_priority(FuriThreadPriority priority) {
    furi_check(priority <= FuriThreadPriorityIsr);

    UBaseType_t new_priority = priority ? priority : FuriThreadPriorityNormal;
    vTaskPrioritySet(NULL, new_priority);
}

FuriThreadPriority furi_thread_get_current_priority(void) {
    return (FuriThreadPriority)uxTaskPriorityGet(NULL);
}

void furi_thread_set_state_callback(FuriThread* thread, FuriThreadStateCallback callback) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->state_callback = callback;
}

void furi_thread_set_state_context(FuriThread* thread, void* context) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->state_context = context;
}

FuriThreadState furi_thread_get_state(FuriThread* thread) {
    furi_check(thread);
    return thread->state;
}

void furi_thread_set_signal_callback(
    FuriThread* thread,
    FuriThreadSignalCallback callback,
    void* context) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped || thread == furi_thread_get_current());

    thread->signal_callback = callback;
    thread->signal_context = context;
}

FuriThreadSignalCallback furi_thread_get_signal_callback(const FuriThread* thread) {
    furi_check(thread);

    return thread->signal_callback;
}

bool furi_thread_signal(const FuriThread* thread, uint32_t signal, void* arg) {
    furi_check(thread);

    bool is_consumed = false;

    if(thread->signal_callback) {
        is_consumed = thread->signal_callback(signal, arg, thread->signal_context);
    }

    return is_consumed;
}

void furi_thread_start(FuriThread* thread) {
    furi_check(thread);
    furi_check(thread->callback);
    furi_check(thread->state == FuriThreadStateStopped);
    furi_check(thread->stack_size > 0);

    furi_thread_set_state(thread, FuriThreadStateStarting);

    uint32_t stack_depth = thread->stack_size / sizeof(StackType_t);
    UBaseType_t priority = thread->priority ? thread->priority : FuriThreadPriorityNormal;

    thread->is_active = true;

    furi_check(
        xTaskCreateStatic(
            furi_thread_body,
            thread->name,
            stack_depth,
            thread,
            priority,
            thread->stack_buffer,
            &thread->container) == (TaskHandle_t)thread);
}

void furi_thread_cleanup_tcb_event(TaskHandle_t task) {
    FuriThread* thread = pvTaskGetThreadLocalStoragePointer(task, 0);
    if(thread) {
        // clear thread local storage
        vTaskSetThreadLocalStoragePointer(task, 0, NULL);
        furi_check(thread == (FuriThread*)task);
        thread->is_active = false;
    }
}

bool furi_thread_join(FuriThread* thread) {
    furi_check(thread);
    // Cannot join a service thread
    furi_check(!thread->is_service);
    // Cannot join a thread to itself
    furi_check(furi_thread_get_current() != thread);

    // !!! IMPORTANT NOTICE !!!
    //
    // If your thread exited, but your app stuck here: some other thread uses
    // all cpu time, which delays kernel from releasing task handle
    while(thread->is_active) {
        furi_delay_ms(10);
    }

    return true;
}

FuriThreadId furi_thread_get_id(FuriThread* thread) {
    furi_check(thread);
    return (FuriThreadId)thread;
}

void furi_thread_enable_heap_trace(FuriThread* thread) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->heap_trace_enabled = true;
}

void furi_thread_disable_heap_trace(FuriThread* thread) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    thread->heap_trace_enabled = false;
}

size_t furi_thread_get_heap_size(FuriThread* thread) {
    furi_check(thread);
    furi_check(thread->heap_trace_enabled == true);
    return thread->heap_size;
}

int32_t furi_thread_get_return_code(FuriThread* thread) {
    furi_check(thread);
    furi_check(thread->state == FuriThreadStateStopped);
    return thread->ret;
}

FuriThreadId furi_thread_get_current_id(void) {
    return (FuriThreadId)xTaskGetCurrentTaskHandle();
}

FuriThread* furi_thread_get_current(void) {
    FuriThread* thread = pvTaskGetThreadLocalStoragePointer(NULL, 0);
    return thread;
}

void furi_thread_yield(void) {
    furi_check(!FURI_IS_IRQ_MODE());
    taskYIELD();
}

/* Limits */
#define MAX_BITS_TASK_NOTIFY  31U
#define MAX_BITS_EVENT_GROUPS 24U

#define THREAD_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_TASK_NOTIFY) - 1U))
#define EVENT_FLAGS_INVALID_BITS  (~((1UL << MAX_BITS_EVENT_GROUPS) - 1U))

uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    uint32_t rflags;
    BaseType_t yield;

    if((hTask == NULL) || ((flags & THREAD_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        rflags = (uint32_t)FuriStatusError;

        if(FURI_IS_IRQ_MODE()) {
            yield = pdFALSE;

            (void)xTaskNotifyIndexedFromISR(hTask, THREAD_NOTIFY_INDEX, flags, eSetBits, &yield);
            (void)xTaskNotifyAndQueryIndexedFromISR(
                hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags, NULL);

            portYIELD_FROM_ISR(yield);
        } else {
            (void)xTaskNotifyIndexed(hTask, THREAD_NOTIFY_INDEX, flags, eSetBits);
            (void)xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags);
        }
    }
    /* Return flags after setting */
    return rflags;
}

uint32_t furi_thread_flags_clear(uint32_t flags) {
    TaskHandle_t hTask;
    uint32_t rflags, cflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else if((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        hTask = xTaskGetCurrentTaskHandle();

        if(xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &cflags) ==
           pdPASS) {
            rflags = cflags;
            cflags &= ~flags;

            if(xTaskNotifyIndexed(hTask, THREAD_NOTIFY_INDEX, cflags, eSetValueWithOverwrite) !=
               pdPASS) {
                rflags = (uint32_t)FuriStatusError;
            }
        } else {
            rflags = (uint32_t)FuriStatusError;
        }
    }

    /* Return flags before clearing */
    return rflags;
}

uint32_t furi_thread_flags_get(void) {
    TaskHandle_t hTask;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else {
        hTask = xTaskGetCurrentTaskHandle();

        if(xTaskNotifyAndQueryIndexed(hTask, THREAD_NOTIFY_INDEX, 0, eNoAction, &rflags) !=
           pdPASS) {
            rflags = (uint32_t)FuriStatusError;
        }
    }

    return rflags;
}

uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout) {
    uint32_t rflags, nval;
    uint32_t clear;
    TickType_t t0, td, tout;
    BaseType_t rval;

    if(FURI_IS_IRQ_MODE()) {
        rflags = (uint32_t)FuriStatusErrorISR;
    } else if((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)FuriStatusErrorParameter;
    } else {
        if((options & FuriFlagNoClear) == FuriFlagNoClear) {
            clear = 0U;
        } else {
            clear = flags;
        }

        rflags = 0U;
        tout = timeout;

        t0 = xTaskGetTickCount();
        do {
            rval = xTaskNotifyWaitIndexed(THREAD_NOTIFY_INDEX, 0, clear, &nval, tout);

            if(rval == pdPASS) {
                rflags &= flags;
                rflags |= nval;

                if((options & FuriFlagWaitAll) == FuriFlagWaitAll) {
                    if((flags & rflags) == flags) {
                        break;
                    } else {
                        if(timeout == 0U) {
                            rflags = (uint32_t)FuriStatusErrorResource;
                            break;
                        }
                    }
                } else {
                    if((flags & rflags) != 0) {
                        break;
                    } else {
                        if(timeout == 0U) {
                            rflags = (uint32_t)FuriStatusErrorResource;
                            break;
                        }
                    }
                }

                /* Update timeout */
                td = xTaskGetTickCount() - t0;

                if(td > tout) {
                    tout = 0;
                } else {
                    tout -= td;
                }
            } else {
                if(timeout == 0) {
                    rflags = (uint32_t)FuriStatusErrorResource;
                } else {
                    rflags = (uint32_t)FuriStatusErrorTimeout;
                }
            }
        } while(rval != pdFAIL);
    }

    /* Return flags before clearing */
    return rflags;
}

static const char* furi_thread_state_name(eTaskState state) {
    switch(state) {
    case eRunning:
        return "Running";
    case eReady:
        return "Ready";
    case eBlocked:
        return "Blocked";
    case eSuspended:
        return "Suspended";
    case eDeleted:
        return "Deleted";
    case eInvalid:
        return "Invalid";
    default:
        return "?";
    }
}

bool furi_thread_enumerate(FuriThreadList* thread_list) {
    furi_check(thread_list);
    furi_check(!FURI_IS_IRQ_MODE());

    bool result = false;

    vTaskSuspendAll();
    do {
        uint32_t tick = furi_get_tick();
        uint32_t count = uxTaskGetNumberOfTasks();

        TaskStatus_t* task = pvPortMalloc(count * sizeof(TaskStatus_t));

        if(!task) break;

        configRUN_TIME_COUNTER_TYPE total_run_time;
        count = uxTaskGetSystemState(task, count, &total_run_time);
        for(uint32_t i = 0U; i < count; i++) {
            TaskControlBlock* tcb = (TaskControlBlock*)task[i].xHandle;

            FuriThreadListItem* item =
                furi_thread_list_get_or_insert(thread_list, (FuriThread*)task[i].xHandle);

            FuriThreadId thread_id = (FuriThreadId)task[i].xHandle;
            item->thread = (FuriThread*)thread_id;
            item->app_id = furi_thread_get_appid(thread_id);
            item->name = task[i].pcTaskName;
            item->priority = task[i].uxCurrentPriority;
            item->stack_address = (uint32_t)tcb->pxStack;
            size_t thread_heap = memmgr_heap_get_thread_memory(thread_id);
            item->heap = thread_heap == MEMMGR_HEAP_UNKNOWN ? 0u : thread_heap;
            item->stack_size = (tcb->pxEndOfStack - tcb->pxStack + 1) * sizeof(StackType_t);
            item->stack_min_free = furi_thread_get_stack_space(thread_id);
            item->state = furi_thread_state_name(task[i].eCurrentState);
            item->counter_previous = item->counter_current;
            item->counter_current = task[i].ulRunTimeCounter;
            item->tick = tick;
        }

        vPortFree(task);
        furi_thread_list_process(thread_list, total_run_time, tick);

        result = true;
    } while(false);
    (void)xTaskResumeAll();

    return result;
}

const char* furi_thread_get_name(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    const char* name;

    if(FURI_IS_IRQ_MODE() || (hTask == NULL)) {
        name = NULL;
    } else {
        name = pcTaskGetName(hTask);
    }

    return name;
}

const char* furi_thread_get_appid(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    const char* appid = "system";

    if(!FURI_IS_IRQ_MODE() && (hTask != NULL)) {
        FuriThread* thread = (FuriThread*)pvTaskGetThreadLocalStoragePointer(hTask, 0);
        if(thread) {
            appid = thread->appid;
        }
    }

    return appid;
}

uint32_t furi_thread_get_stack_space(FuriThreadId thread_id) {
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    uint32_t sz;

    if(FURI_IS_IRQ_MODE() || (hTask == NULL)) {
        sz = 0U;
    } else {
        sz = (uint32_t)(uxTaskGetStackHighWaterMark(hTask) * sizeof(StackType_t));
    }

    return sz;
}

static size_t __furi_thread_stdout_write(FuriThread* thread, const char* data, size_t size) {
    if(thread->output.write_callback != NULL) {
        thread->output.write_callback(data, size);
    } else {
        furi_log_tx((const uint8_t*)data, size);
    }
    return size;
}

static int32_t __furi_thread_stdout_flush(FuriThread* thread) {
    FuriString* buffer = thread->output.buffer;
    size_t size = furi_string_size(buffer);
    if(size > 0) {
        __furi_thread_stdout_write(thread, furi_string_get_cstr(buffer), size);
        furi_string_reset(buffer);
    }
    return 0;
}

void furi_thread_set_stdout_callback(FuriThreadStdoutWriteCallback callback) {
    FuriThread* thread = furi_thread_get_current();
    furi_check(thread);
    __furi_thread_stdout_flush(thread);
    thread->output.write_callback = callback;
}

FuriThreadStdoutWriteCallback furi_thread_get_stdout_callback(void) {
    FuriThread* thread = furi_thread_get_current();
    furi_check(thread);
    return thread->output.write_callback;
}

size_t furi_thread_stdout_write(const char* data, size_t size) {
    FuriThread* thread = furi_thread_get_current();
    furi_check(thread);

    if(size == 0 || data == NULL) {
        return __furi_thread_stdout_flush(thread);
    } else {
        if(data[size - 1] == '\n') {
            // if the last character is a newline, we can flush buffer and write data as is, wo buffers
            __furi_thread_stdout_flush(thread);
            __furi_thread_stdout_write(thread, data, size);
        } else {
            // string_cat doesn't work here because we need to write the exact size data
            for(size_t i = 0; i < size; i++) {
                furi_string_push_back(thread->output.buffer, data[i]);
                if(data[i] == '\n') {
                    __furi_thread_stdout_flush(thread);
                }
            }
        }
    }

    return size;
}

int32_t furi_thread_stdout_flush(void) {
    FuriThread* thread = furi_thread_get_current();
    furi_check(thread);

    return __furi_thread_stdout_flush(thread);
}

void furi_thread_suspend(FuriThreadId thread_id) {
    furi_check(thread_id);

    TaskHandle_t hTask = (TaskHandle_t)thread_id;

    vTaskSuspend(hTask);
}

void furi_thread_resume(FuriThreadId thread_id) {
    furi_check(thread_id);

    TaskHandle_t hTask = (TaskHandle_t)thread_id;

    if(FURI_IS_IRQ_MODE()) {
        xTaskResumeFromISR(hTask);
    } else {
        vTaskResume(hTask);
    }
}

bool furi_thread_is_suspended(FuriThreadId thread_id) {
    furi_check(thread_id);

    TaskHandle_t hTask = (TaskHandle_t)thread_id;

    return eTaskGetState(hTask) == eSuspended;
}

</content>
</file>
<file name="furi/core/thread.h">
<content>
/**
 * @file thread.h
 * @brief Furi: Furi Thread API
 */

#pragma once

#include "base.h"
#include "common_defines.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible FuriThread states.
 *
 * Many of the FuriThread functions MUST ONLY be called when the thread is STOPPED.
 */
typedef enum {
    FuriThreadStateStopped, /**< Thread is stopped */
    FuriThreadStateStarting, /**< Thread is starting */
    FuriThreadStateRunning, /**< Thread is running */
} FuriThreadState;

/**
 * @brief Enumeration of possible FuriThread priorities.
 */
typedef enum {
    FuriThreadPriorityNone = 0, /**< Uninitialized, choose system default */
    FuriThreadPriorityIdle = 1, /**< Idle priority */
    FuriThreadPriorityLowest = 14, /**< Lowest */
    FuriThreadPriorityLow = 15, /**< Low */
    FuriThreadPriorityNormal = 16, /**< Normal */
    FuriThreadPriorityHigh = 17, /**< High */
    FuriThreadPriorityHighest = 18, /**< Highest */
    FuriThreadPriorityIsr =
        (FURI_CONFIG_THREAD_MAX_PRIORITIES - 1), /**< Deferred ISR (highest possible) */
} FuriThreadPriority;

/**
 * @brief FuriThread opaque type.
 */
typedef struct FuriThread FuriThread;

/** FuriThreadList type */
typedef struct FuriThreadList FuriThreadList;

/**
 * @brief Unique thread identifier type (used by the OS kernel).
 */
typedef void* FuriThreadId;

/**
 * @brief Thread callback function pointer type.
 *
 * The function to be used as a thread callback MUST follow this signature.
 *
 * @param[in,out] context pointer to a user-specified object
 * @return value to be used as the thread return code
 */
typedef int32_t (*FuriThreadCallback)(void* context);

/**
 * @brief Standard output callback function pointer type.
 *
 * The function to be used as a standard output callback MUST follow this signature.
 *
 * @warning The handler MUST process ALL of the provided data before returning.
 *
 * @param[in] data pointer to the data to be written to the standard out
 * @param[in] size size of the data in bytes
 */
typedef void (*FuriThreadStdoutWriteCallback)(const char* data, size_t size);

/**
 * @brief State change callback function pointer type.
 *
 * The function to be used as a state callback MUST follow this signature.
 *
 * @param[in] state identifier of the state the thread has transitioned to
 * @param[in,out] context pointer to a user-specified object
 */
typedef void (*FuriThreadStateCallback)(FuriThreadState state, void* context);

/**
 * @brief Signal handler callback function pointer type.
 *
 * The function to be used as a signal handler callback MUS follow this signature.
 *
 * @param[in] signal value of the signal to be handled by the recipient
 * @param[in,out] arg optional argument (can be of any value, including NULL)
 * @param[in,out] context pointer to a user-specified object
 * @returns true if the signal was handled, false otherwise
 */
typedef bool (*FuriThreadSignalCallback)(uint32_t signal, void* arg, void* context);

/**
 * @brief Create a FuriThread instance.
 *
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc(void);

/**
 * @brief Create a FuriThread instance (service mode).
 *
 * Service threads are more memory efficient, but have
 * the following limitations:
 *
 * - Cannot return from the callback
 * - Cannot be joined or freed
 * - Stack size cannot be altered
 *
 * @param[in] name human-readable thread name (can be NULL)
 * @param[in] stack_size stack size in bytes (cannot be changed later)
 * @param[in] callback pointer to a function to be executed in this thread
 * @param[in] context pointer to a user-specified object (will be passed to the callback)
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc_service(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context);

/**
 * @brief Create a FuriThread instance w/ extra parameters.
 * 
 * @param[in] name human-readable thread name (can be NULL)
 * @param[in] stack_size stack size in bytes (can be changed later)
 * @param[in] callback pointer to a function to be executed in this thread
 * @param[in] context pointer to a user-specified object (will be passed to the callback)
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc_ex(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context);

/**
 * @brief Delete a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @warning see furi_thread_join for caveats on stopping a thread.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be deleted
 */
void furi_thread_free(FuriThread* thread);

/**
 * @brief Set the name of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] name human-readable thread name (can be NULL)
 */
void furi_thread_set_name(FuriThread* thread, const char* name);

/**
 * @brief Set the application ID of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * Technically, it is like a "process id", but it is not a system-wide unique identifier.
 * All threads spawned by the same app will have the same appid.
 * 
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] appid thread application ID (can be NULL)
 */
void furi_thread_set_appid(FuriThread* thread, const char* appid);

/**
 * @brief Set the stack size of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function. Additionally, it is NOT possible
 * to change the stack size of a service thread under any circumstances.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] stack_size stack size in bytes
 */
void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size);

/**
 * @brief Set the user callback function to be executed in a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] callback pointer to a user-specified function to be executed in this thread
 */
void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback);

/**
 * @brief Set the callback function context.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] context pointer to a user-specified object (will be passed to the callback, can be NULL)
 */
void furi_thread_set_context(FuriThread* thread, void* context);

/**
 * @brief Set the priority of a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] priority priority level value
 */
void furi_thread_set_priority(FuriThread* thread, FuriThreadPriority priority);

/**
 * @brief Get the priority of a FuriThread.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return priority level value
 */
FuriThreadPriority furi_thread_get_priority(FuriThread* thread);

/**
 * @brief Set the priority of the current FuriThread.
 *
 * @param priority priority level value
 */
void furi_thread_set_current_priority(FuriThreadPriority priority);

/**
 * @brief Get the priority of the current FuriThread.
 *
 * @return priority level value
 */
FuriThreadPriority furi_thread_get_current_priority(void);

/**
 * Set the callback function to be executed upon a state thransition of a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] callback pointer to a user-specified callback function
 */
void furi_thread_set_state_callback(FuriThread* thread, FuriThreadStateCallback callback);

/**
 * @brief Set the state change callback context.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] context pointer to a user-specified object (will be passed to the callback, can be NULL)
 */
void furi_thread_set_state_context(FuriThread* thread, void* context);

/**
 * @brief Get the state of a FuriThread isntance.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return thread state value
 */
FuriThreadState furi_thread_get_state(FuriThread* thread);

/**
 * @brief Set a signal handler callback for a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function if calling it from another thread.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] callback pointer to a user-specified callback function
 * @param[in] context pointer to a user-specified object (will be passed to the callback, can be NULL)
 */
void furi_thread_set_signal_callback(
    FuriThread* thread,
    FuriThreadSignalCallback callback,
    void* context);

/**
 * @brief Get a signal callback for a FuriThread instance.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return pointer to the callback function or NULL if none has been set
 */
FuriThreadSignalCallback furi_thread_get_signal_callback(const FuriThread* thread);

/**
 * @brief Send a signal to a FuriThread instance.
 *
 * @param[in] thread pointer to the FuriThread instance to be signaled
 * @param[in] signal signal value to be sent
 * @param[in,out] arg optional argument (can be of any value, including NULL)
 */
bool furi_thread_signal(const FuriThread* thread, uint32_t signal, void* arg);

/**
 * @brief Start a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be started
 */
void furi_thread_start(FuriThread* thread);

/**
 * @brief Wait for a FuriThread to exit.
 *
 * The thread callback function must return in order for the FuriThread instance to become joinable.
 *
 * @warning Use this method only when the CPU is not busy (i.e. when the
 *          Idle task receives control), otherwise it will wait forever.
 *
 * @param[in] thread pointer to the FuriThread instance to be joined
 * @return always true
 */
bool furi_thread_join(FuriThread* thread);

/**
 * @brief Get the unique identifier of a FuriThread instance.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return unique identifier value or NULL if thread is not running
 */
FuriThreadId furi_thread_get_id(FuriThread* thread);

/**
 * @brief Enable heap usage tracing for a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 */
void furi_thread_enable_heap_trace(FuriThread* thread);

/**
 * @brief Disable heap usage tracing for a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 */
void furi_thread_disable_heap_trace(FuriThread* thread);

/**
 * @brief Get heap usage by a FuriThread instance.
 *
 * The heap trace MUST be enabled before callgin this function.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return heap usage in bytes
 */
size_t furi_thread_get_heap_size(FuriThread* thread);

/**
 * @brief Get the return code of a FuriThread instance.
 *
 * This value is equal to the return value of the thread callback function.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return return code value
 */
int32_t furi_thread_get_return_code(FuriThread* thread);

/**
 * @brief Get the unique identifier of the current FuriThread.
 *
 * @return unique identifier value
 */
FuriThreadId furi_thread_get_current_id(void);

/**
 * @brief Get the FuriThread instance associated with the current thread.
 * 
 * @return pointer to a FuriThread instance or NULL if this thread does not belong to Furi
 */
FuriThread* furi_thread_get_current(void);

/**
 * @brief Return control to the scheduler.
 */
void furi_thread_yield(void);

/**
 * @brief Set the thread flags of a FuriThread.
 *
 * Can be used as a simple inter-thread communication mechanism.
 *
 * @param[in] thread_id unique identifier of the thread to be notified
 * @param[in] flags bitmask of thread flags to set
 * @return bitmask combination of previous and newly set flags
 */
uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags);

/**
 * @brief Clear the thread flags of the current FuriThread.
 *
 * @param[in] flags bitmask of thread flags to clear
 * @return bitmask of thread flags before clearing
 */
uint32_t furi_thread_flags_clear(uint32_t flags);

/**
 * @brief Get the thread flags of the current FuriThread.
 * @return current bitmask of thread flags
 */
uint32_t furi_thread_flags_get(void);

/**
 * @brief Wait for some thread flags to be set.
 *
 * @see FuriFlag for option and error flags.
 *
 * @param[in] flags bitmask of thread flags to wait for
 * @param[in] options combination of option flags determining the behavior of the function
 * @param[in] timeout maximum time to wait in milliseconds (use FuriWaitForever to wait forever)
 * @return bitmask combination of received thread and error flags
 */
uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout);

/**
 * @brief      Enumerate all threads.
 *
 * @param[out] thread_list  pointer to the FuriThreadList container
 *
 * @return     true on success, false otherwise
 */
bool furi_thread_enumerate(FuriThreadList* thread_list);

/**
 * @brief Get the name of a thread based on its unique identifier.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return pointer to a zero-terminated string or NULL
 */
const char* furi_thread_get_name(FuriThreadId thread_id);

/**
 * @brief Get the application id of a thread based on its unique identifier.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return pointer to a zero-terminated string
 */
const char* furi_thread_get_appid(FuriThreadId thread_id);

/**
 * @brief Get thread stack watermark.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return stack watermark value
 */
uint32_t furi_thread_get_stack_space(FuriThreadId thread_id);

/**
 * @brief Get the standard output callback for the current thead.
 *
 * @return pointer to the standard out callback function
 */
FuriThreadStdoutWriteCallback furi_thread_get_stdout_callback(void);

/** Set standard output callback for the current thread.
 *
 * @param[in] callback pointer to the callback function or NULL to clear
 */
void furi_thread_set_stdout_callback(FuriThreadStdoutWriteCallback callback);

/** Write data to buffered standard output.
 * 
 * @param[in] data pointer to the data to be written
 * @param[in] size data size in bytes
 * @return number of bytes that was actually written
 */
size_t furi_thread_stdout_write(const char* data, size_t size);

/**
 * @brief Flush buffered data to standard output.
 * 
 * @return error code value
 */
int32_t furi_thread_stdout_flush(void);

/**
 * @brief Suspend a thread.
 *
 * Suspended threads are no more receiving any of the processor time.
 * 
 * @param[in] thread_id unique identifier of the thread to be suspended
 */
void furi_thread_suspend(FuriThreadId thread_id);

/**
 * @brief Resume a thread.
 * 
 * @param[in] thread_id unique identifier of the thread to be resumed
 */
void furi_thread_resume(FuriThreadId thread_id);

/**
 * @brief Test if a thread is suspended.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return true if thread is suspended, false otherwise
 */
bool furi_thread_is_suspended(FuriThreadId thread_id);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/thread_list.c">
<content>
#include "thread_list.h"
#include "check.h"

#include <furi_hal_interrupt.h>

#include <m-array.h>
#include <m-dict.h>

ARRAY_DEF(FuriThreadListItemArray, FuriThreadListItem*, M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemArray_t() ARRAY_OPLIST(FuriThreadListItemArray, M_PTR_OPLIST)

DICT_DEF2(
    FuriThreadListItemDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    FuriThreadListItem*,
    M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemDict_t() \
    DICT_OPLIST(FuriThreadListItemDict, M_DEFAULT_OPLIST, M_PTR_OPLIST)

struct FuriThreadList {
    FuriThreadListItemArray_t items;
    FuriThreadListItemDict_t search;
    uint32_t runtime_previous;
    uint32_t runtime_current;
    uint32_t isr_previous;
    uint32_t isr_current;
};

FuriThreadList* furi_thread_list_alloc(void) {
    FuriThreadList* instance = malloc(sizeof(FuriThreadList));

    FuriThreadListItemArray_init(instance->items);
    FuriThreadListItemDict_init(instance->search);

    return instance;
}

void furi_thread_list_free(FuriThreadList* instance) {
    furi_check(instance);

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        free(item);
        FuriThreadListItemArray_next(it);
    }

    FuriThreadListItemDict_clear(instance->search);
    FuriThreadListItemArray_clear(instance->items);

    free(instance);
}

size_t furi_thread_list_size(FuriThreadList* instance) {
    furi_check(instance);
    return FuriThreadListItemArray_size(instance->items);
}

FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position) {
    furi_check(instance);
    furi_check(position < furi_thread_list_size(instance));

    return *FuriThreadListItemArray_get(instance->items, position);
}

FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread) {
    furi_check(instance);

    FuriThreadListItem** item_ptr = FuriThreadListItemDict_get(instance->search, (uint32_t)thread);
    if(item_ptr) {
        return *item_ptr;
    }

    FuriThreadListItem* item = malloc(sizeof(FuriThreadListItem));

    FuriThreadListItemArray_push_back(instance->items, item);
    FuriThreadListItemDict_set_at(instance->search, (uint32_t)thread, item);

    return item;
}

void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick) {
    furi_check(instance);

    instance->runtime_previous = instance->runtime_current;
    instance->runtime_current = runtime;

    instance->isr_previous = instance->isr_current;
    instance->isr_current = furi_hal_interrupt_get_time_in_isr_total();

    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        if(item->tick != tick) {
            FuriThreadListItemArray_remove(instance->items, it);
            (void)FuriThreadListItemDict_erase(instance->search, (uint32_t)item->thread);
            free(item);
        } else {
            uint32_t item_counter = item->counter_current - item->counter_previous;
            if(item_counter && item->counter_previous && item->counter_current) {
                item->cpu = (float)item_counter / (float)runtime_counter * 100.0f;
                if(item->cpu > 200.0f) item->cpu = 0.0f;
            } else {
                item->cpu = 0.0f;
            }

            FuriThreadListItemArray_next(it);
        }
    }
}

float furi_thread_list_get_isr_time(FuriThreadList* instance) {
    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;
    const uint32_t isr_counter = instance->isr_current - instance->isr_previous;

    return (float)isr_counter / (float)runtime_counter;
}

</content>
</file>
<file name="furi/core/thread_list.h">
<content>
#pragma once

#include "base.h"
#include "common_defines.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriThread* thread; /**< Pointer to FuriThread, valid while it is running */
    const char* app_id; /**< Thread application id, valid while it is running */
    const char* name; /**< Thread name, valid while it is running */
    FuriThreadPriority priority; /**< Thread priority */
    uint32_t stack_address; /**< Thread stack address */
    size_t heap; /**< Thread heap size if tracking enabled, 0 - otherwise */
    uint32_t stack_size; /**< Thread stack size */
    uint32_t stack_min_free; /**< Thread minimum of the stack size ever reached */
    const char*
        state; /**< Thread state, can be: "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid" */
    float cpu; /**< Thread CPU usage time in percents (including interrupts happened while running) */

    // Service variables
    uint32_t counter_previous; /**< Thread previous runtime counter */
    uint32_t counter_current; /**< Thread current runtime counter */
    uint32_t tick; /**< Thread last seen tick */
} FuriThreadListItem;

/** Anonymous FuriThreadList type */
typedef struct FuriThreadList FuriThreadList;

/** Allocate FuriThreadList instance
 *
 * @return     FuriThreadList instance
 */
FuriThreadList* furi_thread_list_alloc(void);

/** Free FuriThreadList instance
 *
 * @param      instance  The FuriThreadList instance to free
 */
void furi_thread_list_free(FuriThreadList* instance);

/** Get FuriThreadList instance size
 *
 * @param      instance  The instance
 *
 * @return     Item count
 */
size_t furi_thread_list_size(FuriThreadList* instance);

/** Get item at position
 *
 * @param      instance  The FuriThreadList instance
 * @param[in]  position  The position of the item
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position);

/** Get item by thread FuriThread pointer
 *
 * @param      instance  The FuriThreadList instance
 * @param      thread    The FuriThread pointer
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread);

/** Process items in the FuriThreadList instance
 *
 * @param      instance  The instance
 * @param[in]  runtime   The runtime of the system since start
 * @param[in]  tick      The tick when processing happened
 */
void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick);

/** Get percent of time spent in ISR
 *
 * @param      instance  The instance
 *
 * @return     percent of time spent in ISR
 */
float furi_thread_list_get_isr_time(FuriThreadList* instance);

#ifdef __cplusplus
}
#endif

</content>
</file>

<file name="furi/core/thread_list.c">
<content>
#include "thread_list.h"
#include "check.h"

#include <furi_hal_interrupt.h>

#include <m-array.h>
#include <m-dict.h>

ARRAY_DEF(FuriThreadListItemArray, FuriThreadListItem*, M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemArray_t() ARRAY_OPLIST(FuriThreadListItemArray, M_PTR_OPLIST)

DICT_DEF2(
    FuriThreadListItemDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    FuriThreadListItem*,
    M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemDict_t() \
    DICT_OPLIST(FuriThreadListItemDict, M_DEFAULT_OPLIST, M_PTR_OPLIST)

struct FuriThreadList {
    FuriThreadListItemArray_t items;
    FuriThreadListItemDict_t search;
    uint32_t runtime_previous;
    uint32_t runtime_current;
    uint32_t isr_previous;
    uint32_t isr_current;
};

FuriThreadList* furi_thread_list_alloc(void) {
    FuriThreadList* instance = malloc(sizeof(FuriThreadList));

    FuriThreadListItemArray_init(instance->items);
    FuriThreadListItemDict_init(instance->search);

    return instance;
}

void furi_thread_list_free(FuriThreadList* instance) {
    furi_check(instance);

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        free(item);
        FuriThreadListItemArray_next(it);
    }

    FuriThreadListItemDict_clear(instance->search);
    FuriThreadListItemArray_clear(instance->items);

    free(instance);
}

size_t furi_thread_list_size(FuriThreadList* instance) {
    furi_check(instance);
    return FuriThreadListItemArray_size(instance->items);
}

FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position) {
    furi_check(instance);
    furi_check(position < furi_thread_list_size(instance));

    return *FuriThreadListItemArray_get(instance->items, position);
}

FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread) {
    furi_check(instance);

    FuriThreadListItem** item_ptr = FuriThreadListItemDict_get(instance->search, (uint32_t)thread);
    if(item_ptr) {
        return *item_ptr;
    }

    FuriThreadListItem* item = malloc(sizeof(FuriThreadListItem));

    FuriThreadListItemArray_push_back(instance->items, item);
    FuriThreadListItemDict_set_at(instance->search, (uint32_t)thread, item);

    return item;
}

void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick) {
    furi_check(instance);

    instance->runtime_previous = instance->runtime_current;
    instance->runtime_current = runtime;

    instance->isr_previous = instance->isr_current;
    instance->isr_current = furi_hal_interrupt_get_time_in_isr_total();

    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        if(item->tick != tick) {
            FuriThreadListItemArray_remove(instance->items, it);
            (void)FuriThreadListItemDict_erase(instance->search, (uint32_t)item->thread);
            free(item);
        } else {
            uint32_t item_counter = item->counter_current - item->counter_previous;
            if(item_counter && item->counter_previous && item->counter_current) {
                item->cpu = (float)item_counter / (float)runtime_counter * 100.0f;
                if(item->cpu > 200.0f) item->cpu = 0.0f;
            } else {
                item->cpu = 0.0f;
            }

            FuriThreadListItemArray_next(it);
        }
    }
}

float furi_thread_list_get_isr_time(FuriThreadList* instance) {
    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;
    const uint32_t isr_counter = instance->isr_current - instance->isr_previous;

    return (float)isr_counter / (float)runtime_counter;
}

</content>
</file>
<file name="furi/core/thread_list.h">
<content>
#pragma once

#include "base.h"
#include "common_defines.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriThread* thread; /**< Pointer to FuriThread, valid while it is running */
    const char* app_id; /**< Thread application id, valid while it is running */
    const char* name; /**< Thread name, valid while it is running */
    FuriThreadPriority priority; /**< Thread priority */
    uint32_t stack_address; /**< Thread stack address */
    size_t heap; /**< Thread heap size if tracking enabled, 0 - otherwise */
    uint32_t stack_size; /**< Thread stack size */
    uint32_t stack_min_free; /**< Thread minimum of the stack size ever reached */
    const char*
        state; /**< Thread state, can be: "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid" */
    float cpu; /**< Thread CPU usage time in percents (including interrupts happened while running) */

    // Service variables
    uint32_t counter_previous; /**< Thread previous runtime counter */
    uint32_t counter_current; /**< Thread current runtime counter */
    uint32_t tick; /**< Thread last seen tick */
} FuriThreadListItem;

/** Anonymous FuriThreadList type */
typedef struct FuriThreadList FuriThreadList;

/** Allocate FuriThreadList instance
 *
 * @return     FuriThreadList instance
 */
FuriThreadList* furi_thread_list_alloc(void);

/** Free FuriThreadList instance
 *
 * @param      instance  The FuriThreadList instance to free
 */
void furi_thread_list_free(FuriThreadList* instance);

/** Get FuriThreadList instance size
 *
 * @param      instance  The instance
 *
 * @return     Item count
 */
size_t furi_thread_list_size(FuriThreadList* instance);

/** Get item at position
 *
 * @param      instance  The FuriThreadList instance
 * @param[in]  position  The position of the item
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position);

/** Get item by thread FuriThread pointer
 *
 * @param      instance  The FuriThreadList instance
 * @param      thread    The FuriThread pointer
 *
 * @return     The FuriThreadListItem instance
 */
FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread);

/** Process items in the FuriThreadList instance
 *
 * @param      instance  The instance
 * @param[in]  runtime   The runtime of the system since start
 * @param[in]  tick      The tick when processing happened
 */
void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick);

/** Get percent of time spent in ISR
 *
 * @param      instance  The instance
 *
 * @return     percent of time spent in ISR
 */
float furi_thread_list_get_isr_time(FuriThreadList* instance);

#ifdef __cplusplus
}
#endif

</content>
</file>


