#pragma once

#include <gui/view.h>

typedef struct TimerView TimerView;

typedef void (*TimerViewCallback)(void* context);

TimerView* timer_view_alloc();
void timer_view_free(TimerView* timer_view);
View* timer_view_get_view(TimerView* timer_view);
void timer_view_set_left_callback(TimerView* timer_view, TimerViewCallback callback, void* context);
void timer_view_set_right_callback(TimerView* timer_view, TimerViewCallback callback, void* context);
void timer_view_set_ok_callback(TimerView* timer_view, TimerViewCallback callback, void* context);
void timer_view_set_back_callback(TimerView* timer_view, TimerViewCallback callback, void* context);
void timer_view_update_timer(TimerView* timer_view, uint32_t remaining_time);
void timer_view_set_running(TimerView* timer_view, bool running);
