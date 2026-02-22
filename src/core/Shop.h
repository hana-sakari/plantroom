#pragma once

#include "PlantDef.h"
#include "User.h"
#include "Inventory.h"
#include <vector>
#include <string>

enum class BuyResult { Success, NotEnoughCoins, NotFound };

class Shop {
public:
    explicit Shop(const std::vector<PlantDef> &catalog);

    const std::vector<PlantDef> &getAvailable() const;

    BuyResult buy(const std::string &defId, User &user, Inventory &inventory);

private:
    const std::vector<PlantDef> &catalog_;

    const PlantDef *findDef(const std::string &defId) const;
};
