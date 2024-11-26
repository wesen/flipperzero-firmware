#pragma once

#include "../film_developer/views/loading_cpp.hpp"
#include "ViewModel.hpp"

class FifthView : public flipper::LoadingCpp {
public:
    FifthView() = default;

    void init() override {
        LoadingCpp::init();
        // Loading view is self-contained and auto-animates
    }
}; 