# Record System Context
Service management system handling:
- System service registration (RECORD_*)
- Service acquisition (furi_record_open)
- Service release (furi_record_close)
Manages singleton services like GUI, Storage, and Notifications.

---

<file name="furi/core/record.c">
<content>
#include "record.h"
#include "check.h"
#include "mutex.h"
#include "event_flag.h"

#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>

#define FURI_RECORD_FLAG_READY (0x1)

typedef struct {
    FuriEventFlag* flags;
    void* data;
    size_t holders_count;
} FuriRecordData;

DICT_DEF2(FuriRecordDataDict, const char*, M_CSTR_DUP_OPLIST, FuriRecordData, M_POD_OPLIST)

typedef struct {
    FuriMutex* mutex;
    FuriRecordDataDict_t records;
} FuriRecord;

static FuriRecord* furi_record = NULL;

static FuriRecordData* furi_record_get(const char* name) {
    return FuriRecordDataDict_get(furi_record->records, name);
}

static void furi_record_put(const char* name, FuriRecordData* record_data) {
    FuriRecordDataDict_set_at(furi_record->records, name, *record_data);
}

static void furi_record_erase(const char* name, FuriRecordData* record_data) {
    furi_event_flag_free(record_data->flags);
    FuriRecordDataDict_erase(furi_record->records, name);
}

void furi_record_init(void) {
    furi_record = malloc(sizeof(FuriRecord));
    furi_record->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    FuriRecordDataDict_init(furi_record->records);
}

static FuriRecordData* furi_record_data_get_or_create(const char* name) {
    furi_check(furi_record);
    FuriRecordData* record_data = furi_record_get(name);
    if(!record_data) {
        FuriRecordData new_record;
        new_record.flags = furi_event_flag_alloc();
        new_record.data = NULL;
        new_record.holders_count = 0;
        furi_record_put(name, &new_record);
        record_data = furi_record_get(name);
    }
    return record_data;
}

static void furi_record_lock(void) {
    furi_check(furi_mutex_acquire(furi_record->mutex, FuriWaitForever) == FuriStatusOk);
}

static void furi_record_unlock(void) {
    furi_check(furi_mutex_release(furi_record->mutex) == FuriStatusOk);
}

bool furi_record_exists(const char* name) {
    furi_check(furi_record);
    furi_check(name);

    bool ret = false;

    furi_record_lock();
    ret = (furi_record_get(name) != NULL);
    furi_record_unlock();

    return ret;
}

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

bool furi_record_destroy(const char* name) {
    furi_check(furi_record);
    furi_check(name);

    bool ret = false;

    furi_record_lock();

    FuriRecordData* record_data = furi_record_get(name);
    furi_check(record_data);
    if(record_data->holders_count == 0) {
        furi_record_erase(name, record_data);
        ret = true;
    }

    furi_record_unlock();

    return ret;
}

void* furi_record_open(const char* name) {
    furi_check(furi_record);
    furi_check(name);

    furi_record_lock();

    FuriRecordData* record_data = furi_record_data_get_or_create(name);
    record_data->holders_count++;

    furi_record_unlock();

    // Wait for record to become ready
    furi_check(
        furi_event_flag_wait(
            record_data->flags,
            FURI_RECORD_FLAG_READY,
            FuriFlagWaitAny | FuriFlagNoClear,
            FuriWaitForever) == FURI_RECORD_FLAG_READY);

    return record_data->data;
}

void furi_record_close(const char* name) {
    furi_check(furi_record);
    furi_check(name);

    furi_record_lock();

    FuriRecordData* record_data = furi_record_get(name);
    furi_check(record_data);
    record_data->holders_count--;

    furi_record_unlock();
}

</content>
</file>
<file name="furi/core/record.h">
<content>
/**
 * @file record.h
 * Furi: record API
 */

#pragma once

#include <stdbool.h>
#include "core_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize record storage For internal use only.
 */
void furi_record_init(void);

/** Check if record exists
 *
 * @param      name  record name
 * @note       Thread safe. Create and destroy must be executed from the same
 *             thread.
 */
bool furi_record_exists(const char* name);

/** Create record
 *
 * @param      name  record name
 * @param      data  data pointer
 * @note       Thread safe. Create and destroy must be executed from the same
 *             thread.
 */
void furi_record_create(const char* name, void* data);

/** Destroy record
 *
 * @param      name  record name
 *
 * @return     true if successful, false if still have holders or thread is not
 *             owner.
 * @note       Thread safe. Create and destroy must be executed from the same
 *             thread.
 */
bool furi_record_destroy(const char* name);

/** Open record
 *
 * @param      name  record name
 *
 * @return     pointer to the record
 * @note       Thread safe. Open and close must be executed from the same
 *             thread. Suspends caller thread till record is available
 */
FURI_RETURNS_NONNULL void* furi_record_open(const char* name);

/** Close record
 *
 * @param      name  record name
 * @note       Thread safe. Open and close must be executed from the same
 *             thread.
 */
void furi_record_close(const char* name);

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

<file name="furi/core/event_flag.c">
<content>
#include "event_flag.h"
#include "common_defines.h"
#include "check.h"

#include <FreeRTOS.h>
#include <event_groups.h>

#include "event_loop_link_i.h"

#define FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS 24U
#define FURI_EVENT_FLAG_VALID_BITS            ((1UL << FURI_EVENT_FLAG_MAX_BITS_EVENT_GROUPS) - 1U)
#define FURI_EVENT_FLAG_INVALID_BITS          (~(FURI_EVENT_FLAG_VALID_BITS))

struct FuriEventFlag {
    StaticEventGroup_t container;
    FuriEventLoopLink event_loop_link;
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

    // Event Loop must be disconnected
    furi_check(!instance->event_loop_link.item_in);
    furi_check(!instance->event_loop_link.item_out);

    vEventGroupDelete((EventGroupHandle_t)instance);
    free(instance);
}

uint32_t furi_event_flag_set(FuriEventFlag* instance, uint32_t flags) {
    furi_check(instance);
    furi_check((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;
    BaseType_t yield;

    FURI_CRITICAL_ENTER();

    if(FURI_IS_IRQ_MODE()) {
        yield = pdFALSE;
        if(xEventGroupSetBitsFromISR(hEventGroup, (EventBits_t)flags, &yield) == pdFAIL) {
            rflags = (uint32_t)FuriFlagErrorResource;
        } else {
            rflags = flags;
            portYIELD_FROM_ISR(yield);
        }
    } else {
        rflags = xEventGroupSetBits(hEventGroup, (EventBits_t)flags);
    }

    if(rflags & flags) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventIn);
    }

    FURI_CRITICAL_EXIT();

    /* Return event flags after setting */
    return rflags;
}

uint32_t furi_event_flag_clear(FuriEventFlag* instance, uint32_t flags) {
    furi_check(instance);
    furi_check((flags & FURI_EVENT_FLAG_INVALID_BITS) == 0U);

    EventGroupHandle_t hEventGroup = (EventGroupHandle_t)instance;
    uint32_t rflags;

    FURI_CRITICAL_ENTER();
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

    if(rflags & flags) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
    }
    FURI_CRITICAL_EXIT();

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

    if((rflags & FuriFlagError) == 0U) {
        furi_event_loop_link_notify(&instance->event_loop_link, FuriEventLoopEventOut);
    }

    /* Return event flags before clearing */
    return rflags;
}

