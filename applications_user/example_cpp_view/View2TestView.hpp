#pragma once

#include "../film_developer/views/view_cpp.hpp"
#include "ViewModel.hpp"

class View2TestView : public flipper::ViewCpp {
public:
    View2TestView() = default;

    void init() override {
        ViewCpp::init();
        ViewModel* initial_model;
        initial_model = new ViewModel();
        set_model(initial_model);
    }

protected:
    void draw(Canvas* canvas, void* model) override {
        UNUSED(model);
        canvas_clear(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 33, 31, "Second View");

        auto m = get_model<ViewModel>();
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 12, 41, m->message);

        canvas_draw_str(canvas, 5, 54, "Press OK/Right to switch");
    }

    bool input(InputEvent* event) override {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk || event->key == InputKeyRight) {
                send_custom_event(static_cast<uint32_t>(CustomEvent::ToggleScene));
                return true;
            }
        }
        return false;
    }
};