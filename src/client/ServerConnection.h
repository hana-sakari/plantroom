#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "core/MarketListing.h"
#include "core/PlantSpecies.h"
#include "core/User.h"

struct ActionResult {
    bool ok;
    std::string error;
};

class ServerConnection {
public:
    explicit ServerConnection(const std::string &socketPath);

    ~ServerConnection(); //closes fd
    ActionResult loginPass(const std::string &username, const std::string &password);

    ActionResult loginKey(const std::string &username);

    ActionResult registerUser(const std::string &username, const std::string &password);

    User getState();

    std::vector<PlantSpecies> getShopList();

    ActionResult buy(uint32_t speciesId);

    ActionResult place(uint8_t slot, uint32_t inventoryIdx);

    ActionResult remove(uint8_t slot);

    ActionResult save();

    ActionResult addPublicKey(const std::string &pubkey);

    std::vector<MarketListing> getMarketListings();

    ActionResult postToMarket(uint32_t inventoryIdx, uint32_t price);

    ActionResult buyFromMarket(uint32_t listingId);

    ActionResult removeFromMarket(uint32_t listingId);

private:
    int fd_;

    nlohmann::json exchange(const nlohmann::json &req);
};
