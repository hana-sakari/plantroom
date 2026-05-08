#include "client/ServerConnection.h"
#include "server/Protocol.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>

ServerConnection::ServerConnection(const std::string &socketPath) {
    fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0)
        throw std::runtime_error("socket() failed");

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    socketPath.copy(addr.sun_path, sizeof(addr.sun_path) - 1);

    if (connect(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        close(fd_);
        throw std::runtime_error("connect() failed — is plantroom-server running?");
    }
}

ServerConnection::~ServerConnection() {
    close(fd_);
}

nlohmann::json ServerConnection::exchange(const nlohmann::json &req) {
    if (!Protocol::writeLine(fd_, req))
        throw std::runtime_error("connection lost while sending");
    std::string line;
    if (!Protocol::readLine(fd_, line))
        throw std::runtime_error("connection lost while reading");
    return nlohmann::json::parse(line);
}

ActionResult ServerConnection::loginPass(const std::string &username, const std::string &password) {
    nlohmann::json req = {
        {"cmd", Cmd::LOGIN_PASS},
        {"username", username},
        {"password", password}
    };

    auto resp = exchange(req);
    if (resp["status"] == "ok") {
        return ActionResult{true};
    }

    return ActionResult{false, resp["message"]};
}

ActionResult ServerConnection::loginKey(const std::string &username) {
    nlohmann::json req = {
        {"cmd", Cmd::LOGIN_KEY},
        {"username", username}
    };

    auto resp = exchange(req);
    if (resp["status"] == "ok") {
        return ActionResult{true};
    }
    return ActionResult{false, resp["message"]};

}

ActionResult ServerConnection::registerUser(const std::string &username, const std::string &password) {
    nlohmann::json req = {
        {"cmd", Cmd::REGISTER},
        {"username", username},
        {"password", password}
    };

    auto resp = exchange(req);
    if (resp["status"] == "ok") {
        return ActionResult{true};
    }
    return ActionResult{false, resp["message"]};

}

User ServerConnection::getState() {
    auto resp = exchange({{"cmd", Cmd::GET_STATE}});
    if (resp["status"] != "ok")
        throw std::runtime_error(resp.value("message", "server error"));

    User user;
    user.id = resp["id"];
    user.name = resp["name"];
    user.coins = resp["coins"];
    user.lastSavedAt = resp["lastSavedAt"];
    user.plantsInventory = resp["plantsInventory"].get<std::vector<Plant>>();
    for (int i = 0; i < SLOT_COUNT; i++) {
        if (resp["slots"][i].is_null())
            user.slots_[i] = std::nullopt;
        else
            user.slots_[i] = resp["slots"][i].get<Plant>();
    }
    return user;
}

std::vector<PlantSpecies> ServerConnection::getShopList() {
    auto resp = exchange({{"cmd", Cmd::SHOP_LIST}});
    if (resp["status"] != "ok")
        throw std::runtime_error(resp.value("message", "server error"));
    return resp["listings"].get<std::vector<PlantSpecies>>();
}

ActionResult ServerConnection::buy(uint32_t speciesId) {
    auto resp = exchange({{"cmd", Cmd::BUY}, {"speciesId", speciesId}});
    if (resp["status"] == "ok")
        return {true, ""};
    return {false, resp.value("message", "unknown error")};
}

ActionResult ServerConnection::place(uint8_t slot, uint32_t inventoryIdx) {
    auto resp = exchange({
        {"cmd",          Cmd::PLACE},
        {"slot",         slot},
        {"inventoryIdx", inventoryIdx}
    });
    if (resp["status"] == "ok")
        return {true, ""};
    return {false, resp.value("message", "unknown error")};
}

ActionResult ServerConnection::remove(uint8_t slot) {
    auto resp = exchange({{"cmd", Cmd::REMOVE}, {"slot", slot}});
    if (resp["status"] == "ok")
        return {true, ""};
    return {false, resp.value("message", "unknown error")};
}

ActionResult ServerConnection::save() {
    auto resp = exchange({{"cmd", Cmd::SAVE}});
    if (resp["status"] == "ok")
        return {true, ""};
    return {false, resp.value("message", "unknown error")};
}
