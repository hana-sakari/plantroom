#include "server/SharedState.h"
#include "persistence/CatalogLoader.h"
#include "persistence/SaveManager.h"
#include <algorithm>
#include <ctime>
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
    loadMarket();
}

std::shared_lock<std::shared_mutex> SharedState::lockUserShared(const std::string &username) {
    return std::shared_lock<std::shared_mutex>(*userMutexes_.at(username));
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
    User user;
    try {
        user = sm.load(username);
    } catch (const std::exception &) {
        // no save file yet, new user
        user = User{0, username, 100, 0, {}, {}};
    }

    if (user.lastSavedAt > 0) {
        IncomeEngine engine(catalog_);
        user.coins += engine.calculateOfflineIncome(user, user.lastSavedAt, std::time(nullptr));
    }

    std::lock_guard lock(mapMutex_);
    allUsers_.emplace(username, std::move(user));
    userMutexes_.emplace(username, std::make_unique<std::shared_mutex>());
    // One IncomeEngine per user so the coin accumulator persists between ticks.
    incomeEngines_.emplace(username, std::make_unique<IncomeEngine>(catalog_));
}

void SharedState::saveUser(const std::string &username) {
    SaveManager sm(dataDir_ / "saves");
    std::unique_lock lock(*userMutexes_.at(username));
    allUsers_.at(username).lastSavedAt = std::time(nullptr);
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

void SharedState::loadMarket() {
    auto path = dataDir_ / "market.json";
    if (!std::filesystem::exists(path)) return;
    std::ifstream f(path);
    auto j = nlohmann::json::parse(f);
    market_ = j.get<std::vector<MarketListing> >();
    for (const auto &l: market_)
        if (l.id >= nextListingId_) nextListingId_ = l.id + 1;
}

void SharedState::saveMarket() const {
    std::ofstream f(dataDir_ / "market.json");
    f << nlohmann::json(market_).dump(2);
}

std::vector<MarketListing> SharedState::getMarketListings() const {
    std::lock_guard lock(marketMutex_);
    return market_;
}

std::string SharedState::postToMarket(const std::string &seller, uint32_t inventoryIdx, uint32_t price) {
    Plant plant;
    {
        std::unique_lock lock(*userMutexes_.at(seller));
        auto &user = allUsers_.at(seller);
        if (inventoryIdx >= user.plantsInventory.size())
            return "invalid inventory index";
        plant = user.plantsInventory[inventoryIdx];
        user.plantsInventory.erase(user.plantsInventory.begin() + inventoryIdx);
    }
    std::lock_guard lock(marketMutex_);
    market_.push_back({nextListingId_++, seller, plant, price});
    saveMarket();
    return "";
}

std::string SharedState::buyFromMarket(const std::string &buyer, uint32_t listingId) {
    MarketListing listing;
    {
        std::lock_guard lock(marketMutex_);
        auto it = std::find_if(market_.begin(), market_.end(),
            [listingId](const MarketListing &l) { return l.id == listingId; });
        if (it == market_.end()) return "listing not found";
        if (it->seller == buyer) return "cannot buy your own listing";
        listing = *it;
    }

    {
        std::unique_lock lock(*userMutexes_.at(buyer));
        auto &user = allUsers_.at(buyer);
        if (user.coins < listing.price) return "not enough coins";
        user.coins -= listing.price;
        user.plantsInventory.push_back(listing.plant);
    }

    {
        std::lock_guard lock(marketMutex_);
        auto it = std::find_if(market_.begin(), market_.end(),
            [listingId](const MarketListing &l) { return l.id == listingId; });
        if (it == market_.end()) {
            // already bought by someone else — refund
            std::unique_lock lock(*userMutexes_.at(buyer));
            auto &user = allUsers_.at(buyer);
            user.coins += listing.price;
            user.plantsInventory.pop_back();
            return "listing already sold";
        }
        market_.erase(it);
        saveMarket();
    }

    {
        std::lock_guard mapLock(mapMutex_);
        auto it = allUsers_.find(listing.seller);
        if (it != allUsers_.end()) {
            std::unique_lock lock(*userMutexes_.at(listing.seller));
            it->second.coins += listing.price;
        } else {
            // seller offline — credit their save file
            SaveManager sm(dataDir_ / "saves");
            try {
                User seller = sm.load(listing.seller);
                seller.coins += listing.price;
                sm.save(seller);
            } catch (...) {
            }
        }
    }

    return "";
}

std::string SharedState::removeFromMarket(const std::string &seller, uint32_t listingId) {
    Plant plant;
    {
        std::lock_guard lock(marketMutex_);
        auto it = std::find_if(market_.begin(), market_.end(),
            [listingId](const MarketListing &l) { return l.id == listingId; });
        if (it == market_.end()) return "listing not found";
        if (it->seller != seller) return "not your listing";
        plant = it->plant;
        market_.erase(it);
        saveMarket();
    }
    std::unique_lock lock(*userMutexes_.at(seller));
    allUsers_.at(seller).plantsInventory.push_back(plant);
    return "";
}

const std::vector<PlantSpecies> &SharedState::getShopListings() const { return shopListings_; }
const std::vector<PlantSpecies> &SharedState::getCatalog() const { return catalog_; }

const PlantSpecies *SharedState::findSpeciesById(uint32_t speciesId) const {
    auto it = std::ranges::find_if(catalog_,
        [speciesId](const PlantSpecies &s) { return s.id == speciesId; });
    return it != catalog_.end() ? &*it : nullptr;
}
