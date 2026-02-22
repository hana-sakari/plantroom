#pragma once

#include "User.h"
#include "Room.h"
#include "PlantDef.h"
#include <vector>
#include <ctime>

class IncomeEngine {
public:
    explicit IncomeEngine(const std::vector<PlantDef> &catalog);

    int calculateOfflineIncome(const Room &room, std::time_t from, std::time_t to) const; //how much money was produced by plants while player offline
    //think about later : different pay rate for when the app is opened vs player is offline

    void tick(User &user, const Room &room) const;

private:
    const std::vector<PlantDef> &catalog_;

    const PlantDef *findDef(const std::string &defId) const;
};
