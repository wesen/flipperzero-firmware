#pragma once

class MotorController {
public:
    MotorController() : is_running_(false), direction_(0) {}

    void start_clockwise() {
        is_running_ = true;
        direction_ = 1;
    }

    void start_counter_clockwise() {
        is_running_ = true;
        direction_ = -1;
    }

    void stop() {
        is_running_ = false;
        direction_ = 0;
    }

    bool is_running() const { return is_running_; }
    int get_direction() const { return direction_; }

    static MotorController& instance() {
        static MotorController instance;
        return instance;
    }

private:
    bool is_running_;
    int direction_;  // 1 for CW, -1 for CCW, 0 for stopped
}; 