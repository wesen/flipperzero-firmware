#pragma once

#include "view_cpp.hpp"
#include "ViewModel.hpp"
#include "FirstView.hpp"
#include "SecondView.hpp"
#include "ThirdView.hpp"
#include "FourthView.hpp"
#include "FifthView.hpp"
#include "SixthView.hpp"
#include "SeventhView.hpp"
#include "EighthView.hpp"
#include "NinthView.hpp"
#include "TenthView.hpp"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

using namespace flipper;

class ExampleCppViewApp {
public:
    enum ViewId {
        ViewFirst,
        ViewSecond,
        ViewThird,
        ViewFourth,
        ViewFifth,
        ViewSixth,
        ViewSeventh,
        ViewEighth,
        ViewNinth,
        ViewTenth,
    };

    enum class State {
        First,
        Second,
        Third,
        Fourth,
        Fifth,
        Sixth,
        Seventh,
        Eighth,
        Ninth,
        Tenth,
    };

    ExampleCppViewApp() = default;

    void init() {
        gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
        view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
        view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);

        // Set view dispatchers and init views
        first_view.set_view_dispatcher(view_dispatcher);
        second_view.set_view_dispatcher(view_dispatcher);
        third_view.set_view_dispatcher(view_dispatcher);
        fourth_view.set_view_dispatcher(view_dispatcher);
        fifth_view.set_view_dispatcher(view_dispatcher);
        sixth_view.set_view_dispatcher(view_dispatcher);
        seventh_view.set_view_dispatcher(view_dispatcher);
        eighth_view.set_view_dispatcher(view_dispatcher);
        ninth_view.set_view_dispatcher(view_dispatcher);
        tenth_view.set_view_dispatcher(view_dispatcher);

        first_view.init();
        second_view.init();
        third_view.init();
        fourth_view.init();
        fifth_view.init();
        sixth_view.init();
        seventh_view.init();
        eighth_view.init();
        ninth_view.init();
        tenth_view.init();

        // Add views
        view_dispatcher_add_view(view_dispatcher, ViewFirst, first_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewSecond, second_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewThird, third_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewFourth, fourth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewFifth, fifth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewSixth, sixth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewSeventh, seventh_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewEighth, eighth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewNinth, ninth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewTenth, tenth_view.get_view());
    }

    ~ExampleCppViewApp() {
        if(view_dispatcher != nullptr) {
            view_dispatcher_remove_view(view_dispatcher, ViewTenth);
            view_dispatcher_remove_view(view_dispatcher, ViewNinth);
            view_dispatcher_remove_view(view_dispatcher, ViewEighth);
            view_dispatcher_remove_view(view_dispatcher, ViewSeventh);
            view_dispatcher_remove_view(view_dispatcher, ViewSixth);
            view_dispatcher_remove_view(view_dispatcher, ViewFifth);
            view_dispatcher_remove_view(view_dispatcher, ViewFourth);
            view_dispatcher_remove_view(view_dispatcher, ViewThird);
            view_dispatcher_remove_view(view_dispatcher, ViewSecond);
            view_dispatcher_remove_view(view_dispatcher, ViewFirst);
            view_dispatcher_free(view_dispatcher);
            furi_record_close(RECORD_GUI);
        }
    }

    void run() {
        view_dispatcher_switch_to_view(view_dispatcher, ViewEighth);
        state = State::Seventh;
        view_dispatcher_run(view_dispatcher);
    }

private:
    FirstView first_view;
    SecondView second_view;
    ThirdView third_view;
    FourthView fourth_view;
    FifthView fifth_view;
    SixthView sixth_view;
    SeventhView seventh_view;
    EighthView eighth_view;
    NinthView ninth_view;
    TenthView tenth_view;
    ViewDispatcher* view_dispatcher = nullptr;
    Gui* gui = nullptr;
    State state = State::First;

    static bool custom_callback(void* context, uint32_t event) {
        auto* app = static_cast<ExampleCppViewApp*>(context);

        // Switch state
        if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
            if(app->state == State::First) {
                app->state = State::Second;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewSecond);
            } else if(app->state == State::Second) {
                app->state = State::Third;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewThird);
            } else if(app->state == State::Third) {
                app->state = State::Fourth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewFourth);
            } else if(app->state == State::Fourth) {
                app->state = State::Fifth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewFifth);
            } else if(app->state == State::Fifth) {
                app->state = State::Sixth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewSixth);
            } else if(app->state == State::Sixth) {
                app->state = State::Seventh;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewSeventh);
            } else if(app->state == State::Seventh) {
                app->state = State::Eighth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewEighth);
            } else if(app->state == State::Eighth) {
                app->state = State::Ninth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewNinth);
            } else if(app->state == State::Ninth) {
                app->state = State::Tenth;
                view_dispatcher_switch_to_view(app->view_dispatcher, ViewTenth);
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
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }
};
