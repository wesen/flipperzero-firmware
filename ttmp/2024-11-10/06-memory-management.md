# Memory Management Context
Memory management subsystem containing:
- Memory Manager: Core memory allocation and tracking
- Heap Implementation: Dynamic memory management
Ensures proper resource allocation and prevents memory leaks.

---

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


<file name="furi/core/memmgr.c">
<content>
#include "memmgr.h"
#include <string.h>
#include <furi_hal_memory.h>

extern void* pvPortMalloc(size_t xSize);
extern void vPortFree(void* pv);
extern size_t xPortGetFreeHeapSize(void);
extern size_t xPortGetTotalHeapSize(void);
extern size_t xPortGetMinimumEverFreeHeapSize(void);

void* malloc(size_t size) {
    return pvPortMalloc(size);
}

void free(void* ptr) {
    vPortFree(ptr);
}

void* realloc(void* ptr, size_t size) {
    if(size == 0) {
        vPortFree(ptr);
        return NULL;
    }

    void* p = pvPortMalloc(size);
    if(ptr != NULL) {
        memcpy(p, ptr, size);
        vPortFree(ptr);
    }

    return p;
}

void* calloc(size_t count, size_t size) {
    return pvPortMalloc(count * size);
}

char* strdup(const char* s) {
    // arg s marked as non-null, so we need hack to check for NULL
    furi_check(((uint32_t)s << 2) != 0);

    size_t siz = strlen(s) + 1;
    char* y = pvPortMalloc(siz);
    memcpy(y, s, siz);

    return y;
}

size_t memmgr_get_free_heap(void) {
    return xPortGetFreeHeapSize();
}

size_t memmgr_get_total_heap(void) {
    return xPortGetTotalHeapSize();
}

size_t memmgr_get_minimum_free_heap(void) {
    return xPortGetMinimumEverFreeHeapSize();
}

void* __wrap__malloc_r(struct _reent* r, size_t size) {
    UNUSED(r);
    return pvPortMalloc(size);
}

void __wrap__free_r(struct _reent* r, void* ptr) {
    UNUSED(r);
    vPortFree(ptr);
}

void* __wrap__calloc_r(struct _reent* r, size_t count, size_t size) {
    UNUSED(r);
    return calloc(count, size);
}

void* __wrap__realloc_r(struct _reent* r, void* ptr, size_t size) {
    UNUSED(r);
    return realloc(ptr, size);
}

void* memmgr_alloc_from_pool(size_t size) {
    void* p = furi_hal_memory_alloc(size);
    if(p == NULL) p = malloc(size);

    return p;
}

size_t memmgr_pool_get_free(void) {
    return furi_hal_memory_get_free();
}

size_t memmgr_pool_get_max_block(void) {
    return furi_hal_memory_max_pool_block();
}

void* aligned_malloc(size_t size, size_t alignment) {
    void* p1; // original block
    void** p2; // aligned block
    int offset = alignment - 1 + sizeof(void*);
    if((p1 = (void*)malloc(size + offset)) == NULL) {
        return NULL;
    }
    p2 = (void**)(((size_t)(p1) + offset) & ~(alignment - 1));
    p2[-1] = p1;
    return p2;
}

void aligned_free(void* p) {
    free(((void**)p)[-1]);
}

</content>
</file>
<file name="furi/core/memmgr.h">
<content>
/**
 * @file memmgr.h
 * Furi: memory management API and glue
 */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"

