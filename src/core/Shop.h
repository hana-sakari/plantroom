#pragma once

#include "PlantSpecies.h"
#include "User.h"
#include <vector>
#include <string>

enum class BuyResult { Success, NotEnoughCoins, NotFound };

class Shop {
public:
    explicit Shop(const std::vector<PlantSpecies> &catalog);

    auto getAvailable() const -> const std::vector<PlantSpecies> &;

    auto buy(const std::string &speciesId, User &user) -> BuyResult;

private:
    const std::vector<PlantSpecies> catalog_;

    auto findSpecies(const std::string &speciesId) const -> const PlantSpecies *;
};
