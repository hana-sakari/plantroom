#pragma once

#include "core/Plant.h"
#include "core/PlantSpecies.h"
#include <vector>
#include <string>

namespace PlantUtils {

inline const PlantSpecies* findSpecies(const std::vector<PlantSpecies>& catalog, uint32_t id) {
    for (const auto& s : catalog)
        if (s.id == id) return &s;
    return nullptr;
}

inline std::string stageName(GrowthStage stage) {
    switch (stage) {
        case GrowthStage::Seedling: return "Seedling";
        case GrowthStage::Growing:  return "Growing";
        case GrowthStage::Mature:   return "Mature";
    }
    return "?";
}

inline std::string rarityName(Rarity r) {
    switch (r) {
        case Rarity::Common: return "Common";
        case Rarity::Rare:   return "Rare";
        case Rarity::Exotic: return "Exotic";
    }
    return "?";
}

} // namespace PlantUtils
