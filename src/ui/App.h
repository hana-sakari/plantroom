#pragma once

#include "core/Game.h"
#include "core/User.h"
#include "core/IncomeEngine.h"
#include "persistence/SaveManager.h"
#include <filesystem>

class App {
    Game game;
    User user;
    SaveManager saveManager;
    IncomeEngine engine;

    uint8_t currentTab = 0;

    BuyResult onBuyPlant(uint32_t speciesId);
    void onPlacePlant(uint8_t slot, uint32_t inventoryIdx);
    void onRemovePlant(uint8_t slot);
public:
    explicit App(const std::string& username, Game& game, std::filesystem::path saveDir);
    void run();
};
