#pragma once

#include "file_browser_cpp.hpp"
#include "ViewModel.hpp"

#include <gui/icon_i.h>

const uint8_t _I_badusb_10px_0[] = {0x00,0x00,0x00,0xf8,0x00,0xfc,0x01,0x74,0x01,0x24,0x01,0xfc,0x01,0xfc,0x01,0xa8,0x00,0x00,0x00,0x00,0x00,};
const uint8_t* const _I_badusb_10px[] = {_I_badusb_10px_0};

const Icon I_badusb_10px = {.width=10,.height=10,.frame_count=1,.frame_rate=0,.frames=_I_badusb_10px};
class FifthView : public flipper::FileBrowserCpp {
public:
    FifthView() : FileBrowserCpp(furi_string_alloc()) {
    }

    ~FifthView() {
        if(result_path) {
            furi_string_free(result_path);
        }
    }

    void init() override {
        FileBrowserCpp::init();
        
        // Configure browser
        configure(
            ".txt", // extension filter
            "/", // base path
            false, // skip assets
            true, // hide dot files
            &I_badusb_10px, // file icon
            true); // hide extension

        set_callback(browser_callback, this);
        
        // Start browser at root
        start(result_path);
    }

private:
    FuriString* result_path = furi_string_alloc();

    static void browser_callback(void* context) {
        FifthView* instance = static_cast<FifthView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
}; 