#pragma once

#include "Shop.h"
class Game {
    Shop shop_;
public:
    explicit Game(Shop  shop);
    const Shop& getShop() const {return shop_;}

};
