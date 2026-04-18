#pragma once

#include "core/Game.h"
#include "core/User.h"
#include "persistence/SaveManager.h"


class App { //responsible only for ui and inputs from user
    Game game;
    User user;
    SaveManager saveManager;

    uint8_t currentTab = 0;

    void onBuyPlant(uint32_t speciesId);
    void onPlacePlant(uint8_t slot, uint32_t plantId);
    void onRemovePlant(uint8_t slot);
public:
    explicit App(const std::string& username, Game& game);
    void run();
};
