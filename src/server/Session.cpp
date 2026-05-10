#include "server/Session.h"
#include "server/Protocol.h"
#include "core/Plant.h"
#include <nlohmann/json.hpp>

Session::Session(int fd, SharedState &sharedState, AuthStore &authStore) : fd(fd), sharedState(sharedState),
    authStore(authStore) {
}

void Session::run() {
    std::string line;

    while (true) {
        if (!Protocol::readLine(fd, line))
            break;

        try {
            auto msg = nlohmann::json::parse(line);
            std::string cmd = msg["cmd"];

            // reject game commands until logged in
            bool isAuthCmd = cmd == Cmd::LOGIN_PASS
                    || cmd == Cmd::LOGIN_KEY
                    || cmd == Cmd::REGISTER;
            if (username.empty() && !isAuthCmd) {
                Protocol::writeLine(fd, Protocol::errorResponse("not logged in"));
                continue;
            }

            if (cmd == Cmd::LOGIN_PASS) {
                std::string user = msg["username"];
                std::string pass = msg["password"];
                if (!authStore.verifyPassword(user, pass)) {
                    Protocol::writeLine(fd, Protocol::errorResponse("invalid credentials"));
                } else {
                    username = user;
                    sharedState.loadUser(username);
                    Protocol::writeLine(fd, Protocol::okResponse());
                }
            } else if (cmd == Cmd::LOGIN_KEY) {
                // OpenSSH already verified the key
                std::string user = msg["username"];
                if (!authStore.userExists(user)) {
                    Protocol::writeLine(fd, Protocol::errorResponse("user not found"));
                } else {
                    username = user;
                    sharedState.loadUser(username);
                    Protocol::writeLine(fd, Protocol::okResponse());
                }
            } else if (cmd == Cmd::REGISTER) {
                std::string user = msg["username"];
                std::string pass = msg["password"];
                if (!authStore.registerUser(user, pass)) {
                    Protocol::writeLine(fd, Protocol::errorResponse("username taken"));
                } else {
                    username = user;
                    sharedState.loadUser(username);
                    Protocol::writeLine(fd, Protocol::okResponse());
                }
            } else if (cmd == Cmd::GET_STATE) {
                auto lock = sharedState.lockUserShared(username);
                User *user = sharedState.getUser(username);
                Protocol::writeLine(fd, Protocol::stateResponse(*user));
            } else if (cmd == Cmd::SHOP_LIST) {
                Protocol::writeLine(fd, Protocol::shopListResponse(sharedState.getShopListings()));
            } else if (cmd == Cmd::BUY) {
                uint32_t speciesId = msg["speciesId"];
                bool ok = false;
                std::string errMsg;
                sharedState.modifyUser(username, [&](User &user) {
                    const PlantSpecies *species = sharedState.findSpeciesById(speciesId);
                    if (!species) {
                        errMsg = "unknown species";
                        return;
                    }
                    if (user.coins < species->price) {
                        errMsg = "not enough coins";
                        return;
                    }
                    user.coins -= species->price;
                    user.plantsInventory.push_back(Plant{speciesId});
                    ok = true;
                });
                if (ok)
                    Protocol::writeLine(fd, Protocol::okResponse());
                else
                    Protocol::writeLine(fd, Protocol::errorResponse(errMsg));
            } else if (cmd == Cmd::PLACE) {
                uint32_t slot = msg["slot"];
                uint32_t inventoryIdx = msg["inventoryIdx"];
                bool ok = false;
                std::string errMsg;
                sharedState.modifyUser(username, [&](User &user) {
                    if (inventoryIdx >= user.plantsInventory.size()) {
                        errMsg = "not enough inventory plants";
                        return;
                    }
                    if (slot >= SLOT_COUNT) {
                        errMsg = "invalid slot index";
                        return;
                    }
                    user.slots_[slot] = user.plantsInventory[inventoryIdx];
                    user.plantsInventory.erase(user.plantsInventory.begin() + inventoryIdx);
                    ok = true;
                });
                if (ok)
                    Protocol::writeLine(fd, Protocol::okResponse());
                else {
                    Protocol::writeLine(fd, Protocol::errorResponse(errMsg));
                }
            } else if (cmd == Cmd::REMOVE) {
                uint32_t slot = msg["slot"];
                bool ok = false;
                std::string errMsg;
                sharedState.modifyUser(username, [&](User &user) {
                    if (slot >= SLOT_COUNT) {
                        errMsg = "invalid slot index";
                        return;
                    }
                    if (!user.slots_[slot].has_value()) {
                        errMsg = "slot is already empty";
                        return;
                    }
                    user.plantsInventory.push_back(user.slots_[slot].value());
                    user.slots_[slot].reset();
                    ok = true;
                });
                if (ok)
                    Protocol::writeLine(fd, Protocol::okResponse());
                else
                    Protocol::writeLine(fd, Protocol::errorResponse(errMsg));
            } else if (cmd == Cmd::SAVE) {
                sharedState.saveUser(username);
                Protocol::writeLine(fd, Protocol::okResponse());
            } else if (cmd == Cmd::ADD_KEY) {
                std::string pubkey = msg["publicKey"];
                authStore.addPublicKey(username, pubkey);
                Protocol::writeLine(fd, Protocol::okResponse());
            } else if (cmd == Cmd::MARKET_LIST) {
                Protocol::writeLine(fd, Protocol::marketListResponse(sharedState.getMarketListings()));
            } else if (cmd == Cmd::MARKET_POST) {
                uint32_t inventoryIdx = msg["inventoryIdx"];
                uint32_t price = msg["price"];
                std::string err = sharedState.postToMarket(username, inventoryIdx, price);
                if (err.empty())
                    Protocol::writeLine(fd, Protocol::okResponse());
                else
                    Protocol::writeLine(fd, Protocol::errorResponse(err));
            } else if (cmd == Cmd::MARKET_BUY) {
                uint32_t listingId = msg["listingId"];
                std::string err = sharedState.buyFromMarket(username, listingId);
                if (err.empty())
                    Protocol::writeLine(fd, Protocol::okResponse());
                else
                    Protocol::writeLine(fd, Protocol::errorResponse(err));
            } else if (cmd == Cmd::MARKET_REMOVE) {
                uint32_t listingId = msg["listingId"];
                std::string err = sharedState.removeFromMarket(username, listingId);
                if (err.empty())
                    Protocol::writeLine(fd, Protocol::okResponse());
                else
                    Protocol::writeLine(fd, Protocol::errorResponse(err));
            } else {
                Protocol::writeLine(fd, Protocol::errorResponse("unknown command"));
            }
        } catch (const std::exception &e) {
            Protocol::writeLine(fd, Protocol::errorResponse(e.what()));
        }
    }

    // client disconnected, save their data
    if (!username.empty())
        sharedState.saveUser(username);
}
