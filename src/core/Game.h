#pragma once

#include "Shop.h"
class Game {
    Shop shop_;
public:
    explicit Game(const Shop& shop);
    const Shop& getShop() const {return catalog;}

};
