#include <furi.h>

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    uint32_t count;
} TutorialEventLoopApp;

static void timer_callback(void *context) {
    TutorialEventLoopApp *app = context;
    app->event_loop = furi_event_loop_alloc();
}