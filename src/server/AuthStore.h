#pragma once
#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class AuthStore {
public:
    explicit AuthStore(const std::filesystem::path &authFile);

    bool registerUser(const std::string &username, const std::string &password);

    bool verifyPassword(const std::string &username, const std::string &password);

    bool userExists(const std::string &username) const;

    // stores pubkey under the user, no-ops on duplicate
    bool addPublicKey(const std::string &username, const std::string &pubkey);

private:
    std::string generateSalt() const;

    std::string hashPassword(const std::string &password, const std::string &salt) const;

    void load();

    void save() const;

    std::filesystem::path authFile_;
    std::mutex mutex_;

    struct Credentials {
        std::string passwordHash;
        std::string salt;
        std::vector<std::string> publicKeys;
    };

    std::unordered_map<std::string, Credentials> store_;
};
