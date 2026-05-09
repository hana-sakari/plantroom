#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "core/PlantSpecies.h"
#include "core/User.h"

namespace Cmd {
    inline constexpr const char *LOGIN_PASS = "LOGIN_PASS";
    inline constexpr const char *LOGIN_KEY = "LOGIN_KEY";
    inline constexpr const char *REGISTER = "REGISTER";
    inline constexpr const char *GET_STATE = "GET_STATE";
    inline constexpr const char *BUY = "BUY";
    inline constexpr const char *PLACE = "PLACE";
    inline constexpr const char *REMOVE = "REMOVE";
    inline constexpr const char *SAVE = "SAVE";
    inline constexpr const char *SHOP_LIST = "SHOP_LIST";
    inline constexpr const char *ADD_KEY  = "ADD_KEY";
}

namespace Protocol {
    // Response builders
    nlohmann::json okResponse();

    nlohmann::json errorResponse(const std::string &message);

    nlohmann::json stateResponse(const User &user);

    nlohmann::json shopListResponse(const std::vector<PlantSpecies> &listings);

    // One JSON object per line over a file descriptor. Returns false on EOF/error
    bool readLine(int fd, std::string &out);

    bool writeLine(int fd, const nlohmann::json &msg);
}
