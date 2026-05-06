#include "server/Session.h"
#include "server/Protocol.h"
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
                sharedState.modifyUser(username, [&](User &user) {
                    // TODO: find species, check coins, add plant to inventory
                });
                Protocol::writeLine(fd, Protocol::okResponse());
            } else if (cmd == Cmd::PLACE) {
                uint32_t slot = msg["slot"];
                uint32_t inventoryIdx = msg["inventoryIdx"];
                sharedState.modifyUser(username, [&](User &user) {
                    // TODO: move plant from inventory to slot
                });
                Protocol::writeLine(fd, Protocol::okResponse());
            } else if (cmd == Cmd::REMOVE) {
                uint32_t slot = msg["slot"];
                sharedState.modifyUser(username, [&](User &user) {
                    // TODO: move plant from slot back to inventory
                });
                Protocol::writeLine(fd, Protocol::okResponse());
            } else if (cmd == Cmd::SAVE) {
                sharedState.saveUser(username);
                Protocol::writeLine(fd, Protocol::okResponse());
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
