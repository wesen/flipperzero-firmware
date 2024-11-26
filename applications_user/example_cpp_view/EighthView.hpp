#pragma once

#include "views/byte_input_cpp.hpp"
#include "ViewModel.hpp"

class EighthView : public flipper::ByteInputCpp {
public:
    EighthView() = default;

    void init() override {
        ByteInputCpp::init();
        
        set_header_text("Byte Input Test");
        
        // Set up byte input with test data
        bytes[0] = 0xAB;
        bytes[1] = 0xCD;
        bytes[2] = 0xEF;
        
        set_result_callback(byte_input_callback, byte_changed_callback, this, bytes, 3);
    }

private:
    uint8_t bytes[3];

    static void byte_input_callback(void* context) {
        EighthView* instance = static_cast<EighthView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }

    static void byte_changed_callback(void* context) {
        UNUSED(context);
        // Handle byte value changes here if needed
    }
}; 