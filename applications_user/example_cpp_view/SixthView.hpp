#pragma once

#include "../film_developer/views/text_box_cpp.hpp"
#include "ViewModel.hpp"

class SixthView : public flipper::TextBoxCpp {
public:
    SixthView() = default;

    void init() override {
        TextBoxCpp::init();

        set_text("This is a text box view.\n\n"
                 "It supports multiline text\n"
                 "and automatic scrolling.\n\n"
                 "Press OK/Right to continue.");

        set_font(TextBoxFontText);
        set_focus(TextBoxFocusStart);

        // view_set_input_callback(view, input_callback);
    }

    // bool input(InputEvent* event) override {
    //     if(event->type == InputTypeShort) {
    //         if(event->key == InputKeyOk || event->key == InputKeyRight) {
    //             send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    //             return true;
    //         }
    //     }
    //     return false;
    // }

private:
    static bool input_callback(InputEvent* event, void* context) {
        return static_cast<SixthView*>(context)->input(event);
    }
};
