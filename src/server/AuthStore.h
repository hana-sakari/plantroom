#pragma once
#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>

class AuthStore {
public:
    explicit AuthStore(const std::filesystem::path &authFile);

    bool registerUser(const std::string &username, const std::string &password);

    bool verifyPassword(const std::string &username, const std::string &password);

    bool userExists(const std::string &username) const;

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
    };

    std::unordered_map<std::string, Credentials> store_;
};