static FuriEventLoopLink* furi_event_flag_event_loop_get_link(FuriEventLoopObject* object) {
    FuriEventFlag* instance = object;
    furi_assert(instance);
    return &instance->event_loop_link;
}

static bool
    furi_event_flag_event_loop_get_level(FuriEventLoopObject* object, FuriEventLoopEvent event) {
    FuriEventFlag* instance = object;
    furi_assert(instance);

    if(event == FuriEventLoopEventIn) {
        return (furi_event_flag_get(instance) & FURI_EVENT_FLAG_VALID_BITS);
    } else if(event == FuriEventLoopEventOut) {
        return (furi_event_flag_get(instance) & FURI_EVENT_FLAG_VALID_BITS) !=
               FURI_EVENT_FLAG_VALID_BITS;
    } else {
        furi_crash();
    }
}

const FuriEventLoopContract furi_event_flag_event_loop_contract = {
    .get_link = furi_event_flag_event_loop_get_link,
    .get_level = furi_event_flag_event_loop_get_level,
};

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
    furi_event_loop_process_edge_event(FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status = FuriEventLoopProcessStatusComplete;
    item->callback(item->object, item->callback_context);

    return status;
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_process_level_event(FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status = FuriEventLoopProcessStatusComplete;
    if(item->contract->get_level(item->object, item->event)) {
        item->callback(item->object, item->callback_context);

        if(item->contract->get_level(item->object, item->event)) {
            status = FuriEventLoopProcessStatusIncomplete;
        }
    }

    return status;
}

static inline FuriEventLoopProcessStatus
    furi_event_loop_process_event(FuriEventLoop* instance, FuriEventLoopItem* item) {
    FuriEventLoopProcessStatus status;

    if(item->event & FuriEventLoopEventFlagOnce) {
        furi_event_loop_unsubscribe(instance, item->object);
    }

    if(item->event & FuriEventLoopEventFlagEdge) {
        status = furi_event_loop_process_edge_event(item);
    } else {
        status = furi_event_loop_process_level_event(item);
    }

    if(item->owner == NULL) {
        status = FuriEventLoopProcessStatusFreeLater;
    }

    return status;
}

static inline FuriEventLoopItem* furi_event_loop_get_waiting_item(FuriEventLoop* instance) {
    FuriEventLoopItem* item = NULL;

    FURI_CRITICAL_ENTER();

    if(!WaitingList_empty_p(instance->waiting_list)) {
        item = WaitingList_pop_front(instance->waiting_list);
        WaitingList_init_field(item);
    }

    FURI_CRITICAL_EXIT();

    return item;
}

static inline void furi_event_loop_sync_flags(FuriEventLoop* instance) {
    FURI_CRITICAL_ENTER();

    if(!WaitingList_empty_p(instance->waiting_list)) {
        xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id,
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
            FuriEventLoopFlagEvent,
            eSetBits);
    }

    FURI_CRITICAL_EXIT();
}

