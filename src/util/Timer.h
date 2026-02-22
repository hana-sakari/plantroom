#pragma once

#include <chrono>

class Timer {
public:
    Timer();

    void reset();

    float elapsed() const;

private:
    std::chrono::steady_clock::time_point start_;
};
