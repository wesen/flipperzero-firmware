#pragma once

#include "../film_developer/views/popup_cpp.hpp"
#include "ViewModel.hpp"
#include <assets_icons.h>

class EleventhView : public flipper::PopupCpp {
public:
    EleventhView() = default;

    void init() override {
        PopupCpp::init();

        set_header("Popup Test", 64, 10, AlignCenter, AlignCenter);
        set_text("This is a popup\nwith timeout", 64, 32, AlignCenter, AlignCenter);
        set_icon(0, 12, &I_DolphinSaved_92x58);

        set_callback(popup_callback);
        set_context(this);

        set_timeout(3000); // 3 seconds
        enable_timeout();
    }

private:
    static void popup_callback(void* context) {
        EleventhView* instance = static_cast<EleventhView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
