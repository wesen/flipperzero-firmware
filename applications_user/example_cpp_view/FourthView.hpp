#pragma once

#include "dialog_ex_cpp.hpp"
#include "ViewModel.hpp"

class FourthView : public flipper::DialogExCpp {
public:
    FourthView() = default;

    void init() override {
        DialogExCpp::init();

        set_header("Dialog View", 64, 10, AlignCenter, AlignCenter);
        set_text("Press buttons below\nto interact", 64, 32, AlignCenter, AlignCenter);

        set_left_button_text("Back");
        set_center_button_text("OK");
        set_right_button_text("Next");

        set_result_callback(dialog_callback);
    }

private:
    static void dialog_callback(DialogExResult result, void* context) {
        FourthView* instance = static_cast<FourthView*>(context);

        if(result == DialogExResultLeft || result == DialogExResultRight) {
            instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
        }
    }
};
