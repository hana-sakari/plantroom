#pragma once
#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include "Plant.h"

struct MarketListing {
    uint32_t id;
    std::string seller;
    Plant plant;
    uint32_t price;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MarketListing, id, seller, plant, price)
