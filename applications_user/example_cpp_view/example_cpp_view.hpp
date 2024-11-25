#pragma once

#include "view_cpp.hpp"
#include "ViewModel.hpp"
#include "FirstView.hpp"
#include "SecondView.hpp"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

using namespace flipper;

class ExampleCppViewApp {
public:
    enum ViewId {
        ViewFirst,
        ViewSecond,
    };

    enum class State {
        First,
        Second,
    };

    ExampleCppViewApp() = default;

    void init() {
        gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
        view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
        view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);

        // Set view dispatchers
        first_view.set_view_dispatcher(view_dispatcher);
        second_view.set_view_dispatcher(view_dispatcher);

        first_view.init();
        second_view.init();

        // Add views
        view_dispatcher_add_view(view_dispatcher, ViewFirst, first_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewSecond, second_view.get_view());
    }

    ~ExampleCppViewApp() {
        if(view_dispatcher != nullptr) {
            view_dispatcher_remove_view(view_dispatcher, ViewSecond);
            view_dispatcher_remove_view(view_dispatcher, ViewFirst);
            view_dispatcher_free(view_dispatcher);
            furi_record_close(RECORD_GUI);
        }
    }

    void run() {
        view_dispatcher_switch_to_view(view_dispatcher, ViewFirst);
        view_dispatcher_run(view_dispatcher);
    }

private:
    FirstView first_view;
    SecondView second_view;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    State state = State::First;

    static bool custom_callback(void* context, uint32_t event) {
        auto* app = static_cast<ExampleCppViewApp*>(context);

        // Switch state
        if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
            if(app->state == State::First) {
                app->state = State::Second;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewSecond);
            } else {
                app->state = State::First;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewFirst);
            }
        }

        return true;
    }

    static bool navigation_callback(void* context) {
        furi_assert(context);
        ExampleCppViewApp* app = static_cast<ExampleCppViewApp*>(context);
        // Back means exit the application
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }
};