#ifdef __cplusplus
extern "C" {
#endif

// define for test case "link against furi memmgr"
#define FURI_MEMMGR_GUARD 1

/** Get free heap size
 *
 * @return     free heap size in bytes
 */
size_t memmgr_get_free_heap(void);

/** Get total heap size
 *
 * @return     total heap size in bytes
 */
size_t memmgr_get_total_heap(void);

/** Get heap watermark
 *
 * @return     minimum heap in bytes
 */
size_t memmgr_get_minimum_free_heap(void);

/**
 * An aligned version of malloc, used when you need to get the aligned space on the heap
 * Freeing the received address is performed ONLY through the aligned_free function
 * @param size 
 * @param alignment 
 * @return void* 
 */
void* aligned_malloc(size_t size, size_t alignment);

/**
 * Freed space obtained through the aligned_malloc function
 * @param p pointer to result of aligned_malloc
 */
void aligned_free(void* p);

/**
 * @brief Allocate memory from separate memory pool. That memory can't be freed.
 * 
 * @param size 
 * @return void* 
 */
void* memmgr_alloc_from_pool(size_t size);

/**
 * @brief Get free memory pool size
 * 
 * @return size_t 
 */
size_t memmgr_pool_get_free(void);

/**
 * @brief Get max free block size from memory pool
 * 
 * @return size_t 
 */
size_t memmgr_pool_get_max_block(void);

#ifdef __cplusplus
}
#endif

</content>
</file>
<file name="furi/core/memmgr_heap.c">
<content>
/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

#include "memmgr_heap.h"
#include "check.h"
#include <stdlib.h>
#include <stdio.h>
#include <stm32wbxx.h>
#include <stm32wb55_linker.h>
#include <core/log.h>
#include <core/common_defines.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include <FreeRTOS.h>
#include <task.h>

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#ifdef HEAP_PRINT_DEBUG
#error This feature is broken, logging transport must be replaced with RTT
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE ((size_t)(xHeapStructSize << 1))

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t)8)

/* Heap start end symbols provided by linker */
uint8_t* ucHeap = (uint8_t*)&__heap_start__;

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK* pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize; /*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert);

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit(void);

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) &
                                      ~((size_t)portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

/* Furi heap extension */
#include <m-dict.h>

/* Allocation tracking types */
DICT_DEF2(MemmgrHeapAllocDict, uint32_t, uint32_t) //-V1048

DICT_DEF2( //-V1048
    MemmgrHeapThreadDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    MemmgrHeapAllocDict_t,
    DICT_OPLIST(MemmgrHeapAllocDict))

/* Thread allocation tracing storage */
static MemmgrHeapThreadDict_t memmgr_heap_thread_dict = {0};
static volatile uint32_t memmgr_heap_thread_trace_depth = 0;

/* Initialize tracing storage on start */
void memmgr_heap_init(void) {
    MemmgrHeapThreadDict_init(memmgr_heap_thread_dict);
}

void memmgr_heap_enable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id) == NULL);
        MemmgrHeapAllocDict_t alloc_dict;
        MemmgrHeapAllocDict_init(alloc_dict);
        MemmgrHeapThreadDict_set_at(memmgr_heap_thread_dict, (uint32_t)thread_id, alloc_dict);
        MemmgrHeapAllocDict_clear(alloc_dict);
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

void memmgr_heap_disable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_erase(memmgr_heap_thread_dict, (uint32_t)thread_id));
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

size_t memmgr_heap_get_thread_memory(FuriThreadId thread_id) {
    size_t leftovers = MEMMGR_HEAP_UNKNOWN;
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            leftovers = 0;
            MemmgrHeapAllocDict_it_t alloc_dict_it;
            for(MemmgrHeapAllocDict_it(alloc_dict_it, *alloc_dict);
                !MemmgrHeapAllocDict_end_p(alloc_dict_it);
                MemmgrHeapAllocDict_next(alloc_dict_it)) {
                MemmgrHeapAllocDict_itref_t* data = MemmgrHeapAllocDict_ref(alloc_dict_it);
                if(data->key != 0) {
                    uint8_t* puc = (uint8_t*)data->key;
                    puc -= xHeapStructSize;
                    BlockLink_t* pxLink = (void*)puc;

                    if((pxLink->xBlockSize & xBlockAllocatedBit) != 0 &&
                       pxLink->pxNextFreeBlock == NULL) {
                        leftovers += data->value;
                    }
                }
            }
        }
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
    return leftovers;
}

