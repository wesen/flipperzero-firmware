#include "example_cpp_view.hpp"

ExampleCppViewApp::ViewMap ExampleCppViewApp::view_map[ViewCount] = {
    {ViewFirst, nullptr, ViewFourth},
    {ViewFourth, nullptr, ViewFifth},
    {ViewFifth, nullptr, ViewSixth},
    {ViewSixth, nullptr, ViewSeventh},
    {ViewSeventh, nullptr, ViewEighth},
    {ViewEighth, nullptr, ViewNinth},
    {ViewNinth, nullptr, ViewTenth},
    {ViewTenth, nullptr, ViewEleventh},
    {ViewEleventh, nullptr, ViewTwelfth},
    {ViewTwelfth, nullptr, ViewButtonPanel},
    {ViewButtonPanel, nullptr, ViewWidget},
    {ViewWidget, nullptr, ViewVariableList},
    {ViewVariableList, nullptr, ViewFirst},
};

ExampleCppViewApp::ExampleCppViewApp() {
    gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
    view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(view_dispatcher, this);
    view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
    view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);
}

ExampleCppViewApp::~ExampleCppViewApp() {
    if(view_dispatcher != nullptr) {
        for(size_t i = 0; i < ViewCount; i++) {
            view_dispatcher_remove_view(view_dispatcher, static_cast<ViewId>(i));
        }
        view_dispatcher_free(view_dispatcher);
        furi_record_close(RECORD_GUI);
    }
}

flipper::ViewCpp* ExampleCppViewApp::get_view(ViewId id) {
    return view_map[id].view;
}

ExampleCppViewApp::ViewId ExampleCppViewApp::get_next_view(ViewId current) {
    return view_map[current].next_view;
}

void ExampleCppViewApp::init() {
    // Initialize view map with actual view pointers
    view_map[ViewFirst].view = &first_view;
    view_map[ViewFourth].view = &fourth_view;
    view_map[ViewFifth].view = &fifth_view;
    view_map[ViewSixth].view = &sixth_view;
    view_map[ViewSeventh].view = &seventh_view;
    view_map[ViewEighth].view = &eighth_view;
    view_map[ViewNinth].view = &ninth_view;
    view_map[ViewTenth].view = &tenth_view;
    view_map[ViewEleventh].view = &eleventh_view;
    view_map[ViewTwelfth].view = &twelfth_view;
    view_map[ViewButtonPanel].view = &button_panel_view;
    view_map[ViewWidget].view = &widget_view;
    view_map[ViewVariableList].view = &variable_list_view;

    // Initialize all views
    for(size_t i = 0; i < ViewCount; i++) {
        view_map[i].view->set_view_dispatcher(view_dispatcher);
        view_map[i].view->init();
        view_dispatcher_add_view(
            view_dispatcher, static_cast<ViewId>(i), view_map[i].view->get_view());
    }
}

void ExampleCppViewApp::run() {
    view_dispatcher_switch_to_view(view_dispatcher, ViewVariableList);
    view_dispatcher_run(view_dispatcher);
}

bool ExampleCppViewApp::switch_to_view(ViewId new_view_id) {
    flipper::ViewCpp* current = get_view(current_view);
    flipper::ViewCpp* next = get_view(new_view_id);

    if(current) {
        current->exit();
    }

    if(next) {
        next->enter();
    }

    previous_view = current_view;
    current_view = new_view_id;
    view_dispatcher_switch_to_view(view_dispatcher, new_view_id);
    return true;
}

bool ExampleCppViewApp::custom_callback(void* context, uint32_t event) {
    auto* app = static_cast<ExampleCppViewApp*>(context);
    if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
        ViewId next_view = app->get_next_view(app->current_view);
        app->switch_to_view(next_view);
    }
    return true;
}

bool ExampleCppViewApp::navigation_callback(void* context) {
    furi_assert(context);
    ExampleCppViewApp* app = static_cast<ExampleCppViewApp*>(context);
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

extern "C" int32_t example_cpp_view_app(void* p) {
    UNUSED(p);
    ExampleCppViewApp app;
    app.init();
    app.run();
    return 0;
}
