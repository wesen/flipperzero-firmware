#pragma once

#include "widget_cpp.hpp"
#include "ViewModel.hpp"
#include "example_cpp_view_icons.h"

class WidgetView : public flipper::WidgetCpp {
public:
    WidgetView() = default;

    void init() override {
        WidgetCpp::init();
        reset();

        // Add a frame
        add_frame(0, 0, 128, 64, 3);

        // Add header text
        add_string(64, 8, AlignCenter, AlignTop, FontPrimary, "Widget Demo");

        // Add multiline text
        add_string_multiline(
            5, 20, AlignLeft, AlignTop, FontSecondary, "This is a\nmultiline\ntext example");

        // Add an icon
        add_icon(90, 20, &I_ButtonRight_4x7);

        // // Add a text box with scrolling
        // add_text_box(
        //     5,
        //     40,
        //     118,
        //     20,
        //     AlignLeft,
        //     AlignTop,
        //     "This is a long text that will be wrapped and scrolled if needed...",
        //     true);

        // // Add a button
        add_button(GuiButtonTypeRight, "Next", button_callback, this);
    }

private:
    static void button_callback(GuiButtonType result, InputType type, void* context) {
        if(type == InputTypeShort && result == GuiButtonTypeRight) {
            WidgetView* instance = static_cast<WidgetView*>(context);
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
