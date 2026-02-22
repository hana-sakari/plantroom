#pragma once

#include <string>
#include <vector>

enum class Rarity { Common, Rare, Exotic }; //different price and frequency of popping up at the store

struct PlantDef {
    std::string id;
    std::string name;
    std::string description;
    int price = 0;
    Rarity rarity = Rarity::Common;
    int incomePerMinute = 0;
    std::vector<int> growthDurations; //for each stage
    std::vector<std::string> asciiArt; //different for each stage
};
