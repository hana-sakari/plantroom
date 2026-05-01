#include "IncomeEngine.h"
#include <algorithm>
#include <ctime>

IncomeEngine::IncomeEngine(const std::vector<PlantSpecies>& allSpecies)
    : allSpecies_(allSpecies) {}

const PlantSpecies* IncomeEngine::findDef(uint32_t speciesId) const {
    auto it = std::find_if(allSpecies_.begin(), allSpecies_.end(),
        [&](const PlantSpecies& s) { return s.id == speciesId; });
    return it == allSpecies_.end() ? nullptr : &(*it);
}

void IncomeEngine::tick(User& user) {
    std::time_t now = std::time(nullptr);
    double incomeThisTick = 0.0;

    for (auto& slot : user.slots_) {
        if (!slot.has_value()) continue;
        auto& plant = *slot;
        const PlantSpecies* def = findDef(plant.speciesId);
        if (!def) continue;

        plant.updateGrowthStage(now, def->growthDurations);
        incomeThisTick += def->incomePerMinute / 60.0;
    }

    coinAccumulator_ += incomeThisTick;
    uint32_t earned = static_cast<uint32_t>(coinAccumulator_);
    coinAccumulator_ -= earned;
    user.coins += earned;
}

static constexpr double MAX_OFFLINE_SECONDS = 8 * 60 * 60; // 8 hours

uint32_t IncomeEngine::calculateOfflineIncome(const User& user, std::time_t from, std::time_t to) const {
    double seconds = std::min(static_cast<double>(to - from), MAX_OFFLINE_SECONDS);
    if (seconds <= 0) return 0;

    double total = 0.0;
    for (const auto& slot : user.slots_) {
        if (!slot.has_value()) continue;
        const PlantSpecies* def = findDef(slot->speciesId);
        if (!def) continue;
        total += def->incomePerMinute / 60.0 * seconds;
    }
    return static_cast<uint32_t>(total*0.5); //user gets only half the income when offline
}
