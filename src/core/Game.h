#pragma once

#include "Shop.h"
class Game {
    Shop shop_;
public:
    explicit Game(Shop  shop);
    const Shop& getShop() const {return shop_;}
    void placePlant(User& user, uint8_t slot, uint32_t inventoryIdx);
    void removePlant(User& user, uint8_t slot);
};
