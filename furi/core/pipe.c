#include "pipe.h"
#include "stream_buffer.h"
#include "semaphore.h"
#include "mutex.h"
#include "check.h"
#include "memmgr.h"

/**
 * There are two PipeSides, both of which point to the same primitives
 */
struct FuriPipeSide {
    FuriPipeRole role;
    FuriStreamBuffer* sending;
    FuriStreamBuffer* receiving;
    FuriSemaphore* instance_count; // <! 1 = both sides, 0 = only one side
    FuriMutex* state_transition;
};

FuriPipe furi_pipe_alloc(size_t capacity, size_t trigger_level) {
    FuriPipeSideReceiveSettings settings = {
        .capacity = capacity,
        .trigger_level = trigger_level,
    };
    return furi_pipe_alloc_ex(settings, settings);
}

FuriPipe furi_pipe_alloc_ex(FuriPipeSideReceiveSettings alice, FuriPipeSideReceiveSettings bob) {
    // the underlying primitives are shared
    FuriStreamBuffer* alice_to_bob = furi_stream_buffer_alloc(bob.capacity, bob.trigger_level);
    FuriStreamBuffer* bob_to_alice = furi_stream_buffer_alloc(alice.capacity, alice.trigger_level);
    FuriSemaphore* instance_count = furi_semaphore_alloc(1, 1);
    FuriMutex* state_transition = furi_mutex_alloc(FuriMutexTypeNormal);

    FuriPipeSide* alices_side = malloc(sizeof(FuriPipeSide));
    FuriPipeSide* bobs_side = malloc(sizeof(FuriPipeSide));

    *alices_side = (FuriPipeSide){
        .role = FuriPipeRoleAlice,
        .sending = alice_to_bob,
        .receiving = bob_to_alice,
        .instance_count = instance_count,
        .state_transition = state_transition,
    };
    *bobs_side = (FuriPipeSide){
        .role = FuriPipeRoleBob,
        .sending = bob_to_alice,
        .receiving = alice_to_bob,
        .instance_count = instance_count,
        .state_transition = state_transition,
    };

    return (FuriPipe){.alices_side = alices_side, .bobs_side = bobs_side};
}

FuriPipeRole furi_pipe_role(FuriPipeSide* pipe) {
    furi_check(pipe);
    return pipe->role;
}

FuriPipeState furi_pipe_state(FuriPipeSide* pipe) {
    furi_check(pipe);
    uint32_t count = furi_semaphore_get_count(pipe->instance_count);
    return (count == 1) ? FuriPipeStateOpen : FuriPipeStateBroken;
}

void furi_pipe_free(FuriPipeSide* pipe) {
    furi_check(pipe);

    furi_mutex_acquire(pipe->state_transition, FuriWaitForever);
    FuriStatus status = furi_semaphore_acquire(pipe->instance_count, 0);

    if(status == FuriStatusOk) {
        // the other side is still intact
        furi_mutex_release(pipe->state_transition);
        free(pipe);
    } else {
        // the other side is gone too
        furi_stream_buffer_free(pipe->sending);
        furi_stream_buffer_free(pipe->receiving);
        furi_semaphore_free(pipe->instance_count);
        furi_mutex_free(pipe->state_transition);
        free(pipe);
    }
}

static void _furi_pipe_stdout_cb(const char* data, size_t size, void* context) {
    furi_assert(context);
    FuriPipeSide* pipe = context;
    furi_check(furi_stream_buffer_send(pipe->sending, data, size, FuriWaitForever) == size);
}

static size_t _furi_pipe_stdin_cb(char* data, size_t size, FuriWait timeout, void* context) {
    furi_assert(context);
    FuriPipeSide* pipe = context;
    return furi_stream_buffer_receive(pipe->sending, data, size, timeout);
}

void furi_pipe_install_as_stdio(FuriPipeSide* pipe) {
    furi_check(pipe);
    furi_thread_set_stdout_callback(_furi_pipe_stdout_cb, pipe);
    furi_thread_set_stdin_callback(_furi_pipe_stdin_cb, pipe);
}

size_t furi_pipe_receive(FuriPipeSide* pipe, void* data, size_t length, FuriWait timeout) {
    furi_check(pipe);
    return furi_stream_buffer_receive(pipe->receiving, data, length, timeout);
}

size_t furi_pipe_send(FuriPipeSide* pipe, const void* data, size_t length, FuriWait timeout) {
    furi_check(pipe);
    return furi_stream_buffer_send(pipe->sending, data, length, timeout);
}

size_t furi_pipe_bytes_available(FuriPipeSide* pipe) {
    furi_check(pipe);
    return furi_stream_buffer_bytes_available(pipe->receiving);
}

size_t furi_pipe_spaces_available(FuriPipeSide* pipe) {
    furi_check(pipe);
    return furi_stream_buffer_spaces_available(pipe->sending);
}
