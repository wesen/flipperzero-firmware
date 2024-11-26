#pragma once

#include "../film_developer/views/text_input_cpp.hpp"
#include "ViewModel.hpp"

class TextInputTestView : public flipper::TextInputCpp {
public:
    TextInputTestView() = default;

    void init() override {
        TextInputCpp::init();

        set_header_text("Text Input Test");

        // Set up text input with buffer
        set_result_callback(text_input_callback, this, text_buffer, sizeof(text_buffer), true);
        set_minimum_length(1);
    }

private:
    char text_buffer[32] = "Test";

    static void text_input_callback(void* context) {
        TextInputTestView* instance = static_cast<TextInputTestView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
