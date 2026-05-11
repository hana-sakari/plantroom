#pragma once

#include <ftxui/component/component.hpp>
#include "core/User.h"
#include "core/PlantSpecies.h"
#include <vector>

ftxui::Component MakeRoomView(
    const User &user,
    const std::vector<PlantSpecies> &catalog,
    std::function<void(uint8_t slot, int inventoryIdx)> onPlace,
    std::function<void(uint8_t slot)> onRemove
);
