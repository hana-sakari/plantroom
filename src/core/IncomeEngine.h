#pragma once

#include "User.h"
#include "PlantSpecies.h"
#include <vector>
#include <ctime>

class IncomeEngine {
public:
    explicit IncomeEngine(const std::vector<PlantSpecies> &allSpecies);

    uint32_t calculateOfflineIncome(const User &user, std::time_t from, std::time_t to) const; //how much money was produced by plants while player offline
    //think about later : different pay rate for when the app is opened vs player is offline

    void tick(User &user) const;

private:
    const std::vector<PlantSpecies> &allSpecies_;

    const PlantSpecies *findDef(const std::string &speciesId) const;
};
