#pragma once

#include "views/view_cpp.hpp"
#include "FirstView.hpp"
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

using namespace flipper;

class ExampleCppViewApp {
public:
    enum ViewId {
        ViewFirst,
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
        ViewCount // Keep this last
    };

    struct ViewMap {
        ViewId id;
        ViewCpp* view;
        ViewId next_view;
    };

    ExampleCppViewApp();
    ~ExampleCppViewApp();

    void init();
    void run();
    bool switch_to_view(ViewId view_id);

private:
    // Views
    FirstView first_view;
    FourthView fourth_view;
    FifthView fifth_view;
    SixthView sixth_view;
    SeventhView seventh_view;
    EighthView eighth_view;
    NinthView ninth_view;
    TenthView tenth_view;
    EleventhView eleventh_view;
    TwelfthView twelfth_view;
    ButtonPanelView button_panel_view;
    WidgetView widget_view;

    // View management
    ViewDispatcher* view_dispatcher = nullptr;
    Gui* gui = nullptr;
    ViewId current_view = ViewWidget;
    ViewId previous_view = ViewWidget;

    static ViewMap view_map[ViewCount];
    ViewCpp* get_view(ViewId id);
    ViewId get_next_view(ViewId current);

    static bool custom_callback(void* context, uint32_t event);
    static bool navigation_callback(void* context);
};