static void furi_event_loop_process_waiting_list(FuriEventLoop* instance) {
    FuriEventLoopItem* item = furi_event_loop_get_waiting_item(instance);
    if(!item) return;

    FuriEventLoopProcessStatus status = furi_event_loop_process_event(instance, item);

    if(status == FuriEventLoopProcessStatusComplete) {
        // Event processing complete, do nothing
    } else if(status == FuriEventLoopProcessStatusIncomplete) {
        // Event processing incomplete, put item back in waiting list
        furi_event_loop_item_notify(item);
    } else if(status == FuriEventLoopProcessStatusFreeLater) { //-V547
        // Unsubscribed from inside the callback, delete item
        furi_event_loop_item_free(item);
    } else {
        furi_crash();
    }

    furi_event_loop_sync_flags(instance);
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

static void furi_event_loop_notify(FuriEventLoop* instance, FuriEventLoopFlag flag) {
    if(FURI_IS_IRQ_MODE()) {
        BaseType_t yield = pdFALSE;

        (void)xTaskNotifyIndexedFromISR(
            (TaskHandle_t)instance->thread_id,
            FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX,
            flag,
            eSetBits,
            &yield);

        portYIELD_FROM_ISR(yield);

    } else {
        (void)xTaskNotifyIndexed(
            (TaskHandle_t)instance->thread_id, FURI_EVENT_LOOP_FLAG_NOTIFY_INDEX, flag, eSetBits);
    }
}

void furi_event_loop_stop(FuriEventLoop* instance) {
    furi_check(instance);
    furi_event_loop_notify(instance, FuriEventLoopFlagStop);
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

    furi_event_loop_notify(instance, FuriEventLoopFlagPending);
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

void furi_event_loop_subscribe_event_flag(
    FuriEventLoop* instance,
    FuriEventFlag* event_flag,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context) {
    extern const FuriEventLoopContract furi_event_flag_event_loop_contract;
    furi_event_loop_object_subscribe(
        instance, event_flag, &furi_event_flag_event_loop_contract, event, callback, context);
}

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

bool furi_event_loop_is_subscribed(FuriEventLoop* instance, FuriEventLoopObject* object) {
    furi_check(instance);
    furi_check(instance->thread_id == furi_thread_get_current_id());
    FURI_CRITICAL_ENTER();

    FuriEventLoopItem* const* item = FuriEventLoopTree_cget(instance->tree, object);
    bool result = !!item;

    FURI_CRITICAL_EXIT();
    return result;
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

    furi_event_loop_notify(owner, FuriEventLoopFlagEvent);
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
 *
 * @warning Only ONE instance of FuriEventLoop per thread is possible. ALL FuriEventLoop
 * funcitons MUST be called from the same thread that the instance was created in.
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
 */
typedef void (*FuriEventLoopEventCallback)(FuriEventLoopObject* object, void* context);

/** Opaque event flag type */
typedef struct FuriEventFlag FuriEventFlag;

/** Subscribe to event flag events
 *
 * @warning you can only have one subscription for one event type.
 *
 * @param      instance       The Event Loop instance
 * @param      event_flag     The event flag to add
 * @param[in]  event          The Event Loop event to trigger on
 * @param[in]  callback       The callback to call on event
 * @param      context        The context for callback
 */

void furi_event_loop_subscribe_event_flag(
    FuriEventLoop* instance,
    FuriEventFlag* event_flag,
    FuriEventLoopEvent event,
    FuriEventLoopEventCallback callback,
    void* context);

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

/**
 * @brief Checks if the loop is subscribed to an object of any kind
 * 
 * @param      instance       Event Loop instance
 * @param      object         Object to check
 */
bool furi_event_loop_is_subscribed(FuriEventLoop* instance, FuriEventLoopObject* object);

/**
 * @brief Convenience function for `if(is_subscribed()) unsubscribe()`
 */
static inline void
    furi_event_loop_maybe_unsubscribe(FuriEventLoop* instance, FuriEventLoopObject* object) {
    if(furi_event_loop_is_subscribed(instance, object))
        furi_event_loop_unsubscribe(instance, object);
}

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

typedef bool (
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
 *
 * @warning ALL FuriEventLoopTimer functions MUST be called from the
 * same thread that the owner FuriEventLoop instance was created in.
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
 *
 * @warning ALL FuriEventLoopTimer functions MUST be called from the
 * same thread that the owner FuriEventLoop instance was created in.
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

static void furi_timer_callback(TimerHandle_t hTimer) {
    FuriTimer* instance = pvTimerGetTimerID(hTimer);
    furi_check(instance);
    instance->cb_func(instance->cb_context);
}

static void furi_timer_flush_epilogue(void* context, uint32_t arg) {
    furi_assert(context);
    UNUSED(arg);

    EventGroupHandle_t hEvent = context;

    // See https://github.com/FreeRTOS/FreeRTOS-Kernel/issues/1142
    vTaskSuspendAll();
    xEventGroupSetBits(hEvent, TIMER_DELETED_EVENT);
    (void)xTaskResumeAll();
}

FuriTimer* furi_timer_alloc(FuriTimerCallback func, FuriTimerType type, void* context) {
    furi_check((furi_kernel_is_irq_or_masked() == 0U) && (func != NULL));

    FuriTimer* instance = malloc(sizeof(FuriTimer));

    instance->cb_func = func;
    instance->cb_context = context;

    const UBaseType_t reload = (type == FuriTimerTypeOnce ? pdFALSE : pdTRUE);
    const TimerHandle_t hTimer = xTimerCreateStatic(
        NULL, portMAX_DELAY, reload, instance, furi_timer_callback, &instance->container);

    furi_check(hTimer == (TimerHandle_t)instance);

    return instance;
}

void furi_timer_free(FuriTimer* instance) {
    furi_check(!furi_kernel_is_irq_or_masked());
    furi_check(instance);

    TimerHandle_t hTimer = (TimerHandle_t)instance;
    furi_check(xTimerDelete(hTimer, portMAX_DELAY) == pdPASS);

    furi_timer_flush();

    free(instance);
}

void furi_timer_flush(void) {
    StaticEventGroup_t event_container = {};
    EventGroupHandle_t hEvent = xEventGroupCreateStatic(&event_container);
    furi_check(
        xTimerPendFunctionCall(furi_timer_flush_epilogue, hEvent, 0, portMAX_DELAY) == pdPASS);

    furi_check(
        xEventGroupWaitBits(hEvent, TIMER_DELETED_EVENT, pdFALSE, pdTRUE, portMAX_DELAY) ==
        TIMER_DELETED_EVENT);
    vEventGroupDelete(hEvent);
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

/** Flush timer task control message queue
 *
 * Ensures that all commands before this point was processed.
 */
void furi_timer_flush(void);

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
 * @warning    This is synchronous call that will be blocked till timer queue processed.
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


=== BEGIN: applications/services/expansion/application.fam ===
App(
    appid="expansion_start",
    apptype=FlipperAppType.STARTUP,
    entry_point="expansion_on_system_start",
    cdefines=["SRV_EXPANSION"],
    sdk_headers=[
        "expansion.h",
    ],
    requires=["rpc_start"],
    provides=["expansion_settings"],
    order=150,
)

=== END: applications/services/expansion/application.fam ===

=== BEGIN: applications/services/expansion/expansion.c ===
#include "expansion.h"

#include <furi_hal_serial_control.h>

#include <furi.h>
#include <storage/storage.h>
#include <toolbox/api_lock.h>

#include "expansion_worker.h"
#include "expansion_settings.h"

#define TAG "ExpansionSrv"

#define EXPANSION_CONTROL_QUEUE_SIZE (8UL)
#define EXPANSION_CONTROL_STACK_SIZE (768UL)

typedef enum {
    ExpansionStateDisabled,
    ExpansionStateEnabled,
    ExpansionStateRunning,
} ExpansionState;

typedef enum {
    ExpansionMessageTypeEnable,
    ExpansionMessageTypeDisable,
    ExpansionMessageTypeSetListenSerial,
    ExpansionMessageTypeReloadSettings,
    ExpansionMessageTypeModuleConnected,
    ExpansionMessageTypeModuleDisconnected,
} ExpansionMessageType;

typedef union {
    FuriHalSerialId serial_id;
} ExpansionMessageData;

typedef struct {
    ExpansionMessageType type;
    ExpansionMessageData data;
    FuriApiLock api_lock;
} ExpansionMessage;

struct Expansion {
    FuriThread* thread;
    FuriMessageQueue* queue;
    FuriHalSerialId serial_id;
    ExpansionWorker* worker;
    ExpansionState state;
};

static const char* const expansion_uart_names[] = {
    "USART",
    "LPUART",
};

// Called from the serial control thread
static void expansion_detect_callback(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    ExpansionMessage message = {
        .type = ExpansionMessageTypeModuleConnected,
        .api_lock = NULL, // Not locking the API here to avoid a deadlock
    };

    // Not waiting for available queue space, discarding message if there is none
    const FuriStatus status = furi_message_queue_put(instance->queue, &message, 0);
    UNUSED(status);
}

static void expansion_worker_callback(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    ExpansionMessage message = {
        .type = ExpansionMessageTypeModuleDisconnected,
        .api_lock = NULL, // Not locking the API here to avoid a deadlock
    };

    const FuriStatus status = furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    furi_check(status == FuriStatusOk);
}

static void
    expansion_control_handler_enable(Expansion* instance, const ExpansionMessageData* data) {
    UNUSED(data);

    if(instance->state != ExpansionStateDisabled) {
        return;
    }

    ExpansionSettings settings;
    expansion_settings_load(&settings);

    if(settings.uart_index < FuriHalSerialIdMax) {
        instance->state = ExpansionStateEnabled;
        instance->serial_id = settings.uart_index;
        furi_hal_serial_control_set_expansion_callback(
            instance->serial_id, expansion_detect_callback, instance);

        FURI_LOG_D(TAG, "Detection enabled on %s", expansion_uart_names[instance->serial_id]);
    }
}

static void
    expansion_control_handler_disable(Expansion* instance, const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state == ExpansionStateDisabled) {
        return;
    } else if(instance->state == ExpansionStateRunning) {
        expansion_worker_stop(instance->worker);
        expansion_worker_free(instance->worker);
    } else {
        furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);
    }

    instance->state = ExpansionStateDisabled;

    FURI_LOG_D(TAG, "Detection disabled");
}

static void expansion_control_handler_set_listen_serial(
    Expansion* instance,
    const ExpansionMessageData* data) {
    if(instance->state != ExpansionStateDisabled && instance->serial_id == data->serial_id) {
        return;

    } else if(instance->state == ExpansionStateRunning) {
        expansion_worker_stop(instance->worker);
        expansion_worker_free(instance->worker);

    } else if(instance->state == ExpansionStateEnabled) {
        furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);
    }

    instance->state = ExpansionStateEnabled;
    instance->serial_id = data->serial_id;

    furi_hal_serial_control_set_expansion_callback(
        instance->serial_id, expansion_detect_callback, instance);

    FURI_LOG_D(TAG, "Listen serial changed to %s", expansion_uart_names[instance->serial_id]);
}

static void expansion_control_handler_reload_settings(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);

    ExpansionSettings settings;
    expansion_settings_load(&settings);

    if(settings.uart_index < FuriHalSerialIdMax) {
        const ExpansionMessageData data = {
            .serial_id = settings.uart_index,
        };

        expansion_control_handler_set_listen_serial(instance, &data);

    } else {
        expansion_control_handler_disable(instance, NULL);
    }
}

static void expansion_control_handler_module_connected(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state != ExpansionStateEnabled) {
        return;
    }

    furi_hal_serial_control_set_expansion_callback(instance->serial_id, NULL, NULL);

    instance->state = ExpansionStateRunning;
    instance->worker = expansion_worker_alloc(instance->serial_id);

    expansion_worker_set_callback(instance->worker, expansion_worker_callback, instance);
    expansion_worker_start(instance->worker);
}

