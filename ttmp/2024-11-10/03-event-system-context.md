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


# Event System Context
Core event handling system containing:
- Event Loop: Core event processing mechanism
- Event Flags: For event signaling between threads
- Timer System: For both one-shot and periodic timer events (FuriTimer)
Handles asynchronous operations and timing in the Flipper OS.

---

<file name="applications/debug/event_loop_blink_test/event_loop_blink_test.c">
<content>
#include <furi.h>
#include <furi_hal_resources.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>

#include <input/input.h>

#define TAG "EventLoopBlinkTest"

#define TIMER_COUNT (6U)

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* timers[TIMER_COUNT];
} EventLoopBlinkTestApp;

static const GpioPin* blink_gpio_pins[] = {
    &gpio_ext_pa7,
    &gpio_ext_pa6,
    &gpio_ext_pa4,
    &gpio_ext_pb3,
    &gpio_ext_pb2,
    &gpio_ext_pc3,
};

static_assert(COUNT_OF(blink_gpio_pins) == TIMER_COUNT);

static const uint32_t timer_intervals[] = {
    25,
    50,
    100,
    200,
    400,
    800,
};

static_assert(COUNT_OF(timer_intervals) == TIMER_COUNT);

static void blink_gpio_init(void) {
    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeOutputPushPull);
        furi_hal_gpio_write(blink_gpio_pins[i], false);
    }

    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_ext_pc0, false);
}

static void blink_gpio_deinit(void) {
    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_hal_gpio_write(blink_gpio_pins[i], false);
        furi_hal_gpio_init_simple(blink_gpio_pins[i], GpioModeAnalog);
    }

    furi_hal_gpio_write(&gpio_ext_pc0, false);
    furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeAnalog);
}

static void view_port_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    elements_text_box(
        canvas,
        0,
        0,
        canvas_width(canvas),
        canvas_height(canvas),
        AlignCenter,
        AlignCenter,
        "\e#Event Loop Timers Test\e#\n"
        "Press buttons\n"
        "to enable or disable timers\n"
        "\e#Exit\e# = long press \e#Back\e#",
        false);
}

static void view_port_input_callback(InputEvent* input_event, void* context) {
    EventLoopBlinkTestApp* app = context;
    furi_message_queue_put(app->input_queue, input_event, 0);
}

static bool input_queue_callback(FuriEventLoopObject* object, void* context) {
    FuriMessageQueue* queue = object;
    EventLoopBlinkTestApp* app = context;

    InputEvent event;
    FuriStatus status = furi_message_queue_get(queue, &event, 0);
    furi_assert(status == FuriStatusOk);

    if(event.type == InputTypeShort) {
        const size_t timer_idx = event.key;
        furi_assert(timer_idx < TIMER_COUNT);

        FuriEventLoopTimer* timer = app->timers[timer_idx];

        if(furi_event_loop_timer_is_running(timer)) {
            furi_event_loop_timer_stop(timer);
        } else {
            furi_event_loop_timer_restart(timer);
        }

    } else if(event.type == InputTypeLong) {
        if(event.key == InputKeyBack) {
            furi_event_loop_stop(app->event_loop);
        }
    }

    return true;
}

static void blink_timer_callback(void* context) {
    const GpioPin* gpio = blink_gpio_pins[(size_t)context];
    furi_hal_gpio_write(gpio, !furi_hal_gpio_read(gpio));
}

static void event_loop_tick_callback(void* context) {
    UNUSED(context);
    furi_hal_gpio_write(&gpio_ext_pc0, !furi_hal_gpio_read(&gpio_ext_pc0));
}

int32_t event_loop_blink_test_app(void* arg) {
    UNUSED(arg);

    blink_gpio_init();

    EventLoopBlinkTestApp app;

    app.event_loop = furi_event_loop_alloc();
    app.input_queue = furi_message_queue_alloc(3, sizeof(InputEvent));

    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        app.timers[i] = furi_event_loop_timer_alloc(
            app.event_loop, blink_timer_callback, FuriEventLoopTimerTypePeriodic, (void*)i);
        furi_event_loop_timer_start(app.timers[i], timer_intervals[i]);
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, view_port_draw_callback, &app);
    view_port_input_callback_set(view_port, view_port_input_callback, &app);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_event_loop_tick_set(app.event_loop, 500, event_loop_tick_callback, &app);
    furi_event_loop_subscribe_message_queue(
        app.event_loop, app.input_queue, FuriEventLoopEventIn, input_queue_callback, &app);

    furi_event_loop_run(app.event_loop);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(app.event_loop, app.input_queue);
    furi_message_queue_free(app.input_queue);

    for(size_t i = 0; i < TIMER_COUNT; ++i) {
        furi_event_loop_timer_free(app.timers[i]);
    }

    furi_event_loop_free(app.event_loop);

    blink_gpio_deinit();

    return 0;
}

</content>
</file>
<file name="applications/debug/unit_tests/tests/furi/furi_event_loop.c">
<content>
#include "../test.h"
#include <furi.h>
#include <furi_hal.h>

#include <FreeRTOS.h>
#include <task.h>

#define TAG "TestFuriEventLoop"

#define EVENT_LOOP_EVENT_COUNT (256u)

typedef struct {
    FuriMessageQueue* mq;

    FuriEventLoop* producer_event_loop;
    uint32_t producer_counter;

    FuriEventLoop* consumer_event_loop;
    uint32_t consumer_counter;
} TestFuriData;

bool test_furi_event_loop_producer_mq_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriData* data = context;
    furi_check(data->mq == object, "Invalid queue");

    FURI_LOG_I(
        TAG, "producer_mq_callback: %lu %lu", data->producer_counter, data->consumer_counter);

    if(data->producer_counter == EVENT_LOOP_EVENT_COUNT / 2) {
        furi_event_loop_unsubscribe(data->producer_event_loop, data->mq);
        furi_event_loop_subscribe_message_queue(
            data->producer_event_loop,
            data->mq,
            FuriEventLoopEventOut,
            test_furi_event_loop_producer_mq_callback,
            data);
    }

    if(data->producer_counter == EVENT_LOOP_EVENT_COUNT) {
        furi_event_loop_stop(data->producer_event_loop);
        return false;
    }

    data->producer_counter++;
    furi_check(
        furi_message_queue_put(data->mq, &data->producer_counter, 0) == FuriStatusOk,
        "furi_message_queue_put failed");
    furi_delay_us(furi_hal_random_get() % 1000);

    return true;
}

int32_t test_furi_event_loop_producer(void* p) {
    furi_check(p);

    TestFuriData* data = p;

    FURI_LOG_I(TAG, "producer start 1st run");

    data->producer_event_loop = furi_event_loop_alloc();
    furi_event_loop_subscribe_message_queue(
        data->producer_event_loop,
        data->mq,
        FuriEventLoopEventOut,
        test_furi_event_loop_producer_mq_callback,
        data);

    furi_event_loop_run(data->producer_event_loop);

    // 2 EventLoop index, 0xFFFFFFFF - all possible flags, emulate uncleared flags
    xTaskNotifyIndexed(xTaskGetCurrentTaskHandle(), 2, 0xFFFFFFFF, eSetBits);

    furi_event_loop_unsubscribe(data->producer_event_loop, data->mq);
    furi_event_loop_free(data->producer_event_loop);

    FURI_LOG_I(TAG, "producer start 2nd run");

    data->producer_counter = 0;
    data->producer_event_loop = furi_event_loop_alloc();

    furi_event_loop_subscribe_message_queue(
        data->producer_event_loop,
        data->mq,
        FuriEventLoopEventOut,
        test_furi_event_loop_producer_mq_callback,
        data);

    furi_event_loop_run(data->producer_event_loop);

    furi_event_loop_unsubscribe(data->producer_event_loop, data->mq);
    furi_event_loop_free(data->producer_event_loop);

    FURI_LOG_I(TAG, "producer end");

    return 0;
}

bool test_furi_event_loop_consumer_mq_callback(FuriEventLoopObject* object, void* context) {
    furi_check(context);

    TestFuriData* data = context;
    furi_check(data->mq == object);

    furi_delay_us(furi_hal_random_get() % 1000);
    furi_check(furi_message_queue_get(data->mq, &data->consumer_counter, 0) == FuriStatusOk);

    FURI_LOG_I(
        TAG, "consumer_mq_callback: %lu %lu", data->producer_counter, data->consumer_counter);

    if(data->consumer_counter == EVENT_LOOP_EVENT_COUNT / 2) {
        furi_event_loop_unsubscribe(data->consumer_event_loop, data->mq);
        furi_event_loop_subscribe_message_queue(
            data->consumer_event_loop,
            data->mq,
            FuriEventLoopEventIn,
            test_furi_event_loop_consumer_mq_callback,
            data);
    }

    if(data->consumer_counter == EVENT_LOOP_EVENT_COUNT) {
        furi_event_loop_stop(data->consumer_event_loop);
        return false;
    }

    return true;
}

int32_t test_furi_event_loop_consumer(void* p) {
    furi_check(p);

    TestFuriData* data = p;

    FURI_LOG_I(TAG, "consumer start 1st run");

    data->consumer_event_loop = furi_event_loop_alloc();
    furi_event_loop_subscribe_message_queue(
        data->consumer_event_loop,
        data->mq,
        FuriEventLoopEventIn,
        test_furi_event_loop_consumer_mq_callback,
        data);

    furi_event_loop_run(data->consumer_event_loop);

    // 2 EventLoop index, 0xFFFFFFFF - all possible flags, emulate uncleared flags
    xTaskNotifyIndexed(xTaskGetCurrentTaskHandle(), 2, 0xFFFFFFFF, eSetBits);

    furi_event_loop_unsubscribe(data->consumer_event_loop, data->mq);
    furi_event_loop_free(data->consumer_event_loop);

    FURI_LOG_I(TAG, "consumer start 2nd run");

    data->consumer_counter = 0;
    data->consumer_event_loop = furi_event_loop_alloc();
    furi_event_loop_subscribe_message_queue(
        data->consumer_event_loop,
        data->mq,
        FuriEventLoopEventIn,
        test_furi_event_loop_consumer_mq_callback,
        data);

    furi_event_loop_run(data->consumer_event_loop);

    furi_event_loop_unsubscribe(data->consumer_event_loop, data->mq);
    furi_event_loop_free(data->consumer_event_loop);

    FURI_LOG_I(TAG, "consumer end");

    return 0;
}

void test_furi_event_loop(void) {
    TestFuriData data = {};

    data.mq = furi_message_queue_alloc(16, sizeof(uint32_t));

    FuriThread* producer_thread = furi_thread_alloc();
    furi_thread_set_name(producer_thread, "producer_thread");
    furi_thread_set_stack_size(producer_thread, 1 * 1024);
    furi_thread_set_callback(producer_thread, test_furi_event_loop_producer);
    furi_thread_set_context(producer_thread, &data);
    furi_thread_start(producer_thread);

    FuriThread* consumer_thread = furi_thread_alloc();
    furi_thread_set_name(consumer_thread, "consumer_thread");
    furi_thread_set_stack_size(consumer_thread, 1 * 1024);
    furi_thread_set_callback(consumer_thread, test_furi_event_loop_consumer);
    furi_thread_set_context(consumer_thread, &data);
    furi_thread_start(consumer_thread);

    // Wait for thread to complete their tasks
    furi_thread_join(producer_thread);
    furi_thread_join(consumer_thread);

    // The test itself
    mu_assert_int_eq(data.producer_counter, data.consumer_counter);
    mu_assert_int_eq(data.producer_counter, EVENT_LOOP_EVENT_COUNT);

    // Release memory
    furi_thread_free(consumer_thread);
    furi_thread_free(producer_thread);
    furi_message_queue_free(data.mq);
}

