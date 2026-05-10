#pragma once
#include <ftxui/component/component.hpp>
#include <functional>
#include <string>
#include <vector>
#include "client/ServerConnection.h"
#include "core/MarketListing.h"
#include "core/PlantSpecies.h"
#include "core/User.h"

ftxui::Component MakeMarketView(
    const std::vector<MarketListing> &listings,
    const std::vector<PlantSpecies> &catalog,
    const User &user,
    const std::string &currentUsername,
    std::function<ActionResult(uint32_t listingId)> onBuy,
    std::function<ActionResult(uint32_t inventoryIdx, uint32_t price)> onPost,
    std::function<ActionResult(uint32_t listingId)> onRemove
);
