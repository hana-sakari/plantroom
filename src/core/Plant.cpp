#include "Plant.h"

void Plant::updateGrowthStage(std::time_t now, const std::vector<int>& growthDurations) {
    if (growthDurations.empty()) return;
    std::time_t age = accumulatedGrowthSeconds;
    if (lastPlacedAt > 0) age += now - lastPlacedAt;

    std::time_t toGrowing = growthDurations[0];
    std::time_t toMature = growthDurations.size() >= 2
        ? growthDurations[0] + growthDurations[1]
        : toGrowing;

    if (age >= toMature)       growthStage = GrowthStage::Mature;
    else if (age >= toGrowing) growthStage = GrowthStage::Growing;
    else                       growthStage = GrowthStage::Seedling;
}