</content>
</file>
<file name="applications/examples/example_event_loop/example_event_loop_multi.c">
<content>
/**
 * @file example_event_loop_multi.c
 * @brief Example application that demonstrates multiple primitives used with two FuriEventLoop instances.
 *
 * This application simulates a complex use case of having two concurrent event loops (each one executing in
 * its own thread) using a stream buffer for communication and additional timers and message passing to handle
 * the keypad input. Additionally, it shows how to use thread signals to stop an event loop in another thread.
 * The GUI functionality is there only for the purpose of exclusive access to the input events.
 *
 * The application's functionality consists of the following:
 * - Print keypad key names and types when pressed,
 * - If the Back key is long-pressed, a countdown starts upon completion of which the app exits,
 * - The countdown can be cancelled by long-pressing the Ok button, it also resets the counter,
 * - Blocks of random data are periodically generated in a separate thread,
 * - When ready, the main application thread gets notified and prints the data.
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>

#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMulti"

#define COUNTDOWN_START_VALUE   (5UL)
#define COUNTDOWN_INTERVAL_MS   (1000UL)
#define WORKER_DATA_INTERVAL_MS (1500UL)

#define INPUT_QUEUE_SIZE   (8)
#define STREAM_BUFFER_SIZE (16)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    FuriStreamBuffer* stream_buffer;
} EventLoopMultiAppWorker;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriThread* worker_thread;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* exit_timer;
    FuriStreamBuffer* stream_buffer;
    uint32_t exit_countdown_value;
} EventLoopMultiApp;

/*
 * Worker functions
 */

// This function is executed each time the data is taken out of the stream buffer. It is used to restart the worker timer.
static bool
    event_loop_multi_app_stream_buffer_worker_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    furi_assert(object == worker->stream_buffer);

    FURI_LOG_I(TAG, "Data was removed from buffer");
    // Restart the timer to generate another block of random data.
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);

    return true;
}

// This function is executed when the worker timer expires. The timer will NOT restart automatically
// since it is of one-shot type.
static void event_loop_multi_app_worker_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiAppWorker* worker = context;

    // Generate a block of random data.
    uint8_t data[STREAM_BUFFER_SIZE];
    furi_hal_random_fill_buf(data, sizeof(data));
    // Put the generated data in the stream buffer.
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_send(worker->stream_buffer, &data, sizeof(data), 0) == sizeof(data));
}

static EventLoopMultiAppWorker*
    event_loop_multi_app_worker_alloc(FuriStreamBuffer* stream_buffer) {
    EventLoopMultiAppWorker* worker = malloc(sizeof(EventLoopMultiAppWorker));
    // Create the worker event loop.
    worker->event_loop = furi_event_loop_alloc();
    // Create the timer governing the data generation.
    // It is of one-shot type, i.e. it will not restart automatically upon expiration.
    worker->timer = furi_event_loop_timer_alloc(
        worker->event_loop,
        event_loop_multi_app_worker_timer_callback,
        FuriEventLoopTimerTypeOnce,
        worker);

    // Using the same stream buffer as the main thread (it was already created beforehand).
    worker->stream_buffer = stream_buffer;
    // Notify the worker event loop about data being taken out of the stream buffer.
    furi_event_loop_subscribe_stream_buffer(
        worker->event_loop,
        worker->stream_buffer,
        FuriEventLoopEventOut | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_worker_callback,
        worker);

    return worker;
}

static void event_loop_multi_app_worker_free(EventLoopMultiAppWorker* worker) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(worker->event_loop, worker->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(worker->timer);
    // Now it is okay to delete the event loop.
    furi_event_loop_free(worker->event_loop);

    free(worker);
}

static void event_loop_multi_app_worker_run(EventLoopMultiAppWorker* worker) {
    furi_event_loop_timer_start(worker->timer, WORKER_DATA_INTERVAL_MS);
    furi_event_loop_run(worker->event_loop);
}

// This function is the worker thread body and (obviously) is executed in the worker thread.
static int32_t event_loop_multi_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    // Because an event loop is used, it MUST be created in the thread it will be run in.
    // Therefore, the worker creation and deletion is handled in the worker thread.
    EventLoopMultiAppWorker* worker = event_loop_multi_app_worker_alloc(app->stream_buffer);
    event_loop_multi_app_worker_run(worker);
    event_loop_multi_app_worker_free(worker);

    return 0;
}

/*
 * Main application functions
 */

// This function is executed in the GUI context each time an input event occurs (e.g. the user pressed a key)
static void event_loop_multi_app_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;
    // Pass the event to the the application's input queue
    furi_check(furi_message_queue_put(app->input_queue, event, FuriWaitForever) == FuriStatusOk);
}

// This function is executed each time new data is available in the stream buffer.
static bool
    event_loop_multi_app_stream_buffer_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->stream_buffer);
    // Get the data from the stream buffer
    uint8_t data[STREAM_BUFFER_SIZE];
    // IMPORTANT: No waiting in the event handlers!
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, &data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);

    return true;
}

// This function is executed each time a new message is inserted in the input queue.
static bool event_loop_multi_app_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    furi_assert(object == app->input_queue);

    InputEvent event;
    // IMPORTANT: No waiting in the event handlers!
    furi_check(furi_message_queue_get(app->input_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTypeLong) {
        // The user has long-pressed the Back key, try starting the countdown.
        if(event.key == InputKeyBack) {
            if(!furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually start the countdown
                FURI_LOG_I(TAG, "Starting exit countdown!");
                furi_event_loop_timer_start(app->exit_timer, COUNTDOWN_INTERVAL_MS);

            } else {
                // The countdown is already in progress, print a warning message
                FURI_LOG_W(TAG, "Countdown has already been started");
            }

            // The user has long-pressed the Ok key, try stopping the countdown.
        } else if(event.key == InputKeyOk) {
            if(furi_event_loop_timer_is_running(app->exit_timer)) {
                // Actually cancel the countdown
                FURI_LOG_I(TAG, "Exit countdown cancelled!");
                app->exit_countdown_value = COUNTDOWN_START_VALUE;
                furi_event_loop_timer_stop(app->exit_timer);

            } else {
                // The countdown is not running, print a warning message
                FURI_LOG_W(TAG, "Countdown has not been started yet");
            }

        } else {
            // Not a Back or Ok key, just print its name.
            FURI_LOG_I(TAG, "Long press: %s", input_get_key_name(event.key));
        }

    } else if(event.type == InputTypeShort) {
        // Not a long press, just print the key's name.
        FURI_LOG_I(TAG, "Short press: %s", input_get_key_name(event.key));
    }

    return true;
}

// This function is executed each time the countdown timer expires.
static void event_loop_multi_app_exit_timer_callback(void* context) {
    furi_assert(context);
    EventLoopMultiApp* app = context;

    FURI_LOG_I(TAG, "Exiting in %lu ...", app->exit_countdown_value);

    // If the coundown value has reached 0, exit the application
    if(app->exit_countdown_value == 0) {
        FURI_LOG_I(TAG, "Exiting NOW!");

        // Send a signal to the worker thread to exit.
        // A signal handler that handles FuriSignalExit is already set by default.
        furi_thread_signal(app->worker_thread, FuriSignalExit, NULL);
        // Request the application event loop to stop.
        furi_event_loop_stop(app->event_loop);

        // Otherwise just decrement it and wait for the next time the timer expires.
    } else {
        app->exit_countdown_value -= 1;
    }
}

static EventLoopMultiApp* event_loop_multi_app_alloc(void) {
    EventLoopMultiApp* app = malloc(sizeof(EventLoopMultiApp));
    // Create event loop instances.
    app->event_loop = furi_event_loop_alloc();

    // Create a worker thread instance. The worker event loop will execute inside it.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMultiWorker", 1024, event_loop_multi_app_worker_thread, app);
    // Create a message queue to receive the input events.
    app->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    // Create a stream buffer to receive the generated data.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_SIZE);
    // Create a timer to run the countdown.
    app->exit_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        event_loop_multi_app_exit_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        app);

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    // Start the countdown from this value
    app->exit_countdown_value = COUNTDOWN_START_VALUE;
    // Gain exclusive access to the input events
    view_port_input_callback_set(app->view_port, event_loop_multi_app_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    // Notify the event loop about incoming messages in the queue
    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->input_queue,
        FuriEventLoopEventIn,
        event_loop_multi_app_input_queue_callback,
        app);
    // Notify the event loop about new data in the stream buffer
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        FuriEventLoopEventIn | FuriEventLoopEventFlagEdge,
        event_loop_multi_app_stream_buffer_callback,
        app);

    return app;
}

