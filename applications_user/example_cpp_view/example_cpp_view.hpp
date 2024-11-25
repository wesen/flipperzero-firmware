#pragma once

#include "view_cpp.hpp"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

using namespace flipper;

// Custom events for the application
enum class CustomEvent : uint32_t {
    ToggleScene = 0,
};

struct ViewModel {
    int counter{0};
    const char* message{"Hello from Second View"};
};

class FirstView : public ViewCpp {
public:
    FirstView() {
    }

    void init() override {
        // call super init
        ViewCpp::init();
        ViewModel* initial_model;
        initial_model = new ViewModel();
        set_model(initial_model);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 39, 31, "First View");

        // auto m = get_model<ViewModel>();
        ViewModel* m = static_cast<ViewModel*>(model);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 33, 41, "Counter:");
        char str[16];
        snprintf(str, sizeof(str), "%d", m->counter);
        canvas_draw_str(canvas, 78, 41, str);

        // canvas_draw_str(canvas, 24, 54, "Press OK/Right to switch");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                auto m = get_model<ViewModel>();
                m->counter++;
                return true;
            } else if(event->key == InputKeyRight) {
                send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
                return true;
            }
        }
        return false;
    }

    ViewModel initial_model;
};

class SecondView : public ViewCpp {
public:
    SecondView() {
    }

    void init() override {
        // call super init
        ViewCpp::init();
        ViewModel* initial_model;
        initial_model = new ViewModel();
        set_model(initial_model);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        UNUSED(model);
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 33, 31, "Second View");

        auto m = get_model<ViewModel>();
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 12, 41, m->message);

        canvas_draw_str(canvas, 24, 54, "Press OK/Right to switch");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk || event->key == InputKeyRight) {
                send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
                return true;
            }
        }
        return false;
    }
};

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
        // Back means exit the application, which can be done by stopping the ViewDispatcher.
        view_dispatcher_stop(app->view_dispatcher);
        return true;
        UNUSED(context);
        return false;
    }
};
