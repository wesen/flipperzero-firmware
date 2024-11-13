#include <stdlib.h>
#include <furi.h>

typedef struct {
    int x;
    int y;
} BoxMoverModel;

typedef struct {
    BoxMoverModel* model;
} BoxMover;

BoxMover* box_mover_alloc() {
    BoxMover* box_mover = malloc(sizeof(BoxMover));
    box_mover->model = malloc(sizeof(BoxMoverModel));
    box_mover->model->x = 10;
    box_mover->model->y = 10;
    return box_mover;
}

void box_mover_free(BoxMover* box_mover) {
    free(box_mover->model);
    free(box_mover);
}

int32_t box_mover_app(void*) {
    BoxMover* box_mover = box_mover_alloc();

    box_mover_free(box_mover);
    return 0;
}