static void event_loop_multi_app_free(EventLoopMultiApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->input_queue);
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    furi_stream_buffer_free(app->stream_buffer);
    // IMPORTANT: All timers MUST be deleted before deleting the associated event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->exit_timer);
    furi_thread_free(app->worker_thread);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_multi_app_run(EventLoopMultiApp* app) {
    FURI_LOG_I(TAG, "Press keys to see them printed here.");
    FURI_LOG_I(TAG, "Long press \"Back\" to exit after %lu seconds.", COUNTDOWN_START_VALUE);
    FURI_LOG_I(TAG, "Long press \"Ok\" to cancel the countdown.");

    // Start the worker thread
    furi_thread_start(app->worker_thread);
    // Run the application event loop. This call will block until the application is about to exit.
    furi_event_loop_run(app->event_loop);
    // Wait for the worker thread to finish.
    furi_thread_join(app->worker_thread);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_event_loop_multi_app(void* arg) {
    UNUSED(arg);

    EventLoopMultiApp* app = event_loop_multi_app_alloc();
    event_loop_multi_app_run(app);
    event_loop_multi_app_free(app);

    return 0;
}

</content>
</file>
<file name="applications/examples/example_event_loop/example_event_loop_mutex.c">
<content>
/**
 * @file example_event_loop_mutex.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriMutex integration.
 *
 * This application simulates a use case where a time-consuming blocking operation is executed
 * in a separate thread and a mutex is being used for synchronization. The application runs 10 iterations
 * of the above mentioned simulated work and prints the results to the debug output each time, then exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopMutex"

#define WORKER_ITERATION_COUNT (10)
// We are interested in IN events (for the mutex, that means that the mutex has been released),
// using edge trigger mode (reacting only to changes in mutex state) and
// employing one-shot mode to automatically unsubscribe before the event is processed.
#define MUTEX_EVENT_AND_FLAGS \
    (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge | FuriEventLoopEventFlagOnce)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriMutex* worker_mutex;
    uint8_t worker_result;
} EventLoopMutexApp;

// This funciton is being run in a separate thread to simulate lenghty blocking operations
static int32_t event_loop_mutex_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopMutexApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    // Run 10 iterations of simulated work
    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        FURI_LOG_I(TAG, "Doing work ...");
        // Take the mutex so that no-one can access the worker_result variable
        furi_check(furi_mutex_acquire(app->worker_mutex, FuriWaitForever) == FuriStatusOk);
        // Simulate a blocking operation with a random delay between 900 and 1100 ms
        const uint32_t work_time_ms = 900 + furi_hal_random_get() % 200;
        furi_delay_ms(work_time_ms);
        // Simulate a result with a random number between 0 and 255
        app->worker_result = furi_hal_random_get() % 0xFF;

        FURI_LOG_I(TAG, "Work done in %lu ms", work_time_ms);
        // Release the mutex, which will notify the event loop that the result is ready
        furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
        // Return control to the scheduler so that the event loop can take the mutex in its turn
        furi_thread_yield();
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the mutex gets released
static bool event_loop_mutex_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    EventLoopMutexApp* app = context;
    furi_assert(object == app->worker_mutex);

    // Take the mutex so that no-one can access the worker_result variable
    // IMPORTANT: the wait time MUST be 0, i.e. the event loop event callbacks
    // must NOT ever block. If it is possible that the mutex will be taken by
    // others, then the event callback code must take it into account.
    furi_check(furi_mutex_acquire(app->worker_mutex, 0) == FuriStatusOk);
    // Access the worker_result variable and print it.
    FURI_LOG_I(TAG, "Result available! Value: %u", app->worker_result);
    // Release the mutex, enabling the worker thread to continue when it's ready
    furi_check(furi_mutex_release(app->worker_mutex) == FuriStatusOk);
    // Subscribe for the mutex release events again, since we were unsubscribed automatically
    // before processing the event.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);

    return true;
}

static EventLoopMutexApp* event_loop_mutex_app_alloc(void) {
    EventLoopMutexApp* app = malloc(sizeof(EventLoopMutexApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopMutexWorker", 1024, event_loop_mutex_app_worker_thread, app);
    // Create a mutex instance.
    app->worker_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    // Subscribe for the mutex release events.
    // Note that since FuriEventLoopEventFlagOneShot is used, we will be automatically unsubscribed
    // from events before entering the event processing callback. This is necessary in order to not
    // trigger on events caused by releasing the mutex in the callback.
    furi_event_loop_subscribe_mutex(
        app->event_loop,
        app->worker_mutex,
        MUTEX_EVENT_AND_FLAGS,
        event_loop_mutex_app_event_callback,
        app);

    return app;
}

static void event_loop_mutex_app_free(EventLoopMutexApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->worker_mutex);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_mutex_free(app->worker_mutex);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_mutex_app_run(EventLoopMutexApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_mutex_app(void* arg) {
    UNUSED(arg);

    EventLoopMutexApp* app = event_loop_mutex_app_alloc();
    event_loop_mutex_app_run(app);
    event_loop_mutex_app_free(app);

    return 0;
}

</content>
</file>
<file name="applications/examples/example_event_loop/example_event_loop_stream_buffer.c">
<content>
/**
 * @file example_event_loop_stream_buffer.c
 * @brief Example application that demonstrates the FuriEventLoop and FuriStreamBuffer integration.
 *
 * This application simulates a use case where some data data stream comes from a separate thread (or hardware)
 * and a stream buffer is used to act as an intermediate buffer. The worker thread produces 10 iterations of 32
 * bytes of simulated data, and each time when the buffer is half-filled, the data is taken out of it and printed
 * to the debug output. After completing all iterations, the application exits.
 */

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "ExampleEventLoopStreamBuffer"

#define WORKER_ITERATION_COUNT (10)

#define STREAM_BUFFER_SIZE            (32)
#define STREAM_BUFFER_TRIG_LEVEL      (STREAM_BUFFER_SIZE / 2)
#define STREAM_BUFFER_EVENT_AND_FLAGS (FuriEventLoopEventIn | FuriEventLoopEventFlagEdge)

typedef struct {
    FuriEventLoop* event_loop;
    FuriThread* worker_thread;
    FuriStreamBuffer* stream_buffer;
} EventLoopStreamBufferApp;

// This funciton is being run in a separate thread to simulate data coming from a producer thread or some device.
static int32_t event_loop_stream_buffer_app_worker_thread(void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    FURI_LOG_I(TAG, "Worker thread started");

    for(uint32_t i = 0; i < WORKER_ITERATION_COUNT; ++i) {
        // Produce 32 bytes of simulated data.
        for(uint32_t j = 0; j < STREAM_BUFFER_SIZE; ++j) {
            // Simulate incoming data by generating a random byte.
            uint8_t data = furi_hal_random_get() % 0xFF;
            // Put the byte in the buffer. Depending on the use case, it may or may be not acceptable
            // to wait for free space to become available.
            furi_check(
                furi_stream_buffer_send(app->stream_buffer, &data, 1, FuriWaitForever) == 1);
            // Delay between 30 and 50 ms to slow down the output for clarity.
            furi_delay_ms(30 + furi_hal_random_get() % 20);
        }
    }

    FURI_LOG_I(TAG, "All work done, worker thread out!");
    // Request the event loop to stop
    furi_event_loop_stop(app->event_loop);

    return 0;
}

// This function is being run each time when the number of bytes in the buffer is above its trigger level.
static bool
    event_loop_stream_buffer_app_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    EventLoopStreamBufferApp* app = context;

    furi_assert(object == app->stream_buffer);

    // Temporary buffer that can hold at most half of the stream buffer's capacity.
    uint8_t data[STREAM_BUFFER_TRIG_LEVEL];
    // Receive the data. It is guaranteed that the amount of data in the buffer will be equal to
    // or greater than the trigger level, therefore, no waiting delay is necessary.
    furi_check(
        furi_stream_buffer_receive(app->stream_buffer, data, sizeof(data), 0) == sizeof(data));

    // Format the data for printing and print it to the debug output.
    FuriString* tmp_str = furi_string_alloc();
    for(uint32_t i = 0; i < sizeof(data); ++i) {
        furi_string_cat_printf(tmp_str, "%02X ", data[i]);
    }

    FURI_LOG_I(TAG, "Received data: %s", furi_string_get_cstr(tmp_str));
    furi_string_free(tmp_str);

    return true;
}

