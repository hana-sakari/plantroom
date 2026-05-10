#pragma once
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/IncomeEngine.h"
#include "core/MarketListing.h"
#include "core/PlantSpecies.h"
#include "core/User.h"

class SharedState {
public:
    // Loads catalog from dataDir/plants.json and shop listings from dataDir/shop.json
    // loadUser/saveUser use dataDir/saves/<username>.json
    explicit SharedState(const std::filesystem::path &dataDir);

    // Hold the returned lock for as long as you are reading user data
    std::shared_lock<std::shared_mutex> lockUserShared(const std::string &username);

    // must hold a shared lock from lockUserShared before calling
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

    // Returns nullptr if no species with that id exists
    const PlantSpecies *findSpeciesById(uint32_t speciesId) const;

    std::vector<MarketListing> getMarketListings() const;

    // Returns error string, or "" on success
    std::string postToMarket(const std::string &seller, uint32_t inventoryIdx, uint32_t price);

    std::string buyFromMarket(const std::string &buyer, uint32_t listingId);

    std::string removeFromMarket(const std::string &seller, uint32_t listingId);

private:
    std::filesystem::path dataDir_;
    std::vector<PlantSpecies> catalog_; // all species, loaded from plants.json
    std::vector<PlantSpecies> shopListings_; // current month's subset, from shop.json

    std::unordered_map<std::string, User> allUsers_;
    std::unordered_map<std::string, std::unique_ptr<std::shared_mutex> > userMutexes_;
    std::unordered_map<std::string, std::unique_ptr<IncomeEngine> > incomeEngines_;
    std::mutex mapMutex_; // protects allUsers_, userMutexes_, and incomeEngines_
    void loadMarket();

    void saveMarket() const; // must be called with marketMutex_ held

    mutable std::mutex marketMutex_; // protects market_ and nextListingId_
    std::vector<MarketListing> market_;
    uint32_t nextListingId_ = 1;
};


template<typename Fn>
void SharedState::modifyUser(const std::string &username, Fn fn) {
    std::unique_lock lock(*userMutexes_.at(username));
    fn(allUsers_.at(username));
}
