#include "Shop.h"
#include <algorithm>

Shop::Shop(const std::vector<PlantSpecies>& catalog) : catalog_(catalog) {}

auto Shop::getAvailable() const -> const std::vector<PlantSpecies>& {
    return catalog_;
}

auto Shop::findSpecies(const std::string& speciesId) const -> const PlantSpecies* {
    auto it = std::find_if(catalog_.begin(), catalog_.end(),
        [&](const PlantSpecies& s) { return std::to_string(s.id) == speciesId; });
    return it == catalog_.end() ? nullptr : &(*it);
}

auto Shop::buy(const std::string& speciesId, User& user) -> BuyResult {
    const PlantSpecies* species = findSpecies(speciesId);
    if (!species) return BuyResult::NotFound;
    if (user.coins < static_cast<uint32_t>(species->price)) return BuyResult::NotEnoughCoins;

    user.coins -= species->price;
    user.plantsInventory.push_back(Plant{speciesId, GrowthStage::Seedling, PlantHealth::Healthy, 0, 0});
    return BuyResult::Success;
}
