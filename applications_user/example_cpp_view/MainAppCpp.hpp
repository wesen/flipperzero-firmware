#pragma once

#include "view_cpp.hpp"
#include "scene_manager_cpp.hpp"
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
#include "EleventhView.hpp"
#include "TwelfthView.hpp"
#include "ButtonPanelView.hpp"
#include "WidgetView.hpp"
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

class MainAppCpp {
public:
    // View IDs for the view dispatcher
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
        ViewEleventh,
        ViewTwelfth,
        ViewButtonPanel,
        ViewWidget,
    };

    // Scene IDs for the scene manager
    enum SceneId {
        SceneFirst,
        SceneSecond,
        SceneThird,
        SceneFourth,
        SceneFifth,
        SceneSixth,
        SceneSeventh,
        SceneEighth,
        SceneNinth,
        SceneTenth,
        SceneEleventh,
        SceneTwelfth,
        SceneButtonPanel,
        SceneWidget,
    };

    MainAppCpp() = default;

    void init() {
        // Initialize GUI and view dispatcher
        gui = static_cast<Gui*>(furi_record_open(RECORD_GUI));
        view_dispatcher = view_dispatcher_alloc();
        view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
        view_dispatcher_set_event_callback_context(view_dispatcher, this);
        view_dispatcher_set_custom_event_callback(view_dispatcher, custom_callback);
        view_dispatcher_set_navigation_event_callback(view_dispatcher, navigation_callback);

        // Initialize views
        initialize_views();

        // Initialize scene manager and add scenes
        initialize_scenes();
    }

    ~MainAppCpp() {
        if(view_dispatcher != nullptr) {
            remove_views();
            view_dispatcher_free(view_dispatcher);
            furi_record_close(RECORD_GUI);
        }
    }

    void run() {
        // Start with the widget view
        scene_manager.next_scene(SceneWidget);
        view_dispatcher_run(view_dispatcher);
    }

private:
    void initialize_views() {
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
        eleventh_view.set_view_dispatcher(view_dispatcher);
        twelfth_view.set_view_dispatcher(view_dispatcher);
        button_panel_view.set_view_dispatcher(view_dispatcher);
        widget_view.set_view_dispatcher(view_dispatcher);

        // Initialize all views
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
        eleventh_view.init();
        twelfth_view.init();
        button_panel_view.init();
        widget_view.init();

        // Add views to dispatcher
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
        view_dispatcher_add_view(view_dispatcher, ViewEleventh, eleventh_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewTwelfth, twelfth_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewButtonPanel, button_panel_view.get_view());
        view_dispatcher_add_view(view_dispatcher, ViewWidget, widget_view.get_view());
    }

    void initialize_scenes() {
        // Add scenes to scene manager
        scene_manager.add_scene(
            SceneFirst,
            {[this]() { view_dispatcher_switch_to_view(view_dispatcher, ViewFirst); },
             [this](uint32_t event) {
                 if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
                     scene_manager.next_scene(SceneSecond);
                     return true;
                 }
                 return false;
             },
             [this]() { /* Exit handler */ }});

        scene_manager.add_scene(
            SceneSecond,
            {[this]() { view_dispatcher_switch_to_view(view_dispatcher, ViewSecond); },
             [this](uint32_t event) {
                 if(event == static_cast<uint32_t>(CustomEvent::ToggleScene)) {
                     scene_manager.next_scene(SceneThird);
                     return true;
                 }
                 return false;
             },
             [this]() { /* Exit handler */ }});

        // Add similar scene handlers for other views...
        // For brevity, not all scenes are shown here
    }

    void remove_views() {
        view_dispatcher_remove_view(view_dispatcher, ViewWidget);
        view_dispatcher_remove_view(view_dispatcher, ViewButtonPanel);
        view_dispatcher_remove_view(view_dispatcher, ViewTwelfth);
        view_dispatcher_remove_view(view_dispatcher, ViewEleventh);
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
    }

    static bool custom_callback(void* context, uint32_t event) {
        auto* app = static_cast<MainAppCpp*>(context);
        return app->scene_manager.handle_event(event);
    }

    static bool navigation_callback(void* context) {
        auto* app = static_cast<MainAppCpp*>(context);
        bool handled = app->scene_manager.previous_scene();
        if(!handled) {
            view_dispatcher_stop(app->view_dispatcher);
        }
        return true;
    }

    // Views
    ViewTestView first_view;
    View2TestView second_view;
    SubmenuTestView third_view;
    DialogExTestView fourth_view;
    LoadingTestView fifth_view;
    TextBoxTestView sixth_view;
    FileBrowserTestView seventh_view;
    ByteInputTestView eighth_view;
    NumberInputTestView ninth_view;
    TextInputTestView tenth_view;
    PopupTestView eleventh_view;
    ButtonMenuTestView twelfth_view;
    ButtonPanelView button_panel_view;
    WidgetView widget_view;

    // Core components
    ViewDispatcher* view_dispatcher = nullptr;
    Gui* gui = nullptr;
    flipper::SceneManagerCpp scene_manager;
};
