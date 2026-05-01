#include "Game.h"
#include <utility>
#include <ctime>

Game::Game(Shop shop) : shop_(std::move(shop)) {}

void Game::placePlant(User& user, uint8_t slot, uint32_t inventoryIdx) {
    if (inventoryIdx >= user.plantsInventory.size()) return;
    if (slot >= SLOT_COUNT) return;
    if (user.slots_[slot].has_value()) return;

    user.plantsInventory[inventoryIdx].lastPlacedAt = std::time(nullptr);
    user.slots_[slot] = user.plantsInventory[inventoryIdx];
    user.plantsInventory.erase(user.plantsInventory.begin() + inventoryIdx);
}

void Game::removePlant(User& user, uint8_t slot) {
    if (slot >= SLOT_COUNT) return;
    if (!user.slots_[slot].has_value()) return;

    auto& plant = user.slots_[slot].value();
    if (plant.lastPlacedAt > 0) {
        plant.accumulatedGrowthSeconds += std::time(nullptr) - plant.lastPlacedAt;
        plant.lastPlacedAt = 0;
    }
    user.plantsInventory.push_back(std::move(plant));
    user.slots_[slot] = std::nullopt;
}
