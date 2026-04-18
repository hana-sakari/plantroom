#include "persistence/SaveManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

SaveManager::SaveManager(std::filesystem::path saveDir) : saveDir_(std::move(saveDir)) {}

void SaveManager::save(const User& user) const {
    auto filePath = saveDir_ / (user.name + ".json");
    std::ofstream ofs(filePath);

    std::vector<json> slotsJson;
    for (const auto& slot : user.slots_) {
        slotsJson.push_back(slot);
    }

    json j;
    j["id"] = user.id;
    j["name"] = user.name;
    j["coins"] = user.coins;
    j["lastSavedAt"] = user.lastSavedAt;
    j["plantsInventory"] = user.plantsInventory;
    j["slots_"] = slotsJson;

    ofs << j.dump(2);
}

User SaveManager::load(const std::string& name) const {
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

    auto slotsJson = j["slots_"].get<std::vector<json>>();
    for (size_t i = 0; i < slotsJson.size() && i < user.slots_.size(); ++i) {
        if (!slotsJson[i].is_null()) {
            user.slots_[i] = slotsJson[i].get<Plant>();
        }
    }

    return user;
}
