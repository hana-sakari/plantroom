#pragma once

#include "Plant.h"
#include <vector>

class Inventory {
public:
    void add(const Plant &plant);

    bool remove(int index);

    const Plant &get(int index) const;

    int size() const;

    bool empty() const;

    const std::vector<Plant> &getAll() const;

private:
    std::vector<Plant> plants_;
};
