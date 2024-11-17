#include "scene_manager_01_tutorial.h"
#include "scenes/scene_manager_01_tutorial_scene.h"
#include "views/scene_manager_01_tutorial_views.h"

static bool scene_manager_01_tutorial_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SceneManager01Tutorial* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool scene_manager_01_tutorial_back_event_callback(void* context) {
    furi_assert(context);
    SceneManager01Tutorial* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

int32_t scene_manager_01_tutorial(void* p) {
    UNUSED(p);
    SceneManager01Tutorial* app = malloc(sizeof(SceneManager01Tutorial));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();

    // Allocate scene manager
    app->scene_manager = scene_manager_alloc(&scene_manager_01_tutorial_scene_handlers, app);

    // Allocate views
    app->main_view = scene_manager_01_tutorial_main_view_alloc(app);
    view_dispatcher_add_view(app->view_dispatcher, SceneManager01TutorialViewMain, app->main_view);

    app->circle_view = scene_manager_01_tutorial_circle_view_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, SceneManager01TutorialViewCircle, app->circle_view);

    // Configure view dispatcher
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, scene_manager_01_tutorial_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, scene_manager_01_tutorial_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Start first scene
    scene_manager_next_scene(app->scene_manager, SceneManager01TutorialSceneMain);

    view_dispatcher_run(app->view_dispatcher);

    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, SceneManager01TutorialViewMain);
    view_dispatcher_remove_view(app->view_dispatcher, SceneManager01TutorialViewCircle);

    scene_manager_01_tutorial_main_view_free(app->main_view);
    scene_manager_01_tutorial_circle_view_free(app->circle_view);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(app);
    return 0;
}
