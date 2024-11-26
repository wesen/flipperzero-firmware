#pragma once

#include "views/file_browser_cpp.hpp"
#include "ViewModel.hpp"
#include <storage/storage.h>

#include <gui/icon_i.h>

const uint8_t _I_badusb_10px_0[] = {
    0x00, 0x00, 0x00, 0xf8, 0x00, 0xfc, 0x01, 0x74, 0x01, 0x24, 0x01,
    0xfc, 0x01, 0xfc, 0x01, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const uint8_t* const _I_badusb_10px[] = {_I_badusb_10px_0};

const Icon I_badusb_10px =
    {.width = 10, .height = 10, .frame_count = 1, .frame_rate = 0, .frames = _I_badusb_10px};

class SeventhView : public flipper::FileBrowserCpp {
public:
    SeventhView()
        : FileBrowserCpp(furi_string_alloc()) {
    }

    ~SeventhView() {
        if(result_path) {
            furi_string_free(result_path);
        }
    }

    void init() override {
        FileBrowserCpp::init();

        configure(
            ".txt", // Extension filter
            "/", // Base path
            false, // Skip assets
            true, // Hide dot files
            &I_badusb_10px, // File icon
            true // Hide extension
        );

        set_callback(file_browser_callback, this);
        start(result_path);
    }

protected:
    // These are handled by the dialog internally
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
    }

private:
    static void file_browser_callback(void* context) {
        SeventhView* instance = static_cast<SeventhView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
