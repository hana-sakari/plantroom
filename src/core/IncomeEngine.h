#pragma once

#include "User.h"
#include "PlantSpecies.h"
#include <vector>
#include <ctime>

class IncomeEngine {
public:
    explicit IncomeEngine(const std::vector<PlantSpecies> &allSpecies);

    uint32_t calculateOfflineIncome(const User &user, std::time_t from, std::time_t to) const;

    void tick(User &user);

private:
    const std::vector<PlantSpecies> &allSpecies_;
    double coinAccumulator_ = 0.0;

    const PlantSpecies *findDef(uint32_t speciesId) const;
};
