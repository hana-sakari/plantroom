#pragma once

#include "client/ServerConnection.h"
#include "core/MarketListing.h"
#include "core/PlantSpecies.h"
#include "core/Shop.h"
#include "core/User.h"
#include <vector>

class App {
    ServerConnection& conn_;
    User user_;
    std::vector<PlantSpecies> catalog_;
    std::vector<MarketListing> market_;

    BuyResult onBuyPlant(uint32_t speciesId);
    void onPlacePlant(uint8_t slot, uint32_t inventoryIdx);
    void onRemovePlant(uint8_t slot);
public:
    explicit App(ServerConnection& conn);
    void run();
};
