#pragma once

#include "views/variable_item_list_cpp.hpp"
#include "ViewModel.hpp"

class VariableListTestView : public flipper::VariableItemListCpp {
public:
    VariableListTestView() = default;

    void init() override {
        VariableItemListCpp::init();

        // Add test items
        brightness_item = add_item("Brightness", 5, brightness_change_callback, this);
        set_current_value_index(brightness_item, 2);
        set_current_value_text(brightness_item, "60%");

        volume_item = add_item("Volume", 10, volume_change_callback, this);
        set_current_value_index(volume_item, 7);
        set_current_value_text(volume_item, "70%");

        mode_item = add_item("Mode", 3, mode_change_callback, this);
        set_current_value_index(mode_item, 0);
        set_current_value_text(mode_item, "Normal");

        set_enter_callback(enter_callback, this);
    }

private:
    VariableItem* brightness_item = nullptr;
    VariableItem* volume_item = nullptr;
    VariableItem* mode_item = nullptr;

    static void brightness_change_callback(VariableItem* item) {
        uint8_t index = get_current_value_index(item);
        char str[8];
        snprintf(str, sizeof(str), "%d%%", (index + 1) * 20);
        set_current_value_text(item, str);
    }

    static void volume_change_callback(VariableItem* item) {
        uint8_t index = get_current_value_index(item);
        char str[8];
        snprintf(str, sizeof(str), "%d%%", (index + 1) * 10);
        set_current_value_text(item, str);
    }

    static void mode_change_callback(VariableItem* item) {
        uint8_t index = get_current_value_index(item);
        const char* modes[] = {"Normal", "Sport", "Eco"};
        set_current_value_text(item, modes[index]);
    }

    static void enter_callback(void* context, uint32_t index) {
        UNUSED(index);
        VariableListTestView* instance = static_cast<VariableListTestView*>(context);
        instance->send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
    }
};
