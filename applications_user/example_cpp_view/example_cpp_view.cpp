#include "example_cpp_view.hpp"

ExampleCppViewApp* app;

class Foobar {
public:
    Foobar() {
    }
    ~Foobar() {
    }
};

extern "C" int32_t example_cpp_view_app(void* p) {
    UNUSED(p);
    app = new ExampleCppViewApp();
    app->init();
    app->run();
    delete app;
    return 0;
}