static void expansion_control_handler_module_disconnected(
    Expansion* instance,
    const ExpansionMessageData* data) {
    UNUSED(data);
    if(instance->state != ExpansionStateRunning) {
        return;
    }

    instance->state = ExpansionStateEnabled;
    expansion_worker_free(instance->worker);
    furi_hal_serial_control_set_expansion_callback(
        instance->serial_id, expansion_detect_callback, instance);
}

typedef void (*ExpansionControlHandler)(Expansion*, const ExpansionMessageData*);

static const ExpansionControlHandler expansion_control_handlers[] = {
    [ExpansionMessageTypeEnable] = expansion_control_handler_enable,
    [ExpansionMessageTypeDisable] = expansion_control_handler_disable,
    [ExpansionMessageTypeSetListenSerial] = expansion_control_handler_set_listen_serial,
    [ExpansionMessageTypeReloadSettings] = expansion_control_handler_reload_settings,
    [ExpansionMessageTypeModuleConnected] = expansion_control_handler_module_connected,
    [ExpansionMessageTypeModuleDisconnected] = expansion_control_handler_module_disconnected,
};

static int32_t expansion_control(void* context) {
    furi_assert(context);
    Expansion* instance = context;

    for(;;) {
        ExpansionMessage message;

        FuriStatus status = furi_message_queue_get(instance->queue, &message, FuriWaitForever);
        furi_check(status == FuriStatusOk);

        furi_check(message.type < COUNT_OF(expansion_control_handlers));
        expansion_control_handlers[message.type](instance, &message.data);

        if(message.api_lock != NULL) {
            api_lock_unlock(message.api_lock);
        }
    }

    return 0;
}

static Expansion* expansion_alloc(void) {
    Expansion* instance = malloc(sizeof(Expansion));

    instance->queue =
        furi_message_queue_alloc(EXPANSION_CONTROL_QUEUE_SIZE, sizeof(ExpansionMessage));
    instance->thread =
        furi_thread_alloc_ex(TAG, EXPANSION_CONTROL_STACK_SIZE, expansion_control, instance);

    return instance;
}

static void expansion_storage_callback(const void* message, void* context) {
    furi_assert(context);

    const StorageEvent* event = message;
    Expansion* instance = context;

    if(event->type == StorageEventTypeCardMount) {
        ExpansionMessage em = {
            .type = ExpansionMessageTypeReloadSettings,
            .api_lock = NULL,
        };

        furi_check(furi_message_queue_put(instance->queue, &em, FuriWaitForever) == FuriStatusOk);
    }
}

void expansion_on_system_start(void* arg) {
    UNUSED(arg);

    Expansion* instance = expansion_alloc();
    furi_record_create(RECORD_EXPANSION, instance);
    furi_thread_start(instance->thread);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), expansion_storage_callback, instance);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping settings");
        return;
    }

    expansion_enable(instance);
}

// Public API functions

