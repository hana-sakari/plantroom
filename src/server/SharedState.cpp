#include "server/SharedState.h"
#include "persistence/CatalogLoader.h"
#include "persistence/SaveManager.h"
#include <fstream>
#include <nlohmann/json.hpp>

SharedState::SharedState(const std::filesystem::path &dataDir) : dataDir_(dataDir) {
    catalog_ = CatalogLoader::load(dataDir / "plants.json");

    std::ifstream shopFile(dataDir / "shop.json");
    if (shopFile.is_open()) {
        auto ids = nlohmann::json::parse(shopFile).get<std::vector<uint32_t> >();
        for (uint32_t id: ids) {
            auto it = std::ranges::find_if(catalog_,
                [id](const PlantSpecies &s) { return s.id == id; });
            if (it != catalog_.end())
                shopListings_.push_back(*it);
        }
    } else {
        shopListings_ = catalog_;
    }
}

// Returns a raw pointer into allUsers_
// shared lock on userMutexes_[username] before calling
// and should not hold it after the user may be removed
User *SharedState::getUser(const std::string &username) {
    auto it = allUsers_.find(username);
    return it != allUsers_.end() ? &it->second : nullptr;
}

void SharedState::loadUser(const std::string &username) {
    SaveManager sm(dataDir_ / "saves");
    User user = sm.load(username);

    std::lock_guard lock(mapMutex_);
    allUsers_.emplace(username, std::move(user));
    userMutexes_.emplace(username, std::make_unique<std::shared_mutex>());
    // One IncomeEngine per user so the coin accumulator persists between ticks.
    incomeEngines_.emplace(username, std::make_unique<IncomeEngine>(catalog_));
}

void SharedState::saveUser(const std::string &username) {
    SaveManager sm(dataDir_ / "saves");
    std::shared_lock lock(*userMutexes_.at(username));
    sm.save(allUsers_.at(username));
}

// Called by the server tick thread every second
// Lock order: mapMutex_ first, then per-user mutex
void SharedState::tickAllOnlineUsers() {
    std::lock_guard mapLock(mapMutex_);
    for (auto &[username, user]: allUsers_) {
        std::unique_lock userLock(*userMutexes_.at(username));
        incomeEngines_.at(username)->tick(user);
    }
}

const std::vector<PlantSpecies> &SharedState::getShopListings() const { return shopListings_; }
const std::vector<PlantSpecies> &SharedState::getCatalog() const { return catalog_; }