static EventLoopStreamBufferApp* event_loop_stream_buffer_app_alloc(void) {
    EventLoopStreamBufferApp* app = malloc(sizeof(EventLoopStreamBufferApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a worker thread instance.
    app->worker_thread = furi_thread_alloc_ex(
        "EventLoopStreamBufferWorker", 1024, event_loop_stream_buffer_app_worker_thread, app);
    // Create a stream_buffer instance.
    app->stream_buffer = furi_stream_buffer_alloc(STREAM_BUFFER_SIZE, STREAM_BUFFER_TRIG_LEVEL);
    // Subscribe for the stream buffer IN events in edge triggered mode.
    furi_event_loop_subscribe_stream_buffer(
        app->event_loop,
        app->stream_buffer,
        STREAM_BUFFER_EVENT_AND_FLAGS,
        event_loop_stream_buffer_app_event_callback,
        app);

    return app;
}

static void event_loop_stream_buffer_app_free(EventLoopStreamBufferApp* app) {
    // IMPORTANT: The user code MUST unsubscribe from all events before deleting the event loop.
    // Failure to do so will result in a crash.
    furi_event_loop_unsubscribe(app->event_loop, app->stream_buffer);
    // Delete all instances
    furi_thread_free(app->worker_thread);
    furi_stream_buffer_free(app->stream_buffer);
    furi_event_loop_free(app->event_loop);

    free(app);
}

static void event_loop_stream_buffer_app_run(EventLoopStreamBufferApp* app) {
    furi_thread_start(app->worker_thread);
    furi_event_loop_run(app->event_loop);
    furi_thread_join(app->worker_thread);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_stream_buffer_app(void* arg) {
    UNUSED(arg);

    EventLoopStreamBufferApp* app = event_loop_stream_buffer_app_alloc();
    event_loop_stream_buffer_app_run(app);
    event_loop_stream_buffer_app_free(app);

    return 0;
}

</content>
</file>
<file name="applications/examples/example_event_loop/example_event_loop_timer.c">
<content>
/**
 * @file example_event_loop_timer.c
 * @brief Example application that demonstrates FuriEventLoop's software timer capability.
 *
 * This application prints a countdown from 10 to 0 to the debug output and then exits.
 * Despite only one timer being used in this example for clarity, an event loop instance can have
 * an arbitrary number of independent timers of any type (periodic or one-shot).
 *
 */
#include <furi.h>

#define TAG "ExampleEventLoopTimer"

#define COUNTDOWN_START_VALUE (10)
#define COUNTDOWN_INTERVAL_MS (1000)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t countdown_value;
} EventLoopTimerApp;

// This function is called each time the timer expires (i.e. once per 1000 ms (1s) in this example)
static void event_loop_timer_callback(void* context) {
    furi_assert(context);
    EventLoopTimerApp* app = context;

    // Print the countdown value
    FURI_LOG_I(TAG, "T-00:00:%02lu", app->countdown_value);

    if(app->countdown_value == 0) {
        // If the countdown reached 0, print the final line and stop the event loop
        FURI_LOG_I(TAG, "Blast off to adventure!");
        // After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);

    } else {
        // Decrement the countdown value
        app->countdown_value -= 1;
    }
}

static EventLoopTimerApp* event_loop_timer_app_alloc(void) {
    EventLoopTimerApp* app = malloc(sizeof(EventLoopTimerApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a software timer instance.
    // The timer is bound to the event loop instance and will execute in its context.
    // Here, the timer type is periodic, i.e. it will restart automatically after expiring.
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, event_loop_timer_callback, FuriEventLoopTimerTypePeriodic, app);
    // The countdown value will be tracked in this variable.
    app->countdown_value = COUNTDOWN_START_VALUE;

    return app;
}

static void event_loop_timer_app_free(EventLoopTimerApp* app) {
    // IMPORTANT: All event loop timers MUST be deleted BEFORE deleting the event loop itself.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->timer);
    // With all timers deleted, it's safe to delete the event loop.
    furi_event_loop_free(app->event_loop);
    free(app);
}

static void event_loop_timer_app_run(EventLoopTimerApp* app) {
    FURI_LOG_I(TAG, "All systems go! Prepare for countdown!");

    // Timers can be started either before the event loop is run, or in any
    // callback function called by a running event loop.
    furi_event_loop_timer_start(app->timer, COUNTDOWN_INTERVAL_MS);
    // This call will block until furi_event_loop_stop() is called.
    furi_event_loop_run(app->event_loop);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_timer_app(void* arg) {
    UNUSED(arg);

    EventLoopTimerApp* app = event_loop_timer_app_alloc();
    event_loop_timer_app_run(app);
    event_loop_timer_app_free(app);

    return 0;
}

</content>
</file>
<file name="furi/core/event_flag.c">
<content>
#include "event_flag.h"
#include "common_defines.h"
#include "check.h"

#include <FreeRTOS.h>
#include <event_groups.h>

#define FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS 24U
#define FURI_EVENT_FLAG_INVALID_BITS          (~((1UL << FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS) - 1U))

struct FuriEventFlag {
    StaticEventGroup_t container;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriEventFlag, container) == 0);

FuriEventFlag* furi_event_flag_alloc(void) {
    furi_check(!FURI_IS_IRQ_MODE());

    FuriEventFlag* instance = malloc(sizeof(FuriEventFlag));

    furi_check(xEventGroupCreateStatic(&instance->container) == (EventGroupHandle_t)instance);

    return instance;
}

void furi_event_flag_free(FuriEventFlag* instance) {
    furi_check(!FURI_IS_IRQ_MODE());
    vEventGroupDelete((EventGroupHandle_t)instance);
    free(instance);
}

uint32_t furi_event_flag_set(FuriEventFlag* instance, uint32_t flags) {
    furi_check(instance);
    furi_check((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;
    BaseType_t yield;

    if(FURI_IS_IRQ_MODE()) {
        yield = pdFALSE;
        if(xEventGroupSetBitsFromISR(hEventGroup, (EventBits_t)flags, &yield) == pdFAIL) {
            rflags = (uint32_t)FuriFlagErrorResource;
        } else {
            rflags = flags;
            portYIELD_FROM_ISR(yield);
        }
    } else {
        vTaskSuspendAll();
        rflags = xEventGroupSetBits(hEventGroup, (EventBits_t)flags);
        (void)xTaskResumeAll();
    }

    /* Return event flags after setting */
    return rflags;
}

uint32_t furi_event_flag_clear(FuriEventFlag* instance, uint32_t flags) {
    furi_check(instance);
    furi_check((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = xEventGroupGetBitsFromISR(hEventGroup);

        if(xEventGroupClearBitsFromISR(hEventGroup, (EventBits_t)flags) == pdFAIL) {
            rflags = (uint32_t)FuriStatusErrorResource;
        } else {
            /* xEventGroupClearBitsFromISR only registers clear operation in the timer command queue. */
            /* Yield is required here otherwise clear operation might not execute in the right order. */
            /* See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/93 for more info.               */
            portYIELD_FROM_ISR(pdTRUE);
        }
    } else {
        rflags = xEventGroupClearBits(hEventGroup, (EventBits_t)flags);
    }

    /* Return event flags before clearing */
    return rflags;
}

uint32_t furi_event_flag_get(FuriEventFlag* instance) {
    furi_check(instance);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;

    if(FURI_IS_IRQ_MODE()) {
        rflags = xEventGroupGetBitsFromISR(hEventGroup);
    } else {
        rflags = xEventGroupGetBits(hEventGroup);
    }

    /* Return current event flags */
    return rflags;
}

uint32_t furi_event_flag_wait(
    FuriEventFlag* instance,
    uint32_t flags,
    uint32_t options,
    uint32_t timeout) {
    furi_check(!FURI_IS_IRQ_MODE());
    furi_check(instance);
    furi_check((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    BaseType_t wait_all;
    BaseType_t exit_clr;
    uint32_t rflags;

    if(options & FuriFlagWaitAll) {
        wait_all = pdTRUE;
    } else {
        wait_all = pdFAIL;
    }

    if(options & FuriFlagNoClear) {
        exit_clr = pdFAIL;
    } else {
        exit_clr = pdTRUE;
    }

    rflags = xEventGroupWaitBits(
        hEventGroup, (EventBits_t)flags, exit_clr, wait_all, (TickType_t)timeout);

    if(options & FuriFlagWaitAll) {
        if((flags & rflags) != flags) {
            if(timeout > 0U) {
                rflags = (uint32_t)FuriStatusErrorTimeout;
            } else {
                rflags = (uint32_t)FuriStatusErrorResource;
            }
        }
    } else {
        if((flags & rflags) == 0U) {
            if(timeout > 0U) {
                rflags = (uint32_t)FuriStatusErrorTimeout;
            } else {
                rflags = (uint32_t)FuriStatusErrorResource;
            }
        }
    }

    /* Return event flags before clearing */
    return rflags;
}

</content>
</file>
<file name="furi/core/event_flag.h">
<content>
/**
 * @file event_flag.h
 * Furi Event Flag
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriEventFlag FuriEventFlag;

/** Allocate FuriEventFlag
 *
 * @return     pointer to FuriEventFlag
 */
FuriEventFlag* furi_event_flag_alloc(void);

/** Deallocate FuriEventFlag
 *
 * @param      instance  pointer to FuriEventFlag
 */
void furi_event_flag_free(FuriEventFlag* instance);

/** Set flags
 *
 * @warning    result of this function can be flags that you've just asked to
 *             set or not if someone was waiting for them and asked to clear it.
 *             It is highly recommended to read this function and
 *             xEventGroupSetBits source code.
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags to set
 *
 * @return     Resulting flags(see warning) or error (FuriStatus)
 */
uint32_t furi_event_flag_set(FuriEventFlag* instance, uint32_t flags);

/** Clear flags
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags
 *
 * @return     Resulting flags or error (FuriStatus)
 */
uint32_t furi_event_flag_clear(FuriEventFlag* instance, uint32_t flags);

/** Get flags
 *
 * @param      instance  pointer to FuriEventFlag
 *
 * @return     Resulting flags
 */
uint32_t furi_event_flag_get(FuriEventFlag* instance);

/** Wait flags
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags
 * @param[in]  options   The option flags
 * @param[in]  timeout   The timeout
 *
 * @return     Resulting flags or error (FuriStatus)
 */
uint32_t furi_event_flag_wait(
    FuriEventFlag* instance,
    uint32_t flags,
    uint32_t options,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/event_loop.c">
<content>
#include "event_loop_i.h"

#include "log.h"
#include "check.h"
#include "thread.h"

#include <FreeRTOS.h>
#include <task.h>

#define TAG "FuriEventLoop"

/*
 * Private functions
 */

static FuriEventLoopItem* furi_event_loop_item_alloc(
    FuriEventLoop* owner,
    const FuriEventLoopContract* contract,
    void* object,
    FuriEventLoopEvent event);

static void furi_event_loop_item_free(FuriEventLoopItem* instance);

static void furi_event_loop_item_free_later(FuriEventLoopItem* instance);

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopEventCallback callback,
    void* callback_context);

static void furi_event_loop_item_notify(FuriEventLoopItem* instance);

static bool furi_event_loop_item_is_waiting(FuriEventLoopItem* instance);

static void furi_event_loop_process_pending_callbacks(FuriEventLoop* instance) {
    for(; !PendingQueue_empty_p(instance->pending_queue);
        PendingQueue_pop_back(NULL, instance->pending_queue)) {
        const FuriEventLoopPendingQueueItem* item = PendingQueue_back(instance->pending_queue);
        item->callback(item->context);
    }
}

static bool furi_event_loop_signal_callback(uint32_t signal, void* arg, void* context) {
    furi_assert(context);
    FuriEventLoop* instance = context;
    UNUSED(arg);

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance);
        return true;
    // Room for possible other standard signal handlers
    default:
        return false;
    }
}

/*
 * Main public API
 */

FuriEventLoop* furi_event_loop_alloc(void) {
    FuriEventLoop* instance = malloc(sizeof(FuriEventLoop));

    instance->thread_id = furi_thread_get_current_id();

    FuriEventLoopTree_init(instance->tree);
    WaitingList_init(instance->waiting_list);
    TimerList_init(instance->timer_list);
    TimerQueue_init(instance->timer_queue);
    PendingQueue_init(instance->pending_queue);

    // Clear notification state and value
    TaskHandle_t task = (TaskHandle_t)instance->thread_id;
    xTaskNotifyStateClearIndexed(task, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX);
    ulTaskNotifyValueClearIndexed(task, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0xFFFFFFFF);

    return instance;
}

void furi_event_loop_free(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(instance->state == FuriEventLoopStateStopped);

    furi_event_loop_process_timer_queue(instance);
    furi_check(TimerList_empty_p(instance->timer_list));
    furi_check(WaitingList_empty_p(instance->waiting_list));

    FuriEventLoopTree_clear(instance->tree);
    PendingQueue_clear(instance->pending_queue);

    uint32_t flags = 0;
    BaseType_t ret = xTaskNotifyWaitIndexed(
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, 0);
    if(ret == pdTRUE) {
        FURI_LOG_D(TAG, "Some events were not processed: 0x%lx", flags);
    }

    free(instance);
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_poll_process_level_event(FuriEventLoopItem* item) {
    if(!item->contract->get_level(item->object, item->event)) {
        return FuriEventLoopProcessStatusComplete;
    } else if(item->callback(item->object, item->callback_context)) {
        return FuriEventLoopProcessStatusIncomplete;
    } else {
        return FuriEventLoopProcessStatusAgain;
    }
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_poll_process_edge_event(FuriEventLoopItem* item) {
    if(item->callback(item->object, item->callback_context)) {
        return FuriEventLoopProcessStatusComplete;
    } else {
        return FuriEventLoopProcessStatusAgain;
    }
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_poll_process_event(FuriEventLoop* instance, FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status;
    if(item->event & FuriEventLoopEventFlagOnce) {
        furi_event_loop_unsubscribe(instance, item->object);
    }

    if(item->event & FuriEventLoopEventFlagEdge) {
        status = furi_event_loop_poll_process_edge_event(item);
    } else {
        status = furi_event_loop_poll_process_level_event(item);
    }

    if(item->owner == NULL) {
        status = FuriEventLoopProcessStatusFreeLater;
    }

    return status;
}

static void furi_event_loop_process_waiting_list(FuriEventLoop* instance) {
    FuriEventLoopItem* item = NULL;

    FURI_CRITICAL_ENTER();

    if(!WaitingList_empty_p(instance->waiting_list)) {
        item = WaitingList_pop_front(instance->waiting_list);
        WaitingList_init_field(item);
    }

    FURI_CRITICAL_EXIT();

    if(!item) return;

    while(true) {
        FuriEventLoopProcessStatus ret = furi_event_loop_poll_process_event(instance, item);

        if(ret == FuriEventLoopProcessStatusComplete) {
            // Event processing complete, break from loop
            break;
        } else if(ret == FuriEventLoopProcessStatusIncomplete) {
            // Event processing incomplete more processing needed
        } else if(ret == FuriEventLoopProcessStatusAgain) { //-V547
            furi_event_loop_item_notify(item);
            break;
            // Unsubscribed from inside the callback, delete item
        } else if(ret == FuriEventLoopProcessStatusFreeLater) { //-V547
            furi_event_loop_item_free(item);
            break;
        } else {
            furi_crash();
        }
    }
}

static void furi_event_loop_restore_flags(FuriEventLoop* instance, uint32_t flags) {
    if(flags) {
        xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, flags, eSetBits);
    }
}

void furi_event_loop_run(FuriEventLoop* instance) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    FuriThread* thread = furi_thread_get_current();

    // Set the default signal callback if none was previously set
    if(furi_thread_get_signal_callback(thread) == NULL) {
        furi_thread_set_signal_callback(thread, furi_event_loop_signal_callback, instance);
    }

    furi_event_loop_init_tick(instance);

    while(true) {
        instance->state = FuriEventLoopStateIdle;

        const TickType_t ticks_to_sleep =
            MIN(furi_event_loop_get_timer_wait_time(instance),
                furi_event_loop_get_tick_wait_time(instance));

        uint32_t flags = 0;
        BaseType_t ret = xTaskNotifyWaitIndexed(
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, 0, FuriEventLoopFlagAll, &flags, ticks_to_sleep);

        instance->state = FuriEventLoopStateProcessing;

        if(ret == pdTRUE) {
            if(flags & FuriEventLoopFlagStop) {
                instance->state = FuriEventLoopStateStopped;
                break;

            } else if(flags & FuriEventLoopFlagEvent) {
                furi_event_loop_process_waiting_list(instance);
                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagEvent);

            } else if(flags & FuriEventLoopFlagTimer) {
                furi_event_loop_process_timer_queue(instance);
                furi_event_loop_restore_flags(instance, flags & ~FuriEventLoopFlagTimer);

            } else if(flags & FuriEventLoopFlagPending) {
                furi_event_loop_process_pending_callbacks(instance);

            } else {
                furi_crash();
            }

        } else if(!furi_event_loop_process_expired_timers(instance)) {
            furi_event_loop_process_tick(instance);
        }
    }

    // Disable the default signal callback
    if(furi_thread_get_signal_callback(thread) == furi_event_loop_signal_callback) {
        furi_thread_set_signal_callback(thread, NULL, NULL);
    }
}

void furi_event_loop_stop(FuriEventLoop* instance) {
    furi_check(instance);

    xTaskNotifyIndexed(
        (TaskHandle_t)instance->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagStop,
        eSetBits);
}

/*
 * Public deferred function call API
 */

void furi_event_loop_pend_callback(
    FuriEventLoop* instance,
    FuriEventLoopPendingCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback);

    const FuriEventLoopPendingQueueItem item = {
        .callback = callback,
        .context = context,
    };

    PendingQueue_push_front(instance->pending_queue, item);

    xTaskNotifyIndexed(
        (TaskHandle_t)instance->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagPending,
        eSetBits);
}

/*
 * Private generic susbscription API
 */

static void furi_event_loop_object_subscribe(
    FuriEventLoop* instance,
    FuriEventLoopObject* object,
    const FuriEventLoopContract* contract,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(object);
    furi_assert(contract);
    furi_check(callback);

    FURI_CRITICAL_ENTER();

    furi_check(FuriEventLoopTree_get(instance->tree, object) == NULL);

    // Allocate and setup item
    FuriEventLoopItem* item = furi_event_loop_item_alloc(instance, contract, object, event);
    furi_event_loop_item_set_callback(item, callback, context);

    FuriEventLoopTree_set_at(instance->tree, object, item);

    FuriEventLoopLink* link = item->contract->get_link(object);
    FuriEventLoopEvent event_noflags = item->event & FuriEventLoopEventMask;

    if(event_noflags == FuriEventLoopEventIn) {
        furi_check(link->item_in == NULL);
        link->item_in = item;
    } else if(event_noflags == FuriEventLoopEventOut) {
        furi_check(link->item_out == NULL);
        link->item_out = item;
    } else {
        furi_crash();
    }

    if(!(item->event & FuriEventLoopEventFlagEdge)) {
        if(item->contract->get_level(item->object, event_noflags)) {
            furi_event_loop_item_notify(item);
        }
    }

    FURI_CRITICAL_EXIT();
}

/**
 * Public specialized subscription API
 */

void furi_event_loop_subscribe_message_queue(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_message_queue_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, message_queue, &furi_message_queue_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_stream_buffer(
    FuriEventLoop* instance,
    FuriStreamBuffer* stream_buffer,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_stream_buffer_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, stream_buffer, &furi_stream_buffer_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_semaphore(
    FuriEventLoop* instance,
    FuriSemaphore* semaphore,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_semaphore_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, semaphore, &furi_semaphore_event_loop_contract, event, callback, context);
}

void furi_event_loop_subscribe_mutex(
    FuriEventLoop* instance,
    FuriMutex* mutex,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_mutex_event_loop_contract;

    furi_event_loop_object_subscribe(
        instance, mutex, &furi_mutex_event_loop_contract, event, callback, context);
}

/**
 * Public generic unsubscription API
 */

void furi_event_loop_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());

    FURI_CRITICAL_ENTER();

    FuriEventLoopItem* item = NULL;
    furi_check(FuriEventLoopTree_pop_at(&item, instance->tree, object));

    furi_check(item);
    furi_check(item->owner == instance);

    FuriEventLoopLink* link = item->contract->get_link(object);
    FuriEventLoopEvent event_noflags = item->event & FuriEventLoopEventMask;

    if(event_noflags == FuriEventLoopEventIn) {
        furi_check(link->item_in == item);
        link->item_in = NULL;
    } else if(event_noflags == FuriEventLoopEventOut) {
        furi_check(link->item_out == item);
        link->item_out = NULL;
    } else {
        furi_crash();
    }

    if(furi_event_loop_item_is_waiting(item)) {
        WaitingList_unlink(item);
    }

    if(instance->state == FuriEventLoopStateProcessing) {
        furi_event_loop_item_free_later(item);
    } else {
        furi_event_loop_item_free(item);
    }

    FURI_CRITICAL_EXIT();
}

/* 
 * Private Event Loop Item functions
 */

static FuriEventLoopItem* furi_event_loop_item_alloc(
    FuriEventLoop* owner,
    const FuriEventLoopContract* contract,
    void* object,
    FuriEventLoopEvent event) {
    furi_assert(owner);
    furi_assert(object);

    FuriEventLoopItem* instance = malloc(sizeof(FuriEventLoopItem));

    instance->owner = owner;
    instance->contract = contract;
    instance->object = object;
    instance->event = event;

    WaitingList_init_field(instance);

    return instance;
}

static void furi_event_loop_item_free(FuriEventLoopItem* instance) {
    furi_assert(instance);
    furi_assert(!furi_event_loop_item_is_waiting(instance));
    free(instance);
}

static void furi_event_loop_item_free_later(FuriEventLoopItem* instance) {
    furi_assert(instance);
    furi_assert(!furi_event_loop_item_is_waiting(instance));
    instance->owner = NULL;
}

static void furi_event_loop_item_set_callback(
    FuriEventLoopItem* instance,
    FuriEventLoopEventCallback callback,
    void* callback_context) {
    furi_assert(instance);
    furi_assert(!instance->callback);

    instance->callback = callback;
    instance->callback_context = callback_context;
}

static void furi_event_loop_item_notify(FuriEventLoopItem* instance) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    FuriEventLoop* owner = instance->owner;
    furi_assert(owner);

    if(!furi_event_loop_item_is_waiting(instance)) {
        WaitingList_push_back(owner->waiting_list, instance);
    }

    FURI_CRITICAL_EXIT();

    xTaskNotifyIndexed(
        (TaskHandle_t)owner->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagEvent,
        eSetBits);
}

static bool furi_event_loop_item_is_waiting(FuriEventLoopItem* instance) {
    return instance->WaitingList.prev || instance->WaitingList.next;
}

/*
 * Internal event loop link API, used by supported primitives
 */

void furi_event_loop_link_notify(FuriEventLoopLink* instance, FuriEventLoopEvent event) {
    furi_assert(instance);

    FURI_CRITICAL_ENTER();

    if(event & FuriEventLoopEventIn) {
        if(instance->item_in) furi_event_loop_item_notify(instance->item_in);
    } else if(event & FuriEventLoopEventOut) {
        if(instance->item_out) furi_event_loop_item_notify(instance->item_out);
    } else {
        furi_crash();
    }

    FURI_CRITICAL_EXIT();
}

</content>
</file>
<file name="furi/core/event_loop.h">
<content>
/**
 * @file event_loop.h
 * @brief      Furi Event Loop
 *
 *             This module is designed to handle application event loop in fully
 *             asynchronous, reactive nature. On the low level this modules is
 *             inspired by epoll/kqueue concept, on the high level by asyncio
 *             event loop.
 *
 *             This module is trying to best fit into Furi OS, so we don't
 *             provide any compatibility with other event driven APIs. But
 *             programming concepts are the same, except some runtime
 *             limitations from our side.
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of event types, flags and masks.
 *
 * Only one event direction (In or Out) can be used per subscription.
 * An object can have no more than one subscription for each direction.
 *
 * Additional flags that modify the behaviour can be
 * set using the bitwise OR operation (see flag description).
 */
typedef enum {
    /**
     * @brief Subscribe to In events.
     *
     * In events occur on the following conditions:
     * - One or more items were inserted into a FuriMessageQueue,
     * - Enough data has been written to a FuriStreamBuffer,
     * - A FuriSemaphore has been released at least once,
     * - A FuriMutex has been released.
     */
    FuriEventLoopEventIn = 0x00000001U,
    /**
     * @brief Subscribe to Out events.
     *
     * Out events occur on the following conditions:
     * - One or more items were removed from a FuriMessageQueue,
     * - Any amount of data has been read out of a FuriStreamBuffer,
     * - A FuriSemaphore has been acquired at least once,
     * - A FuriMutex has been acquired.
     */
    FuriEventLoopEventOut = 0x00000002U,
    /**
     * @brief Special value containing the event direction bits, used internally.
     */
    FuriEventLoopEventMask = 0x00000003U,
    /**
     * @brief Use edge triggered events.
     *
     * By default, level triggered events are used. A level above zero
     * is reported based on the following conditions:
     *
     * In events:
     * - a FuriMessageQueue contains one or more items,
     * - a FuriStreamBuffer contains one or more bytes,
     * - a FuriSemaphore can be acquired at least once,
     * - a FuriMutex can be acquired.
     *
     * Out events:
     * - a FuriMessageQueue has at least one item of free space,
     * - a FuriStreamBuffer has at least one byte of free space,
     * - a FuriSemaphore has been acquired at least once,
     * - a FuriMutex has been acquired.
     *
     * If this flag is NOT set, the event will be generated repeatedly until
     * the level becomes zero (e.g. all items have been removed from
     * a FuriMessageQueue in case of the "In" event, etc.)
     *
     * If this flag IS set, then the above check is skipped and the event
     * is generated ONLY when a change occurs, with the event direction
     * (In or Out) taken into account.
     */
    FuriEventLoopEventFlagEdge = 0x00000004U,
    /**
     * @brief Automatically unsubscribe from events after one time.
     *
     * By default, events will be generated each time the specified conditions
     * have been met. If this flag IS set, the event subscription will be cancelled
     * upon the first occurred event and no further events will be generated.
     */
    FuriEventLoopEventFlagOnce = 0x00000008U,
    /**
     * @brief Special value containing the event flag bits, used internally.
     */
    FuriEventLoopEventFlagMask = 0xFFFFFFFCU,
    /**
     * @brief Special value to force the enum to 32-bit values.
     */
    FuriEventLoopEventReserved = UINT32_MAX,
} FuriEventLoopEvent;

/** Anonymous message queue type */
typedef struct FuriEventLoop FuriEventLoop;

/** Allocate Event Loop instance
 *
 * Couple things to keep in mind:
 * - You can have 1 event_loop per 1 thread
 * - You can not use event_loop instance in the other thread
 * - Do not use blocking API to query object delegated to Event Loop
 *
 * @return     The Event Loop instance
 */
FuriEventLoop* furi_event_loop_alloc(void);

/** Free Event Loop instance
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_free(FuriEventLoop* instance);

/** Continuously poll for events
 *
 * Can be stopped with `furi_event_loop_stop`
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_run(FuriEventLoop* instance);

/** Stop Event Loop instance
 *
 * @param      instance  The Event Loop instance
 */
void furi_event_loop_stop(FuriEventLoop* instance);

/*
 * Tick related API
 */

/** Tick callback type
 *
 * @param      context  The context for callback
 */
typedef void (*FuriEventLoopTickCallback)(void* context);

/** Set Event Loop tick callback
 *
 * Tick callback is called periodically after specified inactivity time.
 * It acts like a low-priority timer: it will only fire if there is time
 * left after processing the synchronization primitives and the regular timers.
 * Therefore, it is not monotonic: ticks will be skipped if the event loop is busy.
 *
 * @param      instance  The Event Loop instance
 * @param[in]  interval  The tick interval
 * @param[in]  callback  The callback to call
 * @param      context   The context for callback
 */
void furi_event_loop_tick_set(
    FuriEventLoop* instance,
    uint32_t interval,
    FuriEventLoopTickCallback callback,
    void* context);

/*
 * Deferred function call API
 */

/**
 * @brief Timer callback type for functions to be called in a deferred manner.
 *
 * @param[in,out] context pointer to a user-specific object that was provided during
 *                        furi_event_loop_pend_callback() call
 */
typedef void (*FuriEventLoopPendingCallback)(void* context);

/**
 * @brief Call a function when all preceding timer commands are processed
 *
 * This function may be useful to call another function when the event loop has been started.
 *
 * @param[in,out] instance pointer to the current FuriEventLoop instance
 * @param[in] callback pointer to the callback to be executed when previous commands have been processed
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 */
void furi_event_loop_pend_callback(
    FuriEventLoop* instance,
    FuriEventLoopPendingCallback callback,
    void* context);

/*
 * Event subscription/notification APIs
 */

typedef void FuriEventLoopObject;

/** Callback type for event loop events
 *
 * @param      object   The object that triggered the event
 * @param      context  The context that was provided upon subscription
 *
 * @return     true if event was processed, false if we need to delay processing
 */
typedef bool (*FuriEventLoopEventCallback)(FuriEventLoopObject* object, void* context);

/** Opaque message queue type */
typedef struct FuriMessageQueue FuriMessageQueue;

/** Subscribe to message queue events
 * 
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      message_queue  The message queue to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_message_queue(
    FuriEventLoop* instance,
    FuriMessageQueue* message_queue,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque stream buffer type */
typedef struct FuriStreamBuffer FuriStreamBuffer;

/** Subscribe to stream buffer events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      stream_buffer  The stream buffer to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_stream_buffer(
    FuriEventLoop* instance,
    FuriStreamBuffer* stream_buffer,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque semaphore type */
typedef struct FuriSemaphore FuriSemaphore;

/** Subscribe to semaphore events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      semaphore      The semaphore to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_semaphore(
    FuriEventLoop* instance,
    FuriSemaphore* semaphore,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Opaque mutex type */
typedef struct FuriMutex FuriMutex;

/** Subscribe to mutex events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      mutex          The mutex to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */
void furi_event_loop_subscribe_mutex(
    FuriEventLoop* instance,
    FuriMutex* mutex,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

/** Unsubscribe from events (common)
 *
 * @param      instance       The Event Loop instance
 * @param      object         The object to unsubscribe from
 */
void furi_event_loop_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/event_loop_i.h">
<content>
#pragma once

#include "event_loop.h"
#include "event_loop_link_i.h"
#include "event_loop_timer_i.h"
#include "event_loop_tick_i.h"

#include <m-list.h>
#include <m-bptree.h>
#include <m-i-list.h>

#include "thread.h"

struct FuriEventLoopItem {
    // Source
    FuriEventLoop* owner;

    // Tracking item
    FuriEventLoopEvent event;
    FuriEventLoopObject* object;
    const FuriEventLoopContract* contract;

    // Callback and context
    FuriEventLoopEventCallback callback;
    void* callback_context;

    // Waiting list
    ILIST_INTERFACE(WaitingList, FuriEventLoopItem);
};

ILIST_DEF(WaitingList, FuriEventLoopItem, M_POD_OPLIST)

/* Event Loop RB tree */
#define FURI_EVENT_LOOP_TREE_RANK (4)

BPTREE_DEF2( // NOLINT
    FuriEventLoopTree,
    FURI_EVENT_LOOP_TREE_RANK,
    FuriEventLoopObject*, /* pointer to object we track */
    M_PTR_OPLIST,
    FuriEventLoopItem*, /* pointer to the FuriEventLoopItem */
    M_PTR_OPLIST)

#define M_OPL_FuriEventLoopTree_t() BPTREE_OPLIST(FuriEventLoopTree, M_POD_OPLIST)

#define FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX (2)

typedef enum {
    FuriEventLoopFlagEvent = (1 << 0),
    FuriEventLoopFlagStop = (1 << 1),
    FuriEventLoopFlagTimer = (1 << 2),
    FuriEventLoopFlagPending = (1 << 3),
} FuriEventLoopFlag;

#define FuriEventLoopFlagAll                                                   \
    (FuriEventLoopFlagEvent | FuriEventLoopFlagStop | FuriEventLoopFlagTimer | \
     FuriEventLoopFlagPending)

typedef enum {
    FuriEventLoopProcessStatusComplete,
    FuriEventLoopProcessStatusIncomplete,
    FuriEventLoopProcessStatusAgain,
    FuriEventLoopProcessStatusFreeLater,
} FuriEventLoopProcessStatus;

typedef enum {
    FuriEventLoopStateStopped,
    FuriEventLoopStateIdle,
    FuriEventLoopStateProcessing,
} FuriEventLoopState;

typedef struct {
    FuriEventLoopPendingCallback callback;
    void* context;
} FuriEventLoopPendingQueueItem;

LIST_DUAL_PUSH_DEF(PendingQueue, FuriEventLoopPendingQueueItem, M_POD_OPLIST)

struct FuriEventLoop {
    // Only works if all operations are done from the same thread
    FuriThreadId thread_id;

    // Poller state
    volatile FuriEventLoopState state;

    // Event handling
    FuriEventLoopTree_t tree;
    WaitingList_t waiting_list;

    // Active timer list
    TimerList_t timer_list;
    // Timer request queue
    TimerQueue_t timer_queue;
    // Pending callback queue
    PendingQueue_t pending_queue;
    // Tick event
    FuriEventLoopTick tick;
};

</content>
</file>
<file name="furi/core/event_loop_link_i.h">
<content>
#pragma once

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriEventLoopItem FuriEventLoopItem;

/* Link between Event Loop  */

typedef struct {
    FuriEventLoopItem* item_in;
    FuriEventLoopItem* item_out;
} FuriEventLoopLink;

void furi_event_loop_link_notify(FuriEventLoopLink* instance, FuriEventLoopEvent event);

/* Contract between event loop and an object */

typedef FuriEventLoopLink* (*FuriEventLoopContractGetLink)(FuriEventLoopObject* object);

typedef uint32_t (
    *FuriEventLoopContractGetLevel)(FuriEventLoopObject* object, FuriEventLoopEvent event);

typedef struct {
    const FuriEventLoopContractGetLink get_link;
    const FuriEventLoopContractGetLevel get_level;
} FuriEventLoopContract;

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/event_loop_tick.c">
<content>
#include "event_loop_i.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi.h>

/**
 * Private functions
 */

static inline uint32_t furi_event_loop_tick_get_elapsed_time(const FuriEventLoop* instance) {
    return xTaskGetTickCount() - instance->tick.prev_time;
}

static inline uint32_t furi_event_loop_tick_get_remaining_time(const FuriEventLoop* instance) {
    const uint32_t elapsed_time = furi_event_loop_tick_get_elapsed_time(instance);
    return elapsed_time < instance->tick.interval ? instance->tick.interval - elapsed_time : 0;
}

static inline bool furi_event_loop_tick_is_expired(const FuriEventLoop* instance) {
    return furi_event_loop_tick_get_elapsed_time(instance) >= instance->tick.interval;
}

/*
 * Private tick API
 */

void furi_event_loop_init_tick(FuriEventLoop* instance) {
    if(instance->tick.callback) {
        instance->tick.prev_time = xTaskGetTickCount();
    }
}

void furi_event_loop_process_tick(FuriEventLoop* instance) {
    if(instance->tick.callback && furi_event_loop_tick_is_expired(instance)) {
        instance->tick.prev_time += instance->tick.interval;
        instance->tick.callback(instance->tick.callback_context);
    }
}

uint32_t furi_event_loop_get_tick_wait_time(const FuriEventLoop* instance) {
    uint32_t wait_time = FuriWaitForever;

    if(instance->tick.callback) {
        wait_time = furi_event_loop_tick_get_remaining_time(instance);
    }

    return wait_time;
}

/*
 * Public tick API
 */

void furi_event_loop_tick_set(
    FuriEventLoop* instance,
    uint32_t interval,
    FuriEventLoopTickCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback ? interval > 0 : true);

    instance->tick.callback = callback;
    instance->tick.callback_context = context;
    instance->tick.interval = interval;
    instance->tick.prev_time = xTaskGetTickCount();
}

</content>
</file>
<file name="furi/core/event_loop_tick_i.h">
<content>
#pragma once

#include "event_loop.h"

typedef struct {
    uint32_t interval;
    uint32_t prev_time;
    FuriEventLoopTickCallback callback;
    void* callback_context;
} FuriEventLoopTick;

void furi_event_loop_init_tick(FuriEventLoop* instance);

void furi_event_loop_process_tick(FuriEventLoop* instance);

uint32_t furi_event_loop_get_tick_wait_time(const FuriEventLoop* instance);

</content>
</file>
<file name="furi/core/event_loop_timer.c">
<content>
#include "event_loop_i.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi.h>

/*
 * Private functions
 */

static inline uint32_t furi_event_loop_timer_get_elapsed_time(const FuriEventLoopTimer* timer) {
    return xTaskGetTickCount() - timer->start_time;
}

static inline uint32_t
    furi_event_loop_timer_get_remaining_time_private(const FuriEventLoopTimer* timer) {
    const uint32_t elapsed_time = furi_event_loop_timer_get_elapsed_time(timer);
    return elapsed_time < timer->interval ? timer->interval - elapsed_time : 0;
}

static inline bool furi_event_loop_timer_is_expired(const FuriEventLoopTimer* timer) {
    return furi_event_loop_timer_get_elapsed_time(timer) >= timer->interval;
}

static void furi_event_loop_schedule_timer(FuriEventLoop* instance, FuriEventLoopTimer* timer) {
    FuriEventLoopTimer* timer_pos = NULL;

    FURI_CRITICAL_ENTER();

    const uint32_t remaining_time = furi_event_loop_timer_get_remaining_time_private(timer);

    TimerList_it_t it;
    for(TimerList_it_last(it, instance->timer_list); !TimerList_end_p(it);
        TimerList_previous(it)) {
        FuriEventLoopTimer* tmp = TimerList_ref(it);
        if(remaining_time >= furi_event_loop_timer_get_remaining_time_private(tmp)) {
            timer_pos = tmp;
            break;
        }
    }

    FURI_CRITICAL_EXIT();

    if(timer_pos) {
        TimerList_push_after(timer_pos, timer);
    } else {
        TimerList_push_front(instance->timer_list, timer);
    }
    // At this point, TimerList_front() points to the first timer to expire
}

static void furi_event_loop_timer_enqueue_request(
    FuriEventLoopTimer* timer,
    FuriEventLoopTimerRequest request) {
    if(timer->request != FuriEventLoopTimerRequestNone) {
        // You cannot change your mind after calling furi_event_loop_timer_free()
        furi_check(timer->request != FuriEventLoopTimerRequestFree);
        TimerQueue_unlink(timer);
    }

    timer->request = request;

    FuriEventLoop* instance = timer->owner;
    TimerQueue_push_back(instance->timer_queue, timer);

    xTaskNotifyIndexed(
        (TaskHandle_t)instance->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagTimer,
        eSetBits);
}

/*
 * Private API
 */

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance) {
    uint32_t wait_time = FuriWaitForever;

    if(!TimerList_empty_p(instance->timer_list)) {
        FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);
        wait_time = furi_event_loop_timer_get_remaining_time_private(timer);
    }

    return wait_time;
}

void furi_event_loop_process_timer_queue(FuriEventLoop* instance) {
    while(!TimerQueue_empty_p(instance->timer_queue)) {
        FuriEventLoopTimer* timer = TimerQueue_pop_front(instance->timer_queue);

        if(timer->active) {
            TimerList_unlink(timer);
        }

        if(timer->request == FuriEventLoopTimerRequestStart) {
            timer->active = true;
            timer->interval = timer->next_interval;
            timer->start_time = xTaskGetTickCount();
            timer->request = FuriEventLoopTimerRequestNone;

            furi_event_loop_schedule_timer(instance, timer);

        } else if(timer->request == FuriEventLoopTimerRequestStop) {
            timer->active = false;
            timer->request = FuriEventLoopTimerRequestNone;

        } else if(timer->request == FuriEventLoopTimerRequestFree) {
            free(timer);

        } else {
            furi_crash();
        }
    }
}

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance) {
    if(TimerList_empty_p(instance->timer_list)) {
        return false;
    }
    // The front() element contains the earliest-expiring timer
    FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);

    if(!furi_event_loop_timer_is_expired(timer)) {
        return false;
    }

    TimerList_unlink(timer);

    if(timer->periodic) {
        const uint32_t num_events =
            furi_event_loop_timer_get_elapsed_time(timer) / timer->interval;

        timer->start_time += timer->interval * num_events;
        furi_event_loop_schedule_timer(instance, timer);

    } else {
        timer->active = false;
    }

    timer->callback(timer->context);
    return true;
}

