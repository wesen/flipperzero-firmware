#pragma once
#include "../film_developer/motor_controller.hpp"
#include "../film_developer/debug.hpp"

class MockController : public MotorController {
public:
    void clockwise(bool enable) override {
        if(enable && ccw_active) {
            DEBUG_PRINT("Warning: Enabling CW while CCW is active");
        }
        cw_active = enable;
        DEBUG_PRINT("Motor CW: %s", enable ? "ON" : "OFF");
    }

    void counterClockwise(bool enable) override {
        if(enable && cw_active) {
            DEBUG_PRINT("Warning: Enabling CCW while CW is active");
        }
        ccw_active = enable;
        DEBUG_PRINT("Motor CCW: %s", enable ? "ON" : "OFF");
    }

    void stop() override {
        cw_active = false;
        ccw_active = false;
        DEBUG_PRINT("Motor: STOP");
    }

    // Additional test helper methods
    bool isClockwise() const override {
        return cw_active;
    }
    bool isCounterClockwise() const override {
        return ccw_active;
    }
    bool isStopped() const override {
        return !cw_active && !ccw_active;
    }
    bool isRunning() const override {
        return cw_active || ccw_active;
    }

    // Add new method to get direction
    const char* getDirectionString() const override {
        if (cw_active) return "CW";
        if (ccw_active) return "CCW";
        return "Idle";
    }

private:
    bool cw_active{false};
    bool ccw_active{false};
};