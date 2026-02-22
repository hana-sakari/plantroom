#pragma once

#include <string>
#include <ctime>
#include <vector>

enum class GrowthStage { Seedling, Growing, Mature };

enum class PlantHealth { Healthy, Wilted }; //think about later : need some time to recover from wilted or doesnt give money when wilted

struct Plant {
    std::string defId;
    int slot = -1; //default not placed anywhere
    GrowthStage growthStage = GrowthStage::Seedling;
    PlantHealth health = PlantHealth::Healthy;
    std::time_t placedAt = 0;
    std::time_t lastWateredAt = 0;

    void updateGrowthStage(std::time_t now, const std::vector<int> &growthDurations);
};
