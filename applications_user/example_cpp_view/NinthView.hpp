#pragma once

#include "../film_developer/views/number_input_cpp.hpp"
#include "ViewModel.hpp"

class NinthView : public flipper::NumberInputCpp {
public:
    NinthView() = default;

    void init() override {
        NumberInputCpp::init();
        
        set_header_text("Number Input Test");
        
        // Set up number input with range 0-100, starting at 50
        set_result_callback(number_input_callback, this, 50, 0, 100);
    }

private:
    static void number_input_callback(void* context, int32_t number) {
        UNUSED(number);
        NinthView* instance = static_cast<NinthView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
}; 