#pragma once

#include "../film_developer/views/view_cpp.hpp"
#include "ViewModel.hpp"

class FirstView : public flipper::ViewCpp {
public:
    FirstView() = default;

    void init() override {
        ViewCpp::init();
        ViewModel* initial_model;
        initial_model = new ViewModel();
        set_model(initial_model);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        UNUSED(canvas);
        UNUSED(model);
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 39, 31, "First View");

        ViewModel* m = static_cast<ViewModel*>(model);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 33, 41, "Counter:");
        char str[16];
        snprintf(str, sizeof(str), "%d", m->counter);
        canvas_draw_str(canvas, 78, 41, str);

        canvas_draw_str(canvas, 5, 54, "Press OK/Right to switch");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                auto m = get_model<ViewModel>();
                m->counter++;
                return true;
            } else if(event->key == InputKeyRight) {
                send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
                return true;
            }
        }
        return false;
    }

private:
    ViewModel initial_model;
}; 