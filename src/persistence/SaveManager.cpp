#include "persistence/SaveManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

SaveManager::SaveManager(std::filesystem::path saveDir) : saveDir_(std::move(saveDir)) {
    std::filesystem::create_directories(saveDir_);
}

void SaveManager::save(const User &user) const {
    auto filePath = saveDir_ / (user.name + ".json");
    std::ofstream ofs(filePath);

    json j;
    j["id"] = user.id;
    j["name"] = user.name;
    j["coins"] = user.coins;
    j["lastSavedAt"] = user.lastSavedAt;
    j["plantsInventory"] = user.plantsInventory;
    json slotsJson = json::array();
    for (const auto &slot: user.slots_) {
        slotsJson.push_back(slot.has_value() ? json(*slot) : json(nullptr));
    }
    j["slots_"] = slotsJson;

    ofs << j.dump(2);
}

User SaveManager::load(const std::string &name) const {
    auto filePath = saveDir_ / (name + ".json");
    std::ifstream ifs(filePath);

    json j;
    ifs >> j;

    User user;
    user.id = j["id"];
    user.name = j["name"];
    user.coins = j["coins"];
    user.lastSavedAt = j["lastSavedAt"];
    user.plantsInventory = j["plantsInventory"];
    const auto &slotsJson = j["slots_"];
    for (size_t i = 0; i < slotsJson.size() && i < user.slots_.size(); ++i) {
        user.slots_[i] = slotsJson[i].is_null() ? std::nullopt : std::optional<Plant>(slotsJson[i].get<Plant>());
    }

    return user;
}