void expansion_enable(Expansion* instance) {
    furi_check(instance);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeEnable,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void expansion_disable(Expansion* instance) {
    furi_check(instance);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeDisable,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

void expansion_set_listen_serial(Expansion* instance, FuriHalSerialId serial_id) {
    furi_check(instance);
    furi_check(serial_id < FuriHalSerialIdMax);

    ExpansionMessage message = {
        .type = ExpansionMessageTypeSetListenSerial,
        .data.serial_id = serial_id,
        .api_lock = api_lock_alloc_locked(),
    };

    furi_message_queue_put(instance->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);
}

=== END: applications/services/expansion/expansion.c ===

=== BEGIN: applications/services/expansion/expansion.h ===
/**
 * @file expansion.h
 * @brief Expansion module support library.
 */
#pragma once

#include <furi_hal_serial_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FURI record key to access the expansion object.
 */
#define RECORD_EXPANSION "expansion"

/**
 * @brief Expansion opaque type declaration.
 */
typedef struct Expansion Expansion;

/**
 * @brief Enable support for expansion modules.
 *
 * Calling this function will load user settings and enable
 * expansion module support on the serial port specified in said settings.
 *
 * If expansion module support was disabled in settings, this function
 * does nothing.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 */
void expansion_enable(Expansion* instance);

/**
 * @brief Disable support for expansion modules.
 *
 * Calling this function will cease all communications with the
 * expansion module (if any), release the serial handle and
 * reset the respective pins to the default state.
 *
 * @note Applications requiring serial port access MUST call
 * this function BEFORE calling furi_hal_serial_control_acquire().
 * Similarly, an expansion_enable() call MUST be made right AFTER
 * a call to furi_hal_serial_control_release() to ensure that
 * the user settings are properly restored.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 */
void expansion_disable(Expansion* instance);

/**
 * @brief Enable support for expansion modules on designated serial port.
 *
 * Only one serial port can be used to communicate with an expansion
 * module at a time.
 *
 * Calling this function when expansion module support is already enabled
 * will first disable the previous setting, then enable the current one.
 *
 * @warning This function does not respect user settings for expansion modules,
 * so calling it might leave the system in inconsistent state. Avoid using it
 * unless absolutely necessary.
 *
 * @param[in,out] instance pointer to the Expansion instance.
 * @param[in] serial_id numerical identifier of the serial.
 */
void expansion_set_listen_serial(Expansion* instance, FuriHalSerialId serial_id);

#ifdef __cplusplus
}
#endif

=== END: applications/services/expansion/expansion.h ===

=== BEGIN: applications/services/expansion/expansion_protocol.h ===
/**
 * @file expansion_protocol.h
 * @brief Flipper Expansion Protocol parser reference implementation.
 *
 * This file is licensed separately under The Unlicense.
 * See https://unlicense.org/ for more details.
 *
 * This parser is written with low-spec hardware in mind. It does not use
 * dynamic memory allocation or Flipper-specific libraries and can be
 * included directly into any module's firmware's sources.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default baud rate to start all communications at.
 */
#define EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE (9600UL)

/**
 * @brief Maximum data size per frame, in bytes.
 */
#define EXPANSION_PROTOCOL_MAX_DATA_SIZE (64U)

/**
 * @brief Maximum allowed inactivity period, in milliseconds.
 */
#define EXPANSION_PROTOCOL_TIMEOUT_MS (250U)

/**
 * @brief Dead time after changing connection baud rate.
 */
#define EXPANSION_PROTOCOL_BAUD_CHANGE_DT_MS (25U)

/**
 * @brief Enumeration of supported frame types.
 */
typedef enum {
    ExpansionFrameTypeHeartbeat = 1, /**< Heartbeat frame. */
    ExpansionFrameTypeStatus = 2, /**< Status report frame. */
    ExpansionFrameTypeBaudRate = 3, /**< Baud rate negotiation frame. */
    ExpansionFrameTypeControl = 4, /**< Control frame. */
    ExpansionFrameTypeData = 5, /**< Data frame. */
    ExpansionFrameTypeReserved, /**< Special value. */
} ExpansionFrameType;

/**
 * @brief Enumeration of possible error types.
 */
typedef enum {
    ExpansionFrameErrorNone = 0x00, /**< No error occurred. */
    ExpansionFrameErrorUnknown = 0x01, /**< An unknown error has occurred (generic response). */
    ExpansionFrameErrorBaudRate = 0x02, /**< Requested baud rate is not supported. */
} ExpansionFrameError;

/**
 * @brief Enumeration of suported control commands.
 */
typedef enum {
    /** @brief Start an RPC session.
     *
     * Must only be used while the RPC session is NOT active.
     */
    ExpansionFrameControlCommandStartRpc = 0x00,
    /** @brief Stop an open RPC session.
      *
      * Must only be used while the RPC session IS active.
      */
    ExpansionFrameControlCommandStopRpc = 0x01,
    /** @brief Enable OTG (5V) on external GPIO.
      *
      * Must only be used while the RPC session is NOT active,
      * otherwise OTG is to be controlled via RPC messages.
      */
    ExpansionFrameControlCommandEnableOtg = 0x02,
    /** @brief Disable OTG (5V) on external GPIO.
      *
      * Must only be used while the RPC session is NOT active,
      * otherwise OTG is to be controlled via RPC messages.
      */
    ExpansionFrameControlCommandDisableOtg = 0x03,
} ExpansionFrameControlCommand;

#pragma pack(push, 1)

/**
 * @brief Frame header structure.
 */
typedef struct {
    uint8_t type; /**< Type of the frame. @see ExpansionFrameType. */
} ExpansionFrameHeader;

/**
 * @brief Heartbeat frame contents.
 */
typedef struct {
    /** Empty. */
} ExpansionFrameHeartbeat;

/**
 * @brief Status frame contents.
 */
typedef struct {
    uint8_t error; /**< Reported error code. @see ExpansionFrameError. */
} ExpansionFrameStatus;

/**
 * @brief Baud rate frame contents.
 */
typedef struct {
    uint32_t baud; /**< Requested baud rate. */
} ExpansionFrameBaudRate;

/**
 * @brief Control frame contents.
 */
typedef struct {
    uint8_t command; /**< Control command number. @see ExpansionFrameControlCommand. */
} ExpansionFrameControl;

/**
 * @brief Data frame contents.
 */
typedef struct {
    /** Size of the data. Must be less than EXPANSION_PROTOCOL_MAX_DATA_SIZE. */
    uint8_t size;
    /** Data bytes. Valid only up to ExpansionFrameData::size bytes. */
    uint8_t bytes[EXPANSION_PROTOCOL_MAX_DATA_SIZE];
} ExpansionFrameData;

/**
 * @brief Expansion protocol frame structure.
 */
typedef struct {
    ExpansionFrameHeader header; /**< Header of the frame. Required. */
    union {
        ExpansionFrameHeartbeat heartbeat; /**< Heartbeat frame contents. */
        ExpansionFrameStatus status; /**< Status frame contents. */
        ExpansionFrameBaudRate baud_rate; /**< Baud rate frame contents. */
        ExpansionFrameControl control; /**< Control frame contents. */
        ExpansionFrameData data; /**< Data frame contents. */
    } content; /**< Contents of the frame. */
} ExpansionFrame;

#pragma pack(pop)

/**
 * @brief Expansion checksum type.
 */
typedef uint8_t ExpansionFrameChecksum;

/**
 * @brief Receive function type declaration.
 *
 * @see expansion_frame_decode().
 *
 * @param[out] data pointer to the buffer to reveive the data into.
 * @param[in] data_size maximum output buffer capacity, in bytes.
 * @param[in,out] context pointer to a user-defined context object.
 * @returns number of bytes written into the output buffer.
 */
typedef size_t (*ExpansionFrameReceiveCallback)(uint8_t* data, size_t data_size, void* context);

/**
 * @brief Send function type declaration.
 *
 * @see expansion_frame_encode().
 *
 * @param[in] data pointer to the buffer containing the data to be sent.
 * @param[in] data_size size of the data to send, in bytes.
 * @param[in,out] context pointer to a user-defined context object.
 * @returns number of bytes actually sent.
 */
typedef size_t (*ExpansionFrameSendCallback)(const uint8_t* data, size_t data_size, void* context);

/**
 * @brief Get encoded frame size.
 *
 * The frame MUST be complete and properly formed.
 *
 * @param[in] frame pointer to the frame to be evaluated.
 * @returns encoded frame size, in bytes.
 */
static inline size_t expansion_frame_get_encoded_size(const ExpansionFrame* frame) {
    switch(frame->header.type) {
    case ExpansionFrameTypeHeartbeat:
        return sizeof(frame->header);
    case ExpansionFrameTypeStatus:
        return sizeof(frame->header) + sizeof(frame->content.status);
    case ExpansionFrameTypeBaudRate:
        return sizeof(frame->header) + sizeof(frame->content.baud_rate);
    case ExpansionFrameTypeControl:
        return sizeof(frame->header) + sizeof(frame->content.control);
    case ExpansionFrameTypeData:
        return sizeof(frame->header) + sizeof(frame->content.data.size) + frame->content.data.size;
    default:
        return 0;
    }
}

/**
 * @brief Get remaining number of bytes needed to properly decode a frame.
 *
 * The return value will vary depending on the received_size parameter value.
 * The frame is considered complete when the function returns 0.
 *
 * @param[in] frame pointer to the frame to be evaluated.
 * @param[in] received_size number of bytes currently availabe for evaluation.
 * @param[out] remaining_size pointer to the variable to contain the number of bytes needed for a complete frame.
 * @returns true if the remaining size could be calculated, false on error.
 */
static inline bool expansion_frame_get_remaining_size(
    const ExpansionFrame* frame,
    size_t received_size,
    size_t* remaining_size) {
    if(received_size < sizeof(ExpansionFrameHeader)) {
        // Frame type is unknown as of now
        *remaining_size = sizeof(ExpansionFrameHeader);
        return true;
    }

    const size_t received_content_size = received_size - sizeof(ExpansionFrameHeader);
    size_t content_size;

    switch(frame->header.type) {
    case ExpansionFrameTypeHeartbeat:
        content_size = 0;
        break;
    case ExpansionFrameTypeStatus:
        content_size = sizeof(frame->content.status);
        break;
    case ExpansionFrameTypeBaudRate:
        content_size = sizeof(frame->content.baud_rate);
        break;
    case ExpansionFrameTypeControl:
        content_size = sizeof(frame->content.control);
        break;
    case ExpansionFrameTypeData:
        if(received_content_size < sizeof(frame->content.data.size)) {
            // Data size is unknown as of now
            content_size = sizeof(frame->content.data.size);
        } else if(frame->content.data.size > sizeof(frame->content.data.bytes)) {
            // Malformed frame or garbage input
            return false;
        } else {
            content_size = sizeof(frame->content.data.size) + frame->content.data.size;
        }
        break;
    default:
        return false;
    }

    if(content_size > received_content_size) {
        *remaining_size = content_size - received_content_size;
    } else {
        *remaining_size = 0;
    }

    return true;
}

/**
 * @brief Enumeration of protocol parser statuses.
 */
typedef enum {
    ExpansionProtocolStatusOk, /**< No error has occurred. */
    ExpansionProtocolStatusErrorFormat, /**< Invalid frame type. */
    ExpansionProtocolStatusErrorChecksum, /**< Checksum mismatch. */
    ExpansionProtocolStatusErrorCommunication, /**< Input/output error. */
} ExpansionProtocolStatus;

/**
 * @brief Get the checksum byte corresponding to the frame
 *
 * Lightweight XOR checksum algorithm for basic error detection.
 *
 * @param[in] data pointer to a byte buffer containing the data.
 * @param[in] data_size size of the data buffer.
 * @returns checksum byte of the frame.
 */
static inline ExpansionFrameChecksum
    expansion_protocol_get_checksum(const uint8_t* data, size_t data_size) {
    ExpansionFrameChecksum checksum = 0;
    for(size_t i = 0; i < data_size; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
 * @brief Receive and decode a frame.
 *
 * Will repeatedly call the receive callback function until enough data is received.
 *
 * @param[out] frame pointer to the frame to contain decoded data.
 * @param[in] receive pointer to the function used to receive data.
 * @param[in,out] context pointer to a user-defined context object. Will be passed to the receive callback function.
 * @returns ExpansionProtocolStatusOk on success, any other error code on failure.
 */
static inline ExpansionProtocolStatus expansion_protocol_decode(
    ExpansionFrame* frame,
    ExpansionFrameReceiveCallback receive,
    void* context) {
    size_t total_size = 0;
    size_t remaining_size;

    while(true) {
        if(!expansion_frame_get_remaining_size(frame, total_size, &remaining_size)) {
            return ExpansionProtocolStatusErrorFormat;
        } else if(remaining_size == 0) {
            break;
        }

        const size_t received_size =
            receive((uint8_t*)frame + total_size, remaining_size, context);

        if(received_size == 0) {
            return ExpansionProtocolStatusErrorCommunication;
        }

        total_size += received_size;
    }

    ExpansionFrameChecksum checksum;
    const size_t received_size = receive(&checksum, sizeof(checksum), context);

    if(received_size != sizeof(checksum)) {
        return ExpansionProtocolStatusErrorCommunication;
    } else if(checksum != expansion_protocol_get_checksum((const uint8_t*)frame, total_size)) {
        return ExpansionProtocolStatusErrorChecksum;
    } else {
        return ExpansionProtocolStatusOk;
    }
}

/**
 * @brief Encode and send a frame.
 *
 * @param[in] frame pointer to the frame to be encoded and sent.
 * @param[in] send pointer to the function used to send data.
 * @param[in,out] context pointer to a user-defined context object. Will be passed to the send callback function.
 * @returns ExpansionProtocolStatusOk on success, any other error code on failure.
 */
static inline ExpansionProtocolStatus expansion_protocol_encode(
    const ExpansionFrame* frame,
    ExpansionFrameSendCallback send,
    void* context) {
    const size_t encoded_size = expansion_frame_get_encoded_size(frame);
    if(encoded_size == 0) {
        return ExpansionProtocolStatusErrorFormat;
    }

    const ExpansionFrameChecksum checksum =
        expansion_protocol_get_checksum((const uint8_t*)frame, encoded_size);

    if((send((const uint8_t*)frame, encoded_size, context) != encoded_size) ||
       (send(&checksum, sizeof(checksum), context) != sizeof(checksum))) {
        return ExpansionProtocolStatusErrorCommunication;
    } else {
        return ExpansionProtocolStatusOk;
    }
}

#ifdef __cplusplus
}
#endif

=== END: applications/services/expansion/expansion_protocol.h ===

=== BEGIN: applications/services/expansion/expansion_settings.c ===
#include "expansion_settings.h"

#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#include "expansion_settings_filename.h"

#define TAG "ExpansionSettings"

#define EXPANSION_SETTINGS_PATH    INT_PATH(EXPANSION_SETTINGS_FILE_NAME)
#define EXPANSION_SETTINGS_VERSION (0)
#define EXPANSION_SETTINGS_MAGIC   (0xEA)

void expansion_settings_load(ExpansionSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_load(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);

    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        memset(settings, 0, sizeof(ExpansionSettings));
        expansion_settings_save(settings);
    }
}

void expansion_settings_save(const ExpansionSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_save(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}

=== END: applications/services/expansion/expansion_settings.c ===

=== BEGIN: applications/services/expansion/expansion_settings.h ===
/**
 * @file expansion_settings.h
 * @brief Expansion module support settings.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Expansion module support settings storage type.
 */
typedef struct {
    /**
     * Numerical index of serial port used to communicate
     * with expansion modules.
     */
    uint8_t uart_index;
} ExpansionSettings;

/**
 * @brief Load expansion module support settings from file.
 *
 * @param[in,out] settings pointer to an ExpansionSettings instance to load settings into.
 */
void expansion_settings_load(ExpansionSettings* settings);

/**
 * @brief Save expansion module support settings to file.
 *
 * @param[in] settings pointer to an ExpansionSettings instance to save settings from.
 */
void expansion_settings_save(const ExpansionSettings* settings);

#ifdef __cplusplus
}
#endif

=== END: applications/services/expansion/expansion_settings.h ===

=== BEGIN: applications/services/expansion/expansion_settings_filename.h ===
/**
 * @file expansion_settings_filename.h
 */
#pragma once

/**
 * @brief File name used for expansion settings.
 */
#define EXPANSION_SETTINGS_FILE_NAME ".expansion.settings"

=== END: applications/services/expansion/expansion_settings_filename.h ===

=== BEGIN: applications/services/expansion/expansion_worker.c ===
#include "expansion_worker.h"

#include <furi_hal_power.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#include <furi.h>
#include <rpc/rpc.h>

#include "expansion_protocol.h"

#define TAG "ExpansionSrv"

#define EXPANSION_WORKER_STACK_SZIE  (768UL)
#define EXPANSION_WORKER_BUFFER_SIZE (sizeof(ExpansionFrame) + sizeof(ExpansionFrameChecksum))

typedef enum {
    ExpansionWorkerStateHandShake,
    ExpansionWorkerStateConnected,
    ExpansionWorkerStateRpcActive,
} ExpansionWorkerState;

typedef enum {
    ExpansionWorkerExitReasonUnknown,
    ExpansionWorkerExitReasonUser,
    ExpansionWorkerExitReasonError,
    ExpansionWorkerExitReasonTimeout,
} ExpansionWorkerExitReason;

typedef enum {
    ExpansionWorkerFlagStop = 1 << 0,
    ExpansionWorkerFlagData = 1 << 1,
    ExpansionWorkerFlagError = 1 << 2,
} ExpansionWorkerFlag;

#define EXPANSION_ALL_FLAGS (ExpansionWorkerFlagData | ExpansionWorkerFlagStop)

struct ExpansionWorker {
    FuriThread* thread;
    FuriStreamBuffer* rx_buf;
    FuriSemaphore* tx_semaphore;

    FuriHalSerialId serial_id;
    FuriHalSerialHandle* serial_handle;

    RpcSession* rpc_session;

    ExpansionWorkerState state;
    ExpansionWorkerExitReason exit_reason;
    ExpansionWorkerCallback callback;
    void* cb_context;
};

// Called in UART IRQ context
static void expansion_worker_serial_rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    furi_assert(handle);
    furi_assert(context);

    ExpansionWorker* instance = context;

    if(event & (FuriHalSerialRxEventNoiseError | FuriHalSerialRxEventFrameError |
                FuriHalSerialRxEventOverrunError)) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagError);
    } else if(event & FuriHalSerialRxEventData) {
        while(furi_hal_serial_async_rx_available(handle)) {
            const uint8_t data = furi_hal_serial_async_rx(handle);
            furi_stream_buffer_send(instance->rx_buf, &data, sizeof(data), 0);
        }
        furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagData);
    }
}

