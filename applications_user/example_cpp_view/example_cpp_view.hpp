#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/button_panel.h>

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
#include "VariableListTestView.hpp"

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
        ViewVariableList,
        ViewCount,
    };

    struct ViewMap {
        ViewId id;
        flipper::ViewCpp* view;
        ViewId next_view;
    };

    ExampleCppViewApp();
    ~ExampleCppViewApp();

    void init();
    void run();

private:
    static ViewMap view_map[ViewCount];
    static bool custom_callback(void* context, uint32_t event);
    static bool navigation_callback(void* context);

    bool switch_to_view(ViewId new_view_id);
    flipper::ViewCpp* get_view(ViewId id);
    ViewId get_next_view(ViewId current);

    Gui* gui = nullptr;
    ViewDispatcher* view_dispatcher = nullptr;
    ViewId current_view = ViewFirst;
    ViewId previous_view = ViewFirst;

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
    VariableListTestView variable_list_view;
};