/*
 * Public timer API
 */

FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback);
    furi_check(type <= FuriEventLoopTimerTypePeriodic);

    FuriEventLoopTimer* timer = malloc(sizeof(FuriEventLoopTimer));

    timer->owner = instance;
    timer->callback = callback;
    timer->context = context;
    timer->periodic = (type == FuriEventLoopTimerTypePeriodic);

    TimerList_init_field(timer);
    TimerQueue_init_field(timer);

    return timer;
}

void furi_event_loop_timer_free(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestFree);
}

void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_restart(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = timer->interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_stop(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStop);
}

uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return furi_event_loop_timer_get_remaining_time_private(timer);
}

uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->interval;
}

bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->active;
}

</content>
</file>
<file name="furi/core/event_loop_timer.h">
<content>
/**
 * @file event_loop_timer.h
 * @brief Software timer functionality for FuriEventLoop.
 */

#pragma once

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible timer types.
 */
typedef enum {
    FuriEventLoopTimerTypeOnce = 0, /**< One-shot timer. */
    FuriEventLoopTimerTypePeriodic = 1, /**< Repeating timer. */
} FuriEventLoopTimerType;

/**
 * @brief Timer callback type for functions to be called when a timer expires.
 *
 * In the timer callback, it is ALLOWED:
 * - To start, stop, or restart an existing timer,
 * - To create new timers using furi_event_loop_timer_alloc(),
 * - To delete timers using furi_event_loop_timer_free().
 *
 * @param[in,out] context pointer to a user-specific object that was provided during timer creation
 */
