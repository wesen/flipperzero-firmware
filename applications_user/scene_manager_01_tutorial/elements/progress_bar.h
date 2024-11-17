#pragma once

#include <gui/gui.h>
#include <gui/elements.h>
#include "gui/modules/widget_elements/widget_element_i.h"

typedef struct ProgressBar ProgressBar;

WidgetElement* widget_element_progress_bar_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t progress);
void widget_element_progress_bar_set_progress(WidgetElement* element, uint8_t progress);