static size_t expansion_worker_receive_callback(uint8_t* data, size_t data_size, void* context) {
    ExpansionWorker* instance = context;

    size_t received_size = 0;

    while(true) {
        received_size += furi_stream_buffer_receive(
            instance->rx_buf, data + received_size, data_size - received_size, 0);

        if(received_size == data_size) break;

        const uint32_t flags = furi_thread_flags_wait(
            EXPANSION_ALL_FLAGS, FuriFlagWaitAny, furi_ms_to_ticks(EXPANSION_PROTOCOL_TIMEOUT_MS));

        if(flags & FuriFlagError) {
            if(flags == (unsigned)FuriFlagErrorTimeout) {
                // Exiting due to timeout
                instance->exit_reason = ExpansionWorkerExitReasonTimeout;
            } else {
                // Exiting due to an unspecified error
                instance->exit_reason = ExpansionWorkerExitReasonError;
            }
            break;
        } else if(flags & ExpansionWorkerFlagStop) {
            // Exiting due to explicit request
            instance->exit_reason = ExpansionWorkerExitReasonUser;
            break;
        } else if(flags & ExpansionWorkerFlagError) {
            // Exiting due to RPC error
            instance->exit_reason = ExpansionWorkerExitReasonError;
            break;
        } else if(flags & ExpansionWorkerFlagData) {
            // Go to buffer reading
            continue;
        }
    }

    return received_size;
}

