#pragma once

#include <cstdint>
#include <string>
#include <ctime>
#include <vector>
#include <nlohmann/json.hpp>

enum class GrowthStage { Seedling, Growing, Mature };

enum class PlantHealth { Healthy, Wilted }; //think about later : need some time to recover from wilted or doesnt give money when wilted

struct Plant {
    uint32_t speciesId;
    GrowthStage growthStage = GrowthStage::Seedling;
    PlantHealth health = PlantHealth::Healthy;
    std::time_t placedAt = 0;
    std::time_t lastWateredAt = 0;

    void updateGrowthStage(std::time_t now, const std::vector<int> &growthDurations);
};

NLOHMANN_JSON_SERIALIZE_ENUM(GrowthStage, {{GrowthStage::Seedling, "Seedling"}, {GrowthStage::Growing, "Growing"}, {GrowthStage::Mature, "Mature"}})
NLOHMANN_JSON_SERIALIZE_ENUM(PlantHealth, {{PlantHealth::Healthy, "Healthy"}, {PlantHealth::Wilted, "Wilted"}})
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Plant, speciesId, growthStage, health, placedAt, lastWateredAt)
