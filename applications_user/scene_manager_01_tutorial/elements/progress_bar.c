#include "progress_bar.h"

typedef struct ProgressBar {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t progress;
} ProgressBar;

static void progress_bar_draw(Canvas* canvas, WidgetElement* element) {
    furi_assert(canvas);
    furi_assert(element);

    ProgressBar* model = element->model;

    canvas_draw_frame(canvas, model->x, model->y, model->width, model->height);

    uint8_t fill_width = (model->width - 2) * model->progress / 100;
    canvas_draw_box(canvas, model->x + 1, model->y + 1, fill_width, model->height - 2);
}

static bool progress_bar_input(InputEvent* event, WidgetElement* element) {
    UNUSED(event);
    UNUSED(element);
    return false;
}

static void progress_bar_free(WidgetElement* element) {
    furi_assert(element);
    free(element->model);
    free(element);
}

WidgetElement* widget_element_progress_bar_create(
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t progress) {
    furi_assert(width > 2);
    furi_assert(height > 2);
    furi_assert(progress <= 100);

    WidgetElement* element = malloc(sizeof(WidgetElement));

    element->draw = progress_bar_draw;
    element->input = progress_bar_input;
    element->free = progress_bar_free;

    ProgressBar* model = malloc(sizeof(ProgressBar));
    model->x = x;
    model->y = y;
    model->width = width;
    model->height = height;
    model->progress = progress;
    element->model = model;

    return element;
}

void widget_eleemnt_progress_bar_set_progress(WidgetElement* element, uint8_t progress) {
    furi_assert(element);
    furi_assert(progress <= 100);

    ProgressBar* model = element->model;
    model->progress = progress;
}