static inline bool
    expansion_worker_receive_frame(ExpansionWorker* instance, ExpansionFrame* frame) {
    return expansion_protocol_decode(frame, expansion_worker_receive_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static size_t
    expansion_worker_send_callback(const uint8_t* data, size_t data_size, void* context) {
    ExpansionWorker* instance = context;
    furi_hal_serial_tx(instance->serial_handle, data, data_size);
    furi_hal_serial_tx_wait_complete(instance->serial_handle);
    return data_size;
}

static inline bool
    expansion_worker_send_frame(ExpansionWorker* instance, const ExpansionFrame* frame) {
    return expansion_protocol_encode(frame, expansion_worker_send_callback, instance) ==
           ExpansionProtocolStatusOk;
}

static bool expansion_worker_send_heartbeat(ExpansionWorker* instance) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeHeartbeat,
        .content.heartbeat = {},
    };

    return expansion_worker_send_frame(instance, &frame);
}

static bool
    expansion_worker_send_status_response(ExpansionWorker* instance, ExpansionFrameError error) {
    const ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeStatus,
        .content.status.error = error,
    };

    return expansion_worker_send_frame(instance, &frame);
}

static bool expansion_worker_send_data_response(
    ExpansionWorker* instance,
    const uint8_t* data,
    size_t data_size) {
    furi_assert(data_size <= EXPANSION_PROTOCOL_MAX_DATA_SIZE);

    ExpansionFrame frame = {
        .header.type = ExpansionFrameTypeData,
        .content.data.size = data_size,
    };

    memcpy(frame.content.data.bytes, data, data_size);
    return expansion_worker_send_frame(instance, &frame);
}

// Called in Rpc session thread context
static void expansion_worker_rpc_send_callback(void* context, uint8_t* data, size_t data_size) {
    ExpansionWorker* instance = context;

    for(size_t sent_data_size = 0; sent_data_size < data_size;) {
        if(furi_semaphore_acquire(
               instance->tx_semaphore, furi_ms_to_ticks(EXPANSION_PROTOCOL_TIMEOUT_MS)) !=
           FuriStatusOk) {
            furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagError);
            break;
        }

        const size_t current_data_size =
            MIN(data_size - sent_data_size, EXPANSION_PROTOCOL_MAX_DATA_SIZE);
        if(!expansion_worker_send_data_response(instance, data + sent_data_size, current_data_size))
            break;
        sent_data_size += current_data_size;
    }
}

static bool expansion_worker_rpc_session_open(ExpansionWorker* instance) {
    Rpc* rpc = furi_record_open(RECORD_RPC);
    instance->rpc_session = rpc_session_open(rpc, RpcOwnerUart);

    if(instance->rpc_session) {
        instance->tx_semaphore = furi_semaphore_alloc(1, 1);
        rpc_session_set_context(instance->rpc_session, instance);
        rpc_session_set_send_bytes_callback(
            instance->rpc_session, expansion_worker_rpc_send_callback);
    }

    return instance->rpc_session != NULL;
}

static void expansion_worker_rpc_session_close(ExpansionWorker* instance) {
    if(instance->rpc_session) {
        rpc_session_close(instance->rpc_session);
        furi_semaphore_free(instance->tx_semaphore);
    }

    furi_record_close(RECORD_RPC);
}

static bool expansion_worker_handle_state_handshake(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type != ExpansionFrameTypeBaudRate) break;
        const uint32_t baud_rate = rx_frame->content.baud_rate.baud;

        FURI_LOG_D(TAG, "Proposed baud rate: %lu", baud_rate);

        if(furi_hal_serial_is_baud_rate_supported(instance->serial_handle, baud_rate)) {
            instance->state = ExpansionWorkerStateConnected;
            // Send response at previous baud rate
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;
            furi_hal_serial_set_br(instance->serial_handle, baud_rate);

        } else {
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorBaudRate))
                break;
            FURI_LOG_E(TAG, "Bad baud rate");
        }
        success = true;
    } while(false);

    return success;
}

