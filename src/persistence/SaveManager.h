#pragma once

#include "core/User.h"
#include "core/Room.h"
#include "core/Inventory.h"
#include <filesystem>
#include <string>

struct SaveData {
    User user;
    Room room;
    Inventory inventory;
};

class SaveManager {
public:
    explicit SaveManager(std::filesystem::path saveDir);

    void save(const User &user, const Room &room, const Inventory &inventory) const;

    SaveData load(const std::string &name) const;

private:
    std::filesystem::path saveDir_;
};