typedef void (*FuriEventLoopTimerCallback)(void* context);

/**
 * @brief Opaque event loop timer type.
 */
typedef struct FuriEventLoopTimer FuriEventLoopTimer;

/**
 * @brief Create a new event loop timer instance.
 *
 * @param[in,out] instance pointer to the current FuriEventLoop instance
 * @param[in] callback pointer to the callback function to be executed upon timer timeout
 * @param[in] type timer type value to determine its behavior (single-shot or periodic)
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 * @returns pointer to the created timer instance
 */
FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context);

/**
 * @brief Delete an event loop timer instance.
 *
 * @warning The user code MUST call furi_event_loop_timer_free() on ALL instances
 *          associated with the current event loop BEFORE calling furi_event_loop_free().
 *          The event loop may EITHER be running OR stopped when the timers are being deleted.
 *
 * @param[in,out] timer pointer to the timer instance to be deleted
 */
void furi_event_loop_timer_free(FuriEventLoopTimer* timer);

/**
 * @brief Start a timer or restart it with a new interval.
 *
 * @param[in,out] timer pointer to the timer instance to be (re)started
 * @param[in] interval timer interval in ticks
 */
void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval);

/**
 * @brief Restart a timer with the previously set interval.
 *
 * @param[in,out] timer pointer to the timer instance to be restarted
 */
