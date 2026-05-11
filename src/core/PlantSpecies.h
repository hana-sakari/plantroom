#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class Rarity { Common, Rare, Exotic }; //different price and frequency of popping up at the store

struct PlantSpecies {
    uint32_t id;
    std::string name;
    std::string description;
    uint32_t price = 0;
    Rarity rarity = Rarity::Common;
    uint32_t incomePerMinute = 0;
    std::vector<int> growthDurations; //for each stage
    std::vector<std::string> asciiArt; //different for each stage
};


NLOHMANN_JSON_SERIALIZE_ENUM(Rarity, {{Rarity::Common, "Common"}, {Rarity::Rare, "Rare"}, {Rarity::Exotic, "Exotic"}})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlantSpecies, id, name, description, price, rarity, incomePerMinute, growthDurations,
    asciiArt)
