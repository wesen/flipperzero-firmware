#pragma once

#include "../film_developer/views/submenu_cpp.hpp"
#include "ViewModel.hpp"

class SubmenuTestView : public flipper::SubMenuCpp {
public:
    SubmenuTestView() = default;

    void init() override {
        SubMenuCpp::init();
        set_header("Menu View");

        // Add some example menu items
        add_item("Option 1", 0, menu_callback, this);
        add_item("Option 2", 1, menu_callback, this);
        add_item("Option 3", 2, menu_callback, this);
        add_item("Switch View", 3, menu_callback, this);
    }

private:
    static void menu_callback(void* context, uint32_t index) {
        SubmenuTestView* instance = static_cast<SubmenuTestView*>(context);
        if(index == 3) { // Switch View option
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
