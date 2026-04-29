#pragma once

#include <ftxui/component/component.hpp>
#include "core/PlantSpecies.h"
#include <vector>

ftxui::Component MakeShopView(
    const std::vector<PlantSpecies>& catalog,
    std::function<void(uint32_t speciesId)> onBuy
    );
