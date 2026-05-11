#pragma once

#include <cstdint>
#include <string>
#include <ctime>
#include <vector>
#include <nlohmann/json.hpp>

enum class GrowthStage { Seedling, Growing, Mature };

enum class PlantHealth { Healthy, Wilted };

struct Plant {
    uint32_t speciesId;
    GrowthStage growthStage = GrowthStage::Seedling;
    PlantHealth health = PlantHealth::Healthy;
    std::time_t lastPlacedAt = 0; // when last moved into a slot (0 if in inventory)
    std::time_t accumulatedGrowthSeconds = 0; // total seconds spent in slots
    std::time_t lastWateredAt = 0;

    void updateGrowthStage(std::time_t now, const std::vector<int> &growthDurations);
};

NLOHMANN_JSON_SERIALIZE_ENUM(GrowthStage, {{GrowthStage::Seedling, "Seedling"}, {GrowthStage::Growing, "Growing"},
    {GrowthStage::Mature, "Mature"}})
NLOHMANN_JSON_SERIALIZE_ENUM(PlantHealth, {{PlantHealth::Healthy, "Healthy"}, {PlantHealth::Wilted, "Wilted"}})
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Plant, speciesId, growthStage, health, lastPlacedAt, accumulatedGrowthSeconds,
    lastWateredAt)