static bool expansion_worker_handle_state_connected(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeControl) {
            const uint8_t command = rx_frame->content.control.command;
            if(command == ExpansionFrameControlCommandStartRpc) {
                if(!expansion_worker_rpc_session_open(instance)) break;
                instance->state = ExpansionWorkerStateRpcActive;
            } else if(command == ExpansionFrameControlCommandEnableOtg) {
                furi_hal_power_enable_otg();
            } else if(command == ExpansionFrameControlCommandDisableOtg) {
                furi_hal_power_disable_otg();
            } else {
                break;
            }

            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_worker_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

static bool expansion_worker_handle_state_rpc_active(
    ExpansionWorker* instance,
    const ExpansionFrame* rx_frame) {
    bool success = false;

    do {
        if(rx_frame->header.type == ExpansionFrameTypeData) {
            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

            const size_t size_consumed = rpc_session_feed(
                instance->rpc_session,
                rx_frame->content.data.bytes,
                rx_frame->content.data.size,
                EXPANSION_PROTOCOL_TIMEOUT_MS);
            if(size_consumed != rx_frame->content.data.size) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeControl) {
            const uint8_t command = rx_frame->content.control.command;
            if(command == ExpansionFrameControlCommandStopRpc) {
                instance->state = ExpansionWorkerStateConnected;
                expansion_worker_rpc_session_close(instance);
            } else {
                break;
            }

            if(!expansion_worker_send_status_response(instance, ExpansionFrameErrorNone)) break;

        } else if(rx_frame->header.type == ExpansionFrameTypeStatus) {
            if(rx_frame->content.status.error != ExpansionFrameErrorNone) break;
            furi_semaphore_release(instance->tx_semaphore);

        } else if(rx_frame->header.type == ExpansionFrameTypeHeartbeat) {
            if(!expansion_worker_send_heartbeat(instance)) break;

        } else {
            break;
        }
        success = true;
    } while(false);

    return success;
}

typedef bool (*ExpansionWorkerStateHandler)(ExpansionWorker*, const ExpansionFrame*);

static const ExpansionWorkerStateHandler expansion_handlers[] = {
    [ExpansionWorkerStateHandShake] = expansion_worker_handle_state_handshake,
    [ExpansionWorkerStateConnected] = expansion_worker_handle_state_connected,
    [ExpansionWorkerStateRpcActive] = expansion_worker_handle_state_rpc_active,
};

static inline void expansion_worker_state_machine(ExpansionWorker* instance) {
    ExpansionFrame rx_frame;

    while(true) {
        if(!expansion_worker_receive_frame(instance, &rx_frame)) break;
        if(!expansion_handlers[instance->state](instance, &rx_frame)) break;
    }
}

static int32_t expansion_worker(void* context) {
    furi_assert(context);
    ExpansionWorker* instance = context;

    furi_hal_power_insomnia_enter();

    instance->serial_handle = furi_hal_serial_control_acquire(instance->serial_id);
    furi_check(instance->serial_handle);

    FURI_LOG_D(TAG, "Worker started");

    instance->state = ExpansionWorkerStateHandShake;
    instance->exit_reason = ExpansionWorkerExitReasonUnknown;

    furi_hal_serial_init(instance->serial_handle, EXPANSION_PROTOCOL_DEFAULT_BAUD_RATE);

    furi_hal_serial_async_rx_start(
        instance->serial_handle, expansion_worker_serial_rx_callback, instance, true);

    if(expansion_worker_send_heartbeat(instance)) {
        expansion_worker_state_machine(instance);
    }

    if(instance->state == ExpansionWorkerStateRpcActive) {
        expansion_worker_rpc_session_close(instance);
    }

    FURI_LOG_D(TAG, "Worker stopped");

    furi_hal_serial_control_release(instance->serial_handle);
    furi_hal_power_insomnia_exit();

    // Do not invoke worker callback on user-requested exit
    if((instance->exit_reason != ExpansionWorkerExitReasonUser) && (instance->callback != NULL)) {
        instance->callback(instance->cb_context);
    }

    return 0;
}

ExpansionWorker* expansion_worker_alloc(FuriHalSerialId serial_id) {
    ExpansionWorker* instance = malloc(sizeof(ExpansionWorker));

    instance->thread = furi_thread_alloc_ex(
        TAG "Worker", EXPANSION_WORKER_STACK_SZIE, expansion_worker, instance);
    instance->rx_buf = furi_stream_buffer_alloc(EXPANSION_WORKER_BUFFER_SIZE, 1);
    instance->serial_id = serial_id;

    // Improves responsiveness in heavy games at the expense of dropped frames
    furi_thread_set_priority(instance->thread, FuriThreadPriorityLow);

    return instance;
}

void expansion_worker_free(ExpansionWorker* instance) {
    furi_stream_buffer_free(instance->rx_buf);
    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);
    free(instance);
}

void expansion_worker_set_callback(
    ExpansionWorker* instance,
    ExpansionWorkerCallback callback,
    void* context) {
    instance->callback = callback;
    instance->cb_context = context;
}

void expansion_worker_start(ExpansionWorker* instance) {
    furi_thread_start(instance->thread);
}

void expansion_worker_stop(ExpansionWorker* instance) {
    furi_thread_flags_set(furi_thread_get_id(instance->thread), ExpansionWorkerFlagStop);
    furi_thread_join(instance->thread);
}

=== END: applications/services/expansion/expansion_worker.c ===

=== BEGIN: applications/services/expansion/expansion_worker.h ===
/**
 * @file expansion_worker.h
 * @brief Expansion module handling thread wrapper.
 *
 * The worker is started each time an expansion module is detected
 * and handles all of the communication protocols. Likewise, it is stopped
 * upon module disconnection or communication error.
 *
 * @warning This file is a private implementation detail. Please do not attempt to use it in applications.
 */
#pragma once

#include <furi_hal_serial_types.h>

/**
 * @brief Expansion worker opaque type declaration.
 */
typedef struct ExpansionWorker ExpansionWorker;

/**
 * @brief Worker callback type.
 *
 * @see expansion_worker_set_callback()
 *
 * @param[in,out] context pointer to a user-defined object.
 */
typedef void (*ExpansionWorkerCallback)(void* context);

/**
 * @brief Create an expansion worker instance.
 *
 * @param[in] serial_id numerical identifier of the serial to be used by the worker.
 * @returns pointer to the created instance.
 */
ExpansionWorker* expansion_worker_alloc(FuriHalSerialId serial_id);

/**
 * @brief Delete an expansion worker instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void expansion_worker_free(ExpansionWorker* instance);

/**
 * @brief Set the module disconnect callback.
 *
 * The callback will be triggered upon worker stop EXCEPT
 * when it was stopped via an expansion_worker_stop() call.
 *
 * In other words, the callback will ONLY be triggered if the worker was
 * stopped due to the user disconnecting/resetting/powering down the module,
 * or due to some communication error.
 *
 * @param[in,out] instance pointer to the worker instance to be modified.
 * @param[in] callback pointer to the callback function to be called under the above conditions.
 * @param[in] context pointer to a user-defined object, will be passed as a parameter to the callback.
 */
void expansion_worker_set_callback(
    ExpansionWorker* instance,
    ExpansionWorkerCallback callback,
    void* context);

/**
 * @brief Start the expansion module worker.
 *
 * @param[in,out] instance pointer to the worker instance to be started.
 */
void expansion_worker_start(ExpansionWorker* instance);

/**
 * @brief Stop the expansion module worker.
 *
 * If the worker was stopped via this call (and not because of module disconnect/
 * protocol error), the callback will not be triggered.
 *
 * @param[in,out] instance pointer to the worker instance to be stopped.
 */
void expansion_worker_stop(ExpansionWorker* instance);

=== END: applications/services/expansion/expansion_worker.h ===