void furi_event_loop_timer_restart(FuriEventLoopTimer* timer);

/**
 * @brief Stop a timer without firing its callback.
 *
 * It is safe to call this function on an already stopped timer (it will do nothing).
 *
 * @param[in,out] timer pointer to the timer instance to be stopped
 */
void furi_event_loop_timer_stop(FuriEventLoopTimer* timer);

/**
 * @brief Get the time remaining before the timer becomes expires.
 *
 * For stopped or expired timers, this function returns 0.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns remaining time in ticks
 */
uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer);

/**
 * @brief Get the timer interval.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns timer interval in ticks
 */
uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer);

/**
 * @brief Check if the timer is currently running.
 *
 * A timer is considered running if it has not expired yet.
 * @param[in] timer pointer to the timer to be queried
 * @returns true if the timer is running, false otherwise
 */
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/event_loop_timer_i.h">
<content>
#pragma once

#include "event_loop_timer.h"

#include <m-i-list.h>

typedef enum {
    FuriEventLoopTimerRequestNone,
    FuriEventLoopTimerRequestStart,
    FuriEventLoopTimerRequestStop,
    FuriEventLoopTimerRequestFree,
} FuriEventLoopTimerRequest;

struct FuriEventLoopTimer {
    FuriEventLoop* owner;

    FuriEventLoopTimerCallback callback;
    void* context;

    uint32_t interval;
    uint32_t start_time;
    uint32_t next_interval;

    // Interface for the active timer list
    ILIST_INTERFACE(TimerList, FuriEventLoopTimer);

    // Interface for the timer request queue
    ILIST_INTERFACE(TimerQueue, FuriEventLoopTimer);

    FuriEventLoopTimerRequest request;

    bool active;
    bool periodic;
};

ILIST_DEF(TimerList, FuriEventLoopTimer, M_POD_OPLIST)
ILIST_DEF(TimerQueue, FuriEventLoopTimer, M_POD_OPLIST)

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance);

void furi_event_loop_process_timer_queue(FuriEventLoop* instance);

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance);

</content>
</file>

<file name="applications/examples/example_event_loop/example_event_loop_timer.c">
<content>
/**
 * @file example_event_loop_timer.c
 * @brief Example application that demonstrates FuriEventLoop's software timer capability.
 *
 * This application prints a countdown from 10 to 0 to the debug output and then exits.
 * Despite only one timer being used in this example for clarity, an event loop instance can have
 * an arbitrary number of independent timers of any type (periodic or one-shot).
 *
 */
#include <furi.h>

#define TAG "ExampleEventLoopTimer"

#define COUNTDOWN_START_VALUE (10)
#define COUNTDOWN_INTERVAL_MS (1000)

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t countdown_value;
} EventLoopTimerApp;

// This function is called each time the timer expires (i.e. once per 1000 ms (1s) in this example)
static void event_loop_timer_callback(void* context) {
    furi_assert(context);
    EventLoopTimerApp* app = context;

    // Print the countdown value
    FURI_LOG_I(TAG, "T-00:00:%02lu", app->countdown_value);

    if(app->countdown_value == 0) {
        // If the countdown reached 0, print the final line and stop the event loop
        FURI_LOG_I(TAG, "Blast off to adventure!");
        // After this call, the control will be returned back to event_loop_timers_app_run()
        furi_event_loop_stop(app->event_loop);

    } else {
        // Decrement the countdown value
        app->countdown_value -= 1;
    }
}

static EventLoopTimerApp* event_loop_timer_app_alloc(void) {
    EventLoopTimerApp* app = malloc(sizeof(EventLoopTimerApp));

    // Create an event loop instance.
    app->event_loop = furi_event_loop_alloc();
    // Create a software timer instance.
    // The timer is bound to the event loop instance and will execute in its context.
    // Here, the timer type is periodic, i.e. it will restart automatically after expiring.
    app->timer = furi_event_loop_timer_alloc(
        app->event_loop, event_loop_timer_callback, FuriEventLoopTimerTypePeriodic, app);
    // The countdown value will be tracked in this variable.
    app->countdown_value = COUNTDOWN_START_VALUE;

    return app;
}

static void event_loop_timer_app_free(EventLoopTimerApp* app) {
    // IMPORTANT: All event loop timers MUST be deleted BEFORE deleting the event loop itself.
    // Failure to do so will result in a crash.
    furi_event_loop_timer_free(app->timer);
    // With all timers deleted, it's safe to delete the event loop.
    furi_event_loop_free(app->event_loop);
    free(app);
}

static void event_loop_timer_app_run(EventLoopTimerApp* app) {
    FURI_LOG_I(TAG, "All systems go! Prepare for countdown!");

    // Timers can be started either before the event loop is run, or in any
    // callback function called by a running event loop.
    furi_event_loop_timer_start(app->timer, COUNTDOWN_INTERVAL_MS);
    // This call will block until furi_event_loop_stop() is called.
    furi_event_loop_run(app->event_loop);
}

// The application's entry point - referenced in application.fam
int32_t example_event_loop_timer_app(void* arg) {
    UNUSED(arg);

    EventLoopTimerApp* app = event_loop_timer_app_alloc();
    event_loop_timer_app_run(app);
    event_loop_timer_app_free(app);

    return 0;
}

</content>
</file>
<file name="furi/core/event_loop_timer.c">
<content>
#include "event_loop_i.h"

#include <FreeRTOS.h>
#include <task.h>

#include <furi.h>

/*
 * Private functions
 */

static inline uint32_t furi_event_loop_timer_get_elapsed_time(const FuriEventLoopTimer* timer) {
    return xTaskGetTickCount() - timer->start_time;
}

static inline uint32_t
    furi_event_loop_timer_get_remaining_time_private(const FuriEventLoopTimer* timer) {
    const uint32_t elapsed_time = furi_event_loop_timer_get_elapsed_time(timer);
    return elapsed_time < timer->interval ? timer->interval - elapsed_time : 0;
}

static inline bool furi_event_loop_timer_is_expired(const FuriEventLoopTimer* timer) {
    return furi_event_loop_timer_get_elapsed_time(timer) >= timer->interval;
}

static void furi_event_loop_schedule_timer(FuriEventLoop* instance, FuriEventLoopTimer* timer) {
    FuriEventLoopTimer* timer_pos = NULL;

    FURI_CRITICAL_ENTER();

    const uint32_t remaining_time = furi_event_loop_timer_get_remaining_time_private(timer);

    TimerList_it_t it;
    for(TimerList_it_last(it, instance->timer_list); !TimerList_end_p(it);
        TimerList_previous(it)) {
        FuriEventLoopTimer* tmp = TimerList_ref(it);
        if(remaining_time >= furi_event_loop_timer_get_remaining_time_private(tmp)) {
            timer_pos = tmp;
            break;
        }
    }

    FURI_CRITICAL_EXIT();

    if(timer_pos) {
        TimerList_push_after(timer_pos, timer);
    } else {
        TimerList_push_front(instance->timer_list, timer);
    }
    // At this point, TimerList_front() points to the first timer to expire
}

static void furi_event_loop_timer_enqueue_request(
    FuriEventLoopTimer* timer,
    FuriEventLoopTimerRequest request) {
    if(timer->request != FuriEventLoopTimerRequestNone) {
        // You cannot change your mind after calling furi_event_loop_timer_free()
        furi_check(timer->request != FuriEventLoopTimerRequestFree);
        TimerQueue_unlink(timer);
    }

    timer->request = request;

    FuriEventLoop* instance = timer->owner;
    TimerQueue_push_back(instance->timer_queue, timer);

    xTaskNotifyIndexed(
        (TaskHandle_t)instance->thread_id,
        FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
        FuriEventLoopFlagTimer,
        eSetBits);
}

/*
 * Private API
 */

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance) {
    uint32_t wait_time = FuriWaitForever;

    if(!TimerList_empty_p(instance->timer_list)) {
        FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);
        wait_time = furi_event_loop_timer_get_remaining_time_private(timer);
    }

    return wait_time;
}

void furi_event_loop_process_timer_queue(FuriEventLoop* instance) {
    while(!TimerQueue_empty_p(instance->timer_queue)) {
        FuriEventLoopTimer* timer = TimerQueue_pop_front(instance->timer_queue);

        if(timer->active) {
            TimerList_unlink(timer);
        }

        if(timer->request == FuriEventLoopTimerRequestStart) {
            timer->active = true;
            timer->interval = timer->next_interval;
            timer->start_time = xTaskGetTickCount();
            timer->request = FuriEventLoopTimerRequestNone;

            furi_event_loop_schedule_timer(instance, timer);

        } else if(timer->request == FuriEventLoopTimerRequestStop) {
            timer->active = false;
            timer->request = FuriEventLoopTimerRequestNone;

        } else if(timer->request == FuriEventLoopTimerRequestFree) {
            free(timer);

        } else {
            furi_crash();
        }
    }
}

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance) {
    if(TimerList_empty_p(instance->timer_list)) {
        return false;
    }
    // The front() element contains the earliest-expiring timer
    FuriEventLoopTimer* timer = TimerList_front(instance->timer_list);

    if(!furi_event_loop_timer_is_expired(timer)) {
        return false;
    }

    TimerList_unlink(timer);

    if(timer->periodic) {
        const uint32_t num_events =
            furi_event_loop_timer_get_elapsed_time(timer) / timer->interval;

        timer->start_time += timer->interval * num_events;
        furi_event_loop_schedule_timer(instance, timer);

    } else {
        timer->active = false;
    }

    timer->callback(timer->context);
    return true;
}

/*
 * Public timer API
 */

FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    furi_check(callback);
    furi_check(type <= FuriEventLoopTimerTypePeriodic);

    FuriEventLoopTimer* timer = malloc(sizeof(FuriEventLoopTimer));

    timer->owner = instance;
    timer->callback = callback;
    timer->context = context;
    timer->periodic = (type == FuriEventLoopTimerTypePeriodic);

    TimerList_init_field(timer);
    TimerQueue_init_field(timer);

    return timer;
}

void furi_event_loop_timer_free(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestFree);
}

void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_restart(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    timer->next_interval = timer->interval;

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStart);
}

void furi_event_loop_timer_stop(FuriEventLoopTimer* timer) {
    furi_check(timer);
    furi_check(timer->owner->thread_id == furi_thread_get_current_id());

    furi_event_loop_timer_enqueue_request(timer, FuriEventLoopTimerRequestStop);
}

uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return furi_event_loop_timer_get_remaining_time_private(timer);
}

uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->interval;
}

bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer) {
    furi_check(timer);
    return timer->active;
}

</content>
</file>
<file name="furi/core/event_loop_timer.h">
<content>
/**
 * @file event_loop_timer.h
 * @brief Software timer functionality for FuriEventLoop.
 */

#pragma once

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible timer types.
 */
typedef enum {
    FuriEventLoopTimerTypeOnce = 0, /**< One-shot timer. */
    FuriEventLoopTimerTypePeriodic = 1, /**< Repeating timer. */
} FuriEventLoopTimerType;

/**
 * @brief Timer callback type for functions to be called when a timer expires.
 *
 * In the timer callback, it is ALLOWED:
 * - To start, stop, or restart an existing timer,
 * - To create new timers using furi_event_loop_timer_alloc(),
 * - To delete timers using furi_event_loop_timer_free().
 *
 * @param[in,out] context pointer to a user-specific object that was provided during timer creation
 */
typedef void (*FuriEventLoopTimerCallback)(void* context);

/**
 * @brief Opaque event loop timer type.
 */
typedef struct FuriEventLoopTimer FuriEventLoopTimer;

/**
 * @brief Create a new event loop timer instance.
 *
 * @param[in,out] instance pointer to the current FuriEventLoop instance
 * @param[in] callback pointer to the callback function to be executed upon timer timeout
 * @param[in] type timer type value to determine its behavior (single-shot or periodic)
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 * @returns pointer to the created timer instance
 */
FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* instance,
    FuriEventLoopTimerCallback callback,
    FuriEventLoopTimerType type,
    void* context);

/**
 * @brief Delete an event loop timer instance.
 *
 * @warning The user code MUST call furi_event_loop_timer_free() on ALL instances
 *          associated with the current event loop BEFORE calling furi_event_loop_free().
 *          The event loop may EITHER be running OR stopped when the timers are being deleted.
 *
 * @param[in,out] timer pointer to the timer instance to be deleted
 */
void furi_event_loop_timer_free(FuriEventLoopTimer* timer);

/**
 * @brief Start a timer or restart it with a new interval.
 *
 * @param[in,out] timer pointer to the timer instance to be (re)started
 * @param[in] interval timer interval in ticks
 */
void furi_event_loop_timer_start(FuriEventLoopTimer* timer, uint32_t interval);

/**
 * @brief Restart a timer with the previously set interval.
 *
 * @param[in,out] timer pointer to the timer instance to be restarted
 */
void furi_event_loop_timer_restart(FuriEventLoopTimer* timer);

/**
 * @brief Stop a timer without firing its callback.
 *
 * It is safe to call this function on an already stopped timer (it will do nothing).
 *
 * @param[in,out] timer pointer to the timer instance to be stopped
 */
void furi_event_loop_timer_stop(FuriEventLoopTimer* timer);

/**
 * @brief Get the time remaining before the timer becomes expires.
 *
 * For stopped or expired timers, this function returns 0.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns remaining time in ticks
 */
uint32_t furi_event_loop_timer_get_remaining_time(const FuriEventLoopTimer* timer);

/**
 * @brief Get the timer interval.
 *
 * @param[in] timer pointer to the timer to be queried
 * @returns timer interval in ticks
 */
uint32_t furi_event_loop_timer_get_interval(const FuriEventLoopTimer* timer);

/**
 * @brief Check if the timer is currently running.
 *
 * A timer is considered running if it has not expired yet.
 * @param[in] timer pointer to the timer to be queried
 * @returns true if the timer is running, false otherwise
 */
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* timer);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/event_loop_timer_i.h">
<content>
#pragma once

#include "event_loop_timer.h"

#include <m-i-list.h>

typedef enum {
    FuriEventLoopTimerRequestNone,
    FuriEventLoopTimerRequestStart,
    FuriEventLoopTimerRequestStop,
    FuriEventLoopTimerRequestFree,
} FuriEventLoopTimerRequest;

struct FuriEventLoopTimer {
    FuriEventLoop* owner;

    FuriEventLoopTimerCallback callback;
    void* context;

    uint32_t interval;
    uint32_t start_time;
    uint32_t next_interval;

    // Interface for the active timer list
    ILIST_INTERFACE(TimerList, FuriEventLoopTimer);

    // Interface for the timer request queue
    ILIST_INTERFACE(TimerQueue, FuriEventLoopTimer);

    FuriEventLoopTimerRequest request;

    bool active;
    bool periodic;
};

ILIST_DEF(TimerList, FuriEventLoopTimer, M_POD_OPLIST)
ILIST_DEF(TimerQueue, FuriEventLoopTimer, M_POD_OPLIST)

uint32_t furi_event_loop_get_timer_wait_time(const FuriEventLoop* instance);

void furi_event_loop_process_timer_queue(FuriEventLoop* instance);

bool furi_event_loop_process_expired_timers(FuriEventLoop* instance);

</content>
</file>
<file name="furi/core/timer.c">
<content>
#include "timer.h"
#include "check.h"
#include "kernel.h"

#include <FreeRTOS.h>
#include <event_groups.h>
#include <timers.h>

struct FuriTimer {
    StaticTimer_t container;
    FuriTimerCallback cb_func;
    void* cb_context;
};

// IMPORTANT: container MUST be the FIRST struct member
static_assert(offsetof(FuriTimer, container) == 0);

#define TIMER_DELETED_EVENT (1U << 0)

static void TimerCallback(TimerHandle_t hTimer) {
    FuriTimer* instance = pvTimerGetTimerID(hTimer);
    furi_check(instance);
    instance->cb_func(instance->cb_context);
}

FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context) {
    furi_check((furi_kernel_is_irq_or_masked() == 0U) && (func != NULL));

    FuriTimer* instance = malloc(sizeof(FuriTimer));

    instance->cb_func = func;
    instance->cb_context = context;

    const UBaseType_t reload = (type == FuriTimerTypeOnce ? pdFALSE : pdTRUE);
    const TimerHandle_t hTimer = xTimerCreateStatic(
        NULL, portMAX_DELAY, reload, instance, TimerCallback, &instance->container);

    furi_check(hTimer == (TimerHandle_t)instance);

    return instance;
}

static void furi_timer_epilogue(void* context, uint32_t arg) {
    furi_assert(context);
    UNUSED(arg);

    EventGroupHandle_t hEvent = context;
    vTaskSuspendAll();
    xEventGroupSetBits(hEvent, TIMER_DELETED_EVENT);
    (void)xTaskResumeAll();
}

void furi_timer_free(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    furi_check(xTimerDelete(hTimer, portMAX_DELAY) == pdPASS);

    StaticEventGroup_t event_container = {};
    EventGroupHandle_t hEvent = xEventGroupCreateStatic(&event_container);
    furi_check(xTimerPendFunctionCall(furi_timer_epilogue, hEvent, 0, portMAX_DELAY) == pdPASS);

    furi_check(
        xEventGroupWaitBits(hEvent, TIMER_DELETED_EVENT, pdFALSE, pdTRUE, portMAX_DELAY) ==
        TIMER_DELETED_EVENT);
    vEventGroupDelete(hEvent);

    free(instance);
}

FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);
    furi_check(ticks < portMAX_DELAY);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS) {
        stat = FuriStatusOk;
    } else {
        stat = FuriStatusErrorResource;
    }

    return stat;
}

FuriStatus furi_timer_restart(FuriTimer* instance, uint32_t ticks) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);
    furi_check(ticks < portMAX_DELAY);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    FuriStatus stat;

    if(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS &&
       xTimerReset(hTimer, portMAX_DELAY) == pdPASS) {
        stat = FuriStatusOk;
    } else {
        stat = FuriStatusErrorResource;
    }

    return stat;
}

FuriStatus furi_timer_stop(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    furi_check(xTimerStop(hTimer, portMAX_DELAY) == pdPASS);

    return FuriStatusOk;
}

uint32_t furi_timer_is_running(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    /* Return 0: not running, 1: running */
    return (uint32_t)xTimerIsTimerActive(hTimer);
}

uint32_t furi_timer_get_expire_time(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;

    return (uint32_t)xTimerGetExpiryTime(hTimer);
}

void furi_timer_pending_callback(FuriTimerPendigCallback callback, void* context, uint32_t arg) {
    furi_check(callback);

    BaseType_t ret = pdFAIL;
    if(furi_kernel_is_irq_or_masked()) {
        ret = xTimerPendFunctionCallFromISR(callback, context, arg, NULL);
    } else {
        ret = xTimerPendFunctionCall(callback, context, arg, FuriWaitForever);
    }

    furi_check(ret == pdPASS);
}

void furi_timer_set_thread_priority(FuriTimerThreadPriority priority) {
    furi_check(!furi_kernel_is_irq_or_masked());

    TaskHandle_t task_handle = xTimerGetTimerDaemonTaskHandle();
    furi_check(task_handle); // Don't call this method before timer task start

    if(priority == FuriTimerThreadPriorityNormal) {
        vTaskPrioritySet(task_handle, configTIMER_TASK_PRIORITY);
    } else if(priority == FuriTimerThreadPriorityElevated) {
        vTaskPrioritySet(task_handle, configMAX_PRIORITIES - 1);
    } else {
        furi_crash();
    }
}

</content>
</file>
<file name="furi/core/timer.h">
<content>
/**
 * @file timer.h
 * @brief Furi software Timer API.
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FuriTimerCallback)(void* context);

typedef enum {
    FuriTimerTypeOnce = 0, ///< One-shot timer.
    FuriTimerTypePeriodic = 1 ///< Repeating timer.
} FuriTimerType;

typedef struct FuriTimer FuriTimer;

/** Allocate timer
 *
 * @param[in]  func     The callback function
 * @param[in]  type     The timer type
 * @param      context  The callback context
 *
 * @return     The pointer to FuriTimer instance
 */
FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context);

/** Free timer
 *
 * @param      instance  The pointer to FuriTimer instance
 */
void furi_timer_free(FuriTimer* instance);

/** Start timer
 *
 * @warning    This is asynchronous call, real operation will happen as soon as
 *             timer service process this request.
 *
 * @param      instance  The pointer to FuriTimer instance
 * @param[in]  ticks     The interval in ticks
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_start(FuriTimer* instance, uint32_t ticks);

/** Restart timer with previous timeout value
 *
 * @warning    This is asynchronous call, real operation will happen as soon as
 *             timer service process this request.
 *
 * @param      instance  The pointer to FuriTimer instance
 * @param[in]  ticks     The interval in ticks
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_restart(FuriTimer* instance, uint32_t ticks);

/** Stop timer
 *
 * @warning    This is asynchronous call, real operation will happen as soon as
 *             timer service process this request.
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     The furi status.
 */
FuriStatus furi_timer_stop(FuriTimer* instance);

/** Is timer running
 *
 * @warning    This cal may and will return obsolete timer state if timer
 *             commands are still in the queue. Please read FreeRTOS timer
 *             documentation first.
 *
 * @param      instance  The pointer to FuriTimer instance
 *
 * @return     0: not running, 1: running
 */
uint32_t furi_timer_is_running(FuriTimer* instance);

/** Get timer expire time
 *
 * @param      instance  The Timer instance
 *
 * @return     expire tick
 */
uint32_t furi_timer_get_expire_time(FuriTimer* instance);

typedef void (*FuriTimerPendigCallback)(void* context, uint32_t arg);

void furi_timer_pending_callback(FuriTimerPendigCallback callback, void* context, uint32_t arg);

typedef enum {
    FuriTimerThreadPriorityNormal, /**< Lower then other threads */
    FuriTimerThreadPriorityElevated, /**< Same as other threads */
} FuriTimerThreadPriority;

/** Set Timer thread priority
 *
 * @param[in]  priority  The priority
 */
void furi_timer_set_thread_priority(FuriTimerThreadPriority priority);

#ifdef __cplusplus
}
#endif

</content>
</file>


