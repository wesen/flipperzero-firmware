#pragma once

#include "../film_developer/views/button_menu_cpp.hpp"
#include "ViewModel.hpp"

class ButtonMenuTestView : public flipper::ButtonMenuCpp {
public:
    ButtonMenuTestView() = default;

    void init() override {
        ButtonMenuCpp::init();

        set_header("Button Menu Test");

        // Add some test buttons
        add_item("Control Button", 0, button_callback, ButtonMenuItemTypeControl, this);
        add_item("Common Button", 1, button_callback, ButtonMenuItemTypeCommon, this);
        add_item("Next View", 2, button_callback, ButtonMenuItemTypeControl, this);
    }

private:
    static void button_callback(void* context, int32_t index, InputType type) {
        ButtonMenuTestView* instance = static_cast<ButtonMenuTestView*>(context);
        if(index == 2 && type == InputTypeShort) {
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
