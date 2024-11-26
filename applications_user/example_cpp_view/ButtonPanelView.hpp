#pragma once

#include "../film_developer/views/button_panel_cpp.hpp"
#include "ViewModel.hpp"
#include "example_cpp_view_icons.h"

class ButtonPanelView : public flipper::ButtonPanelCpp {
public:
    ButtonPanelView() = default;

    void init() override {
        ButtonPanelCpp::init();

        // Reserve a 2x3 grid
        reserve(2, 3);

        // Add header label - centered in 64px width
        add_label(5, 8, FontPrimary, "Button Panel");
        add_label(5, 20, FontSecondary, "Press buttons to interact");

        // Add buttons in a compact grid layout
        // Row 1: Left/Right
        add_item(
            0, 0, 0, 8, 32, &I_ButtonLeft_4x7, &I_ButtonLeftPressed_4x7, button_callback, this);
        add_item(
            1, 1, 0, 44, 32, &I_ButtonRight_4x7, &I_ButtonRightPressed_4x7, button_callback, this);

        // Row 2: Up/Down
        add_item(
            2, 0, 1, 8, 45, &I_ButtonUp_7x4, &I_ButtonUpPressed_7x4, button_callback, this);
        add_item(
            3, 1, 1, 44, 45, &I_ButtonDown_7x4, &I_ButtonDownPressed_7x4, button_callback, this);
    }

private:
    static void button_callback(void* context, uint32_t index) {
        UNUSED(index);
        ButtonPanelView* instance = static_cast<ButtonPanelView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
