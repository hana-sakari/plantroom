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

} // namespace PlantUtils
