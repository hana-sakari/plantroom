#pragma once
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/IncomeEngine.h"
#include "core/PlantSpecies.h"
#include "core/User.h"

class SharedState {
public:
    // Loads catalog from dataDir/plants.json and shop listings from dataDir/shop.json
    // loadUser/saveUser use dataDir/saves/<username>.json
    explicit SharedState(const std::filesystem::path &dataDir);


    // must hold a shared lock on the user's mutex before calling
    User *getUser(const std::string &username);

    // Acquires exclusive lock on the user, then calls fn(user)
    template<typename Fn>
    void modifyUser(const std::string &username, Fn fn);

    void loadUser(const std::string &username);

    void saveUser(const std::string &username);

    // Called by the tick thread every second
    void tickAllOnlineUsers();

    const std::vector<PlantSpecies> &getShopListings() const;

    const std::vector<PlantSpecies> &getCatalog() const;

private:
    std::filesystem::path dataDir_;
    std::vector<PlantSpecies> catalog_; // all species, loaded from plants.json
    std::vector<PlantSpecies> shopListings_; // current month's subset, from shop.json

    std::unordered_map<std::string, User> allUsers_;
    std::unordered_map<std::string, std::unique_ptr<std::shared_mutex> > userMutexes_;
    std::unordered_map<std::string, std::unique_ptr<IncomeEngine> > incomeEngines_;
    std::mutex mapMutex_; // protects allUsers_, userMutexes_, and incomeEngines_
    std::mutex marketMutex_;
};


template<typename Fn>
void SharedState::modifyUser(const std::string &username, Fn fn) {
    std::unique_lock lock(*userMutexes_.at(username));
    fn(allUsers_.at(username));
}
