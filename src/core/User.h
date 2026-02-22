#pragma once
#include <string>
#include <ctime>

struct User { //think about identifying users
    std::string name;
    int coins = 0;
    std::time_t lastSavedAt = 0;
};
