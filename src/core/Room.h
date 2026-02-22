#pragma once

#include "Plant.h"
#include <array>
#include <optional>
#include <string>

inline constexpr int SLOT_COUNT = 6;

struct SlotInfo {
    std::string name;
};

class Room {
public:
    Room();

    bool placePlant(int slot, const Plant &plant);

    std::optional<Plant> removePlant(int slot);

    bool hasPlant(int slot) const;

    //different methods for updating the plant or outputting the plant
    Plant *getPlant(int slot);
    const Plant *getPlant(int slot) const;


    const SlotInfo &getSlotInfo(int slot) const;

private:
    std::array<SlotInfo, SLOT_COUNT> slotInfos_;
    std::array<std::optional<Plant>, SLOT_COUNT> slots_;
};
