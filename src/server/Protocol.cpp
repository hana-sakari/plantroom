#include "server/Protocol.h"
#include <unistd.h>

namespace Protocol {
    // Returns false if the connection closed or error occurred
    bool readLine(int fd, std::string &out) {
        out.clear();
        char c;
        while (true) {
            ssize_t n = read(fd, &c, 1);
            if (n <= 0) return false;
            if (c == '\n') return true;
            out += c;
        }
    }


    // Returns false on error
    bool writeLine(int fd, const nlohmann::json &msg) {
        std::string s = msg.dump() + "\n";
        auto total = static_cast<ssize_t>(s.size());
        ssize_t sent = 0;
        while (sent < total) {
            ssize_t n = write(fd, s.data() + sent, total - sent);
            if (n <= 0) return false;
            sent += n;
        }
        return true;
    }

    nlohmann::json okResponse() {
        return {{"status", "ok"}};
    }

    nlohmann::json errorResponse(const std::string &message) {
        return {{"status", "error"}, {"message", message}};
    }

    nlohmann::json stateResponse(const User &user) {
        nlohmann::json j;
        j["status"] = "ok";
        j["id"] = user.id;
        j["name"] = user.name;
        j["coins"] = user.coins;
        j["lastSavedAt"] = user.lastSavedAt;
        j["plantsInventory"] = user.plantsInventory;

        nlohmann::json slots = nlohmann::json::array();
        for (const auto &slot: user.slots_)
            slots.push_back(slot.has_value() ? nlohmann::json(*slot) : nlohmann::json(nullptr));
        j["slots"] = slots;

        return j;
    }

    nlohmann::json shopListResponse(const std::vector<PlantSpecies> &listings) {
        return {{"status", "ok"}, {"listings", listings}};
    }

    nlohmann::json catalogResponse(const std::vector<PlantSpecies> &catalog) {
        return {{"status", "ok"}, {"catalog", catalog}};
    }

    nlohmann::json marketListResponse(const std::vector<MarketListing> &listings) {
        return {{"status", "ok"}, {"listings", listings}};
    }
} // namespace Protocol
