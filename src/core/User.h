#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <ctime>
#include <optional>
#include "Plant.h"

inline constexpr int SLOT_COUNT = 6;


struct User { //think about identifying users
    uint32_t id;
    std::string name;
    uint32_t coins = 0;
    std::time_t lastSavedAt = 0;
    std::vector<Plant> plantsInventory;
    std::array<std::optional<Plant>, SLOT_COUNT> slots_;
};