#undef traceMALLOC
static inline void traceMALLOC(void* pointer, size_t size) {
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            MemmgrHeapAllocDict_set_at(*alloc_dict, (uint32_t)pointer, (uint32_t)size);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

#undef traceFREE
static inline void traceFREE(void* pointer, size_t size) {
    UNUSED(size);
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            // In some cases thread may want to release memory that was not allocated by it
            const bool res = MemmgrHeapAllocDict_erase(*alloc_dict, (uint32_t)pointer);
            UNUSED(res);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

size_t memmgr_heap_get_max_free_block(void) {
    size_t max_free_size = 0;
    BlockLink_t* pxBlock;
    vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        if(pxBlock->xBlockSize > max_free_size) {
            max_free_size = pxBlock->xBlockSize;
        }
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    xTaskResumeAll();
    return max_free_size;
}

void memmgr_heap_printf_free_blocks(void) {
    BlockLink_t* pxBlock;
    //can be enabled once we can do printf with a locked scheduler
    //vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        printf("A %p S %lu\r\n", (void*)pxBlock, (uint32_t)pxBlock->xBlockSize);
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    //xTaskResumeAll();
}

#ifdef HEAP_PRINT_DEBUG
char* ultoa(unsigned long num, char* str, int radix) {
    char temp[33]; // at radix 2 the string is at most 32 + 1 null long.
    int temp_loc = 0;
    int digit;
    int str_loc = 0;

    //construct a backward string of the number.
    do {
        digit = (unsigned long)num % ((unsigned long)radix);
        if(digit < 10)
            temp[temp_loc++] = digit + '0';
        else
            temp[temp_loc++] = digit - 10 + 'A';
        num = ((unsigned long)num) / ((unsigned long)radix);
    } while((unsigned long)num > 0);

    temp_loc--;

    //now reverse the string.
    while(temp_loc >= 0) { // while there are still chars
        str[str_loc++] = temp[temp_loc--];
    }
    str[str_loc] = 0; // add null termination.

    return str;
}

static void print_heap_init(void) {
    char tmp_str[33];
    size_t heap_start = (size_t)&__heap_start__;
    size_t heap_end = (size_t)&__heap_end__;

    // {PHStart|heap_start|heap_end}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{PHStart|");
    ultoa(heap_start, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    ultoa(heap_end, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_malloc(void* ptr, size_t size) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|m|address|size}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|m|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    utoa(size, tmp_str, 10);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_free(void* ptr) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|f|address}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|f|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}
#endif
/*-----------------------------------------------------------*/

void* pvPortMalloc(size_t xWantedSize) {
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void* pvReturn = NULL;
    size_t to_wipe = xWantedSize;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

#ifdef HEAP_PRINT_DEBUG
    BlockLink_t* print_heap_block = NULL;
#endif

    /* If this is the first call to malloc then the heap will require
        initialisation to setup the list of free blocks. */
    if(pxEnd == NULL) {
#ifdef HEAP_PRINT_DEBUG
        print_heap_init();
#endif

        vTaskSuspendAll();
        {
            prvHeapInit();
            memmgr_heap_init();
        }
        (void)xTaskResumeAll();
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    vTaskSuspendAll();
    {
        /* Check the requested block size is not so large that the top bit is
        set.  The top bit of the block size member of the BlockLink_t structure
        is used to determine who owns the block - the application or the
        kernel, so it must be free. */
        if((xWantedSize & xBlockAllocatedBit) == 0) {
            /* The wanted size is increased so it can contain a BlockLink_t
            structure in addition to the requested amount of bytes. */
            if(xWantedSize > 0) {
                xWantedSize += xHeapStructSize;

                /* Ensure that blocks are always aligned to the required number
                of bytes. */
                if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00) {
                    /* Byte alignment required. */
                    xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
                    configASSERT((xWantedSize & portBYTE_ALIGNMENT_MASK) == 0);
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }

            if((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining)) {
                /* Traverse the list from the start (lowest address) block until
                one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;
                while((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL)) {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                was not found. */
                if(pxBlock != pxEnd) {
                    /* Return the memory space pointed to - jumping over the
                    BlockLink_t structure at its start. */
                    pvReturn =
                        (void*)(((uint8_t*)pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* This block is being returned for use so must be taken out
                    of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                    two. */
                    if((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE) {
                        /* This block is to be split into two.  Create a new
                        block following the number of bytes requested. The void
                        cast is used to prevent byte alignment warnings from the
                        compiler. */
                        pxNewBlockLink = (void*)(((uint8_t*)pxBlock) + xWantedSize);
                        configASSERT((((size_t)pxNewBlockLink) & portBYTE_ALIGNMENT_MASK) == 0);

                        /* Calculate the sizes of two blocks split from the
                        single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if(xFreeBytesRemaining < xMinimumEverFreeBytesRemaining) {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                    by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;

#ifdef HEAP_PRINT_DEBUG
                    print_heap_block = pxBlock;
#endif
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC(pvReturn, xWantedSize);
    }
    (void)xTaskResumeAll();

#ifdef HEAP_PRINT_DEBUG
    print_heap_malloc(print_heap_block, print_heap_block->xBlockSize & ~xBlockAllocatedBit);
#endif

#if(configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if(pvReturn == NULL) {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif

    configASSERT((((size_t)pvReturn) & (size_t)portBYTE_ALIGNMENT_MASK) == 0);

    furi_check(pvReturn, xWantedSize ? "out of memory" : "malloc(0)");
    pvReturn = memset(pvReturn, 0, to_wipe);
    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree(void* pv) {
    uint8_t* puc = (uint8_t*)pv;
    BlockLink_t* pxLink;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

    if(pv != NULL) {
        /* The memory being freed will have an BlockLink_t structure immediately
        before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void*)puc;

        /* Check the block is actually allocated. */
        configASSERT((pxLink->xBlockSize & xBlockAllocatedBit) != 0);
        configASSERT(pxLink->pxNextFreeBlock == NULL);

        if((pxLink->xBlockSize & xBlockAllocatedBit) != 0) {
            if(pxLink->pxNextFreeBlock == NULL) {
                /* The block is being returned to the heap - it is no longer
                allocated. */
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

#ifdef HEAP_PRINT_DEBUG
                print_heap_free(pxLink);
#endif

                vTaskSuspendAll();
                {
                    furi_assert((size_t)pv >= SRAM_BASE);
                    furi_assert((size_t)pv < SRAM_BASE + 1024 * 256);
                    furi_assert(pxLink->xBlockSize >= xHeapStructSize);
                    furi_assert((pxLink->xBlockSize - xHeapStructSize) < 1024 * 256);

                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    traceFREE(pv, pxLink->xBlockSize);
                    memset(pv, 0, pxLink->xBlockSize - xHeapStructSize);
                    prvInsertBlockIntoFreeList((BlockLink_t*)pxLink);
                }
                (void)xTaskResumeAll();
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    } else {
#ifdef HEAP_PRINT_DEBUG
        print_heap_free(pv);
#endif
    }
}
/*-----------------------------------------------------------*/

size_t xPortGetTotalHeapSize(void) {
    return (size_t)&__heap_end__ - (size_t)&__heap_start__;
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize(void) {
    return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize(void) {
    return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks(void) {
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit(void) {
    BlockLink_t* pxFirstFreeBlock;
    uint8_t* pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = (size_t)&__heap_end__ - (size_t)&__heap_start__;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (size_t)ucHeap;

    if((uxAddress & portBYTE_ALIGNMENT_MASK) != 0) {
        uxAddress += (portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (size_t)ucHeap;
    }

    pucAlignedHeap = (uint8_t*)uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void*)pucAlignedHeap;
    xStart.xBlockSize = (size_t)0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
    pxEnd = (void*)uxAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (void*)pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = uxAddress - (size_t)pxFirstFreeBlock;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) {
    BlockLink_t* pxIterator;
    uint8_t* puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for(pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
        pxIterator = pxIterator->pxNextFreeBlock) {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = (uint8_t*)pxIterator;
    if((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert) {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = (uint8_t*)pxBlockToInsert;
    if((puc + pxBlockToInsert->xBlockSize) == (uint8_t*)pxIterator->pxNextFreeBlock) {
        if(pxIterator->pxNextFreeBlock != pxEnd) {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        } else {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    } else {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if(pxIterator != pxBlockToInsert) {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }
}

</content>
</file>
<file name="furi/core/memmgr_heap.h">
<content>
/**
 * @file memmgr_heap.h
 * Furi: heap memory management API and allocator
 */

#pragma once

#include <stdint.h>
#include <core/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMMGR_HEAP_UNKNOWN 0xFFFFFFFF

/** Memmgr heap enable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_enable_thread_trace(FuriThreadId thread_id);

/** Memmgr heap disable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_disable_thread_trace(FuriThreadId thread_id);

/** Memmgr heap get allocatred thread memory
 *
 * @param      thread_id  - thread id to track
 *
 * @return     bytes allocated right now
 */
size_t memmgr_heap_get_thread_memory(FuriThreadId thread_id);

/** Memmgr heap get the max contiguous block size on the heap
 *
 * @return     size_t max contiguous block size
 */
size_t memmgr_heap_get_max_free_block(void);

/** Print the address and size of all free blocks to stdout
 */
void memmgr_heap_printf_free_blocks(void);

#ifdef __cplusplus
}
#endif

</content>
</file>

<file name="furi/core/memmgr_heap.c">
<content>
/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

#include "memmgr_heap.h"
#include "check.h"
#include <stdlib.h>
#include <stdio.h>
#include <stm32wbxx.h>
#include <stm32wb55_linker.h>
#include <core/log.h>
#include <core/common_defines.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include <FreeRTOS.h>
#include <task.h>

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#ifdef HEAP_PRINT_DEBUG
#error This feature is broken, logging transport must be replaced with RTT
#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE ((size_t)(xHeapStructSize << 1))

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t)8)

/* Heap start end symbols provided by linker */
uint8_t* ucHeap = (uint8_t*)&__heap_start__;

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK* pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize; /*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert);

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit(void);

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) &
                                      ~((size_t)portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;

/* Furi heap extension */
#include <m-dict.h>

/* Allocation tracking types */
DICT_DEF2(MemmgrHeapAllocDict, uint32_t, uint32_t) //-V1048

DICT_DEF2( //-V1048
    MemmgrHeapThreadDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    MemmgrHeapAllocDict_t,
    DICT_OPLIST(MemmgrHeapAllocDict))

/* Thread allocation tracing storage */
static MemmgrHeapThreadDict_t memmgr_heap_thread_dict = {0};
static volatile uint32_t memmgr_heap_thread_trace_depth = 0;

/* Initialize tracing storage on start */
void memmgr_heap_init(void) {
    MemmgrHeapThreadDict_init(memmgr_heap_thread_dict);
}

void memmgr_heap_enable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id) == NULL);
        MemmgrHeapAllocDict_t alloc_dict;
        MemmgrHeapAllocDict_init(alloc_dict);
        MemmgrHeapThreadDict_set_at(memmgr_heap_thread_dict, (uint32_t)thread_id, alloc_dict);
        MemmgrHeapAllocDict_clear(alloc_dict);
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

void memmgr_heap_disable_thread_trace(FuriThreadId thread_id) {
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        furi_check(MemmgrHeapThreadDict_erase(memmgr_heap_thread_dict, (uint32_t)thread_id));
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
}

size_t memmgr_heap_get_thread_memory(FuriThreadId thread_id) {
    size_t leftovers = MEMMGR_HEAP_UNKNOWN;
    vTaskSuspendAll();
    {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            leftovers = 0;
            MemmgrHeapAllocDict_it_t alloc_dict_it;
            for(MemmgrHeapAllocDict_it(alloc_dict_it, *alloc_dict);
                !MemmgrHeapAllocDict_end_p(alloc_dict_it);
                MemmgrHeapAllocDict_next(alloc_dict_it)) {
                MemmgrHeapAllocDict_itref_t* data = MemmgrHeapAllocDict_ref(alloc_dict_it);
                if(data->key != 0) {
                    uint8_t* puc = (uint8_t*)data->key;
                    puc -= xHeapStructSize;
                    BlockLink_t* pxLink = (void*)puc;

                    if((pxLink->xBlockSize & xBlockAllocatedBit) != 0 &&
                       pxLink->pxNextFreeBlock == NULL) {
                        leftovers += data->value;
                    }
                }
            }
        }
        memmgr_heap_thread_trace_depth--;
    }
    (void)xTaskResumeAll();
    return leftovers;
}

#undef traceMALLOC
static inline void traceMALLOC(void* pointer, size_t size) {
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            MemmgrHeapAllocDict_set_at(*alloc_dict, (uint32_t)pointer, (uint32_t)size);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

#undef traceFREE
static inline void traceFREE(void* pointer, size_t size) {
    UNUSED(size);
    FuriThreadId thread_id = furi_thread_get_current_id();
    if(thread_id && memmgr_heap_thread_trace_depth == 0) {
        memmgr_heap_thread_trace_depth++;
        MemmgrHeapAllocDict_t* alloc_dict =
            MemmgrHeapThreadDict_get(memmgr_heap_thread_dict, (uint32_t)thread_id);
        if(alloc_dict) {
            // In some cases thread may want to release memory that was not allocated by it
            const bool res = MemmgrHeapAllocDict_erase(*alloc_dict, (uint32_t)pointer);
            UNUSED(res);
        }
        memmgr_heap_thread_trace_depth--;
    }
}

size_t memmgr_heap_get_max_free_block(void) {
    size_t max_free_size = 0;
    BlockLink_t* pxBlock;
    vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        if(pxBlock->xBlockSize > max_free_size) {
            max_free_size = pxBlock->xBlockSize;
        }
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    xTaskResumeAll();
    return max_free_size;
}

void memmgr_heap_printf_free_blocks(void) {
    BlockLink_t* pxBlock;
    //can be enabled once we can do printf with a locked scheduler
    //vTaskSuspendAll();

    pxBlock = xStart.pxNextFreeBlock;
    while(pxBlock->pxNextFreeBlock != NULL) {
        printf("A %p S %lu\r\n", (void*)pxBlock, (uint32_t)pxBlock->xBlockSize);
        pxBlock = pxBlock->pxNextFreeBlock;
    }

    //xTaskResumeAll();
}

#ifdef HEAP_PRINT_DEBUG
char* ultoa(unsigned long num, char* str, int radix) {
    char temp[33]; // at radix 2 the string is at most 32 + 1 null long.
    int temp_loc = 0;
    int digit;
    int str_loc = 0;

    //construct a backward string of the number.
    do {
        digit = (unsigned long)num % ((unsigned long)radix);
        if(digit < 10)
            temp[temp_loc++] = digit + '0';
        else
            temp[temp_loc++] = digit - 10 + 'A';
        num = ((unsigned long)num) / ((unsigned long)radix);
    } while((unsigned long)num > 0);

    temp_loc--;

    //now reverse the string.
    while(temp_loc >= 0) { // while there are still chars
        str[str_loc++] = temp[temp_loc--];
    }
    str[str_loc] = 0; // add null termination.

    return str;
}

static void print_heap_init(void) {
    char tmp_str[33];
    size_t heap_start = (size_t)&__heap_start__;
    size_t heap_end = (size_t)&__heap_end__;

    // {PHStart|heap_start|heap_end}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{PHStart|");
    ultoa(heap_start, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    ultoa(heap_end, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_malloc(void* ptr, size_t size) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|m|address|size}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|m|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("|");
    utoa(size, tmp_str, 10);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}

static void print_heap_free(void* ptr) {
    char tmp_str[33];
    const char* name = furi_thread_get_name(furi_thread_get_current_id());
    if(!name) {
        name = "";
    }

    // {thread name|f|address}
    FURI_CRITICAL_ENTER();
    furi_log_puts("{");
    furi_log_puts(name);
    furi_log_puts("|f|0x");
    ultoa((unsigned long)ptr, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts("}\r\n");
    FURI_CRITICAL_EXIT();
}
#endif
/*-----------------------------------------------------------*/

void* pvPortMalloc(size_t xWantedSize) {
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void* pvReturn = NULL;
    size_t to_wipe = xWantedSize;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

#ifdef HEAP_PRINT_DEBUG
    BlockLink_t* print_heap_block = NULL;
#endif

    /* If this is the first call to malloc then the heap will require
        initialisation to setup the list of free blocks. */
    if(pxEnd == NULL) {
#ifdef HEAP_PRINT_DEBUG
        print_heap_init();
#endif

        vTaskSuspendAll();
        {
            prvHeapInit();
            memmgr_heap_init();
        }
        (void)xTaskResumeAll();
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    vTaskSuspendAll();
    {
        /* Check the requested block size is not so large that the top bit is
        set.  The top bit of the block size member of the BlockLink_t structure
        is used to determine who owns the block - the application or the
        kernel, so it must be free. */
        if((xWantedSize & xBlockAllocatedBit) == 0) {
            /* The wanted size is increased so it can contain a BlockLink_t
            structure in addition to the requested amount of bytes. */
            if(xWantedSize > 0) {
                xWantedSize += xHeapStructSize;

                /* Ensure that blocks are always aligned to the required number
                of bytes. */
                if((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00) {
                    /* Byte alignment required. */
                    xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
                    configASSERT((xWantedSize & portBYTE_ALIGNMENT_MASK) == 0);
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }

            if((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining)) {
                /* Traverse the list from the start (lowest address) block until
                one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;
                while((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL)) {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                was not found. */
                if(pxBlock != pxEnd) {
                    /* Return the memory space pointed to - jumping over the
                    BlockLink_t structure at its start. */
                    pvReturn =
                        (void*)(((uint8_t*)pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* This block is being returned for use so must be taken out
                    of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                    two. */
                    if((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE) {
                        /* This block is to be split into two.  Create a new
                        block following the number of bytes requested. The void
                        cast is used to prevent byte alignment warnings from the
                        compiler. */
                        pxNewBlockLink = (void*)(((uint8_t*)pxBlock) + xWantedSize);
                        configASSERT((((size_t)pxNewBlockLink) & portBYTE_ALIGNMENT_MASK) == 0);

                        /* Calculate the sizes of two blocks split from the
                        single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if(xFreeBytesRemaining < xMinimumEverFreeBytesRemaining) {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    } else {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                    by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;

#ifdef HEAP_PRINT_DEBUG
                    print_heap_block = pxBlock;
#endif
                } else {
                    mtCOVERAGE_TEST_MARKER();
                }
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC(pvReturn, xWantedSize);
    }
    (void)xTaskResumeAll();

#ifdef HEAP_PRINT_DEBUG
    print_heap_malloc(print_heap_block, print_heap_block->xBlockSize & ~xBlockAllocatedBit);
#endif

#if(configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if(pvReturn == NULL) {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif

    configASSERT((((size_t)pvReturn) & (size_t)portBYTE_ALIGNMENT_MASK) == 0);

    furi_check(pvReturn, xWantedSize ? "out of memory" : "malloc(0)");
    pvReturn = memset(pvReturn, 0, to_wipe);
    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree(void* pv) {
    uint8_t* puc = (uint8_t*)pv;
    BlockLink_t* pxLink;

    if(FURI_IS_IRQ_MODE()) {
        furi_crash("memmgt in ISR");
    }

    if(pv != NULL) {
        /* The memory being freed will have an BlockLink_t structure immediately
        before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void*)puc;

        /* Check the block is actually allocated. */
        configASSERT((pxLink->xBlockSize & xBlockAllocatedBit) != 0);
        configASSERT(pxLink->pxNextFreeBlock == NULL);

        if((pxLink->xBlockSize & xBlockAllocatedBit) != 0) {
            if(pxLink->pxNextFreeBlock == NULL) {
                /* The block is being returned to the heap - it is no longer
                allocated. */
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

#ifdef HEAP_PRINT_DEBUG
                print_heap_free(pxLink);
#endif

                vTaskSuspendAll();
                {
                    furi_assert((size_t)pv >= SRAM_BASE);
                    furi_assert((size_t)pv < SRAM_BASE + 1024 * 256);
                    furi_assert(pxLink->xBlockSize >= xHeapStructSize);
                    furi_assert((pxLink->xBlockSize - xHeapStructSize) < 1024 * 256);

                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    traceFREE(pv, pxLink->xBlockSize);
                    memset(pv, 0, pxLink->xBlockSize - xHeapStructSize);
                    prvInsertBlockIntoFreeList((BlockLink_t*)pxLink);
                }
                (void)xTaskResumeAll();
            } else {
                mtCOVERAGE_TEST_MARKER();
            }
        } else {
            mtCOVERAGE_TEST_MARKER();
        }
    } else {
#ifdef HEAP_PRINT_DEBUG
        print_heap_free(pv);
#endif
    }
}
/*-----------------------------------------------------------*/

size_t xPortGetTotalHeapSize(void) {
    return (size_t)&__heap_end__ - (size_t)&__heap_start__;
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize(void) {
    return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize(void) {
    return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks(void) {
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit(void) {
    BlockLink_t* pxFirstFreeBlock;
    uint8_t* pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = (size_t)&__heap_end__ - (size_t)&__heap_start__;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (size_t)ucHeap;

    if((uxAddress & portBYTE_ALIGNMENT_MASK) != 0) {
        uxAddress += (portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (size_t)ucHeap;
    }

    pucAlignedHeap = (uint8_t*)uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void*)pucAlignedHeap;
    xStart.xBlockSize = (size_t)0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((size_t)portBYTE_ALIGNMENT_MASK);
    pxEnd = (void*)uxAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (void*)pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = uxAddress - (size_t)pxFirstFreeBlock;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t* pxBlockToInsert) {
    BlockLink_t* pxIterator;
    uint8_t* puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for(pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
        pxIterator = pxIterator->pxNextFreeBlock) {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = (uint8_t*)pxIterator;
    if((puc + pxIterator->xBlockSize) == (uint8_t*)pxBlockToInsert) {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = (uint8_t*)pxBlockToInsert;
    if((puc + pxBlockToInsert->xBlockSize) == (uint8_t*)pxIterator->pxNextFreeBlock) {
        if(pxIterator->pxNextFreeBlock != pxEnd) {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        } else {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    } else {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if(pxIterator != pxBlockToInsert) {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    } else {
        mtCOVERAGE_TEST_MARKER();
    }
}

</content>
</file>
<file name="furi/core/memmgr_heap.h">
<content>
/**
 * @file memmgr_heap.h
 * Furi: heap memory management API and allocator
 */

#pragma once

#include <stdint.h>
#include <core/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMMGR_HEAP_UNKNOWN 0xFFFFFFFF

/** Memmgr heap enable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_enable_thread_trace(FuriThreadId thread_id);

/** Memmgr heap disable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_disable_thread_trace(FuriThreadId thread_id);

/** Memmgr heap get allocatred thread memory
 *
 * @param      thread_id  - thread id to track
 *
 * @return     bytes allocated right now
 */
size_t memmgr_heap_get_thread_memory(FuriThreadId thread_id);

/** Memmgr heap get the max contiguous block size on the heap
 *
 * @return     size_t max contiguous block size
 */
size_t memmgr_heap_get_max_free_block(void);

/** Print the address and size of all free blocks to stdout
 */
void memmgr_heap_printf_free_blocks(void);

#ifdef __cplusplus
}
#endif

</content>
</file>


