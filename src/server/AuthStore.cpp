#include "server/AuthStore.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/rand.h>

static std::string toHex(const uint8_t *data, size_t len) {
    std::string out;
    out.reserve(len * 2);
    char buf[3];
    for (size_t i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), "%02x", data[i]);
        out += buf;
    }
    return out;
}

AuthStore::AuthStore(const std::filesystem::path &authFile) : authFile_(authFile) {
    load();
}

bool AuthStore::userExists(const std::string &username) const {
    return store_.contains(username);
}

// generates 16 cryptographically random bytes, returns them as a 32-char hex string
std::string AuthStore::generateSalt() const {
    uint8_t bytes[16];
    RAND_bytes(bytes, sizeof(bytes));
    return toHex(bytes, sizeof(bytes));
}

// SHA-256(password + salt), returned as a 64-char hex string
std::string AuthStore::hashPassword(const std::string &password, const std::string &salt) const {
    std::string input = password + salt;
    uint8_t hash[32];
    EVP_Digest(input.data(), input.size(), hash, nullptr, EVP_sha256(), nullptr);
    return toHex(hash, sizeof(hash));
}

void AuthStore::load() {
    if (!std::filesystem::exists(authFile_))
        return;

    std::ifstream f(authFile_);
    auto j = nlohmann::json::parse(f);

    for (auto &[username, entry]: j.items()) {
        Credentials c;
        c.passwordHash = entry["passwordHash"];
        c.salt = entry["salt"];
        if (entry.contains("publicKeys"))
            c.publicKeys = entry["publicKeys"].get<std::vector<std::string> >();
        store_[username] = std::move(c);
    }
}

void AuthStore::save() const {
    nlohmann::json j;
    for (auto &[username, creds]: store_) {
        j[username]["passwordHash"] = creds.passwordHash;
        j[username]["salt"] = creds.salt;
        j[username]["publicKeys"] = creds.publicKeys;
    }
    std::ofstream f(authFile_);
    f << j.dump(2);
}

bool AuthStore::addPublicKey(const std::string &username, const std::string &pubkey) {
    std::lock_guard lock(mutex_);
    if (!userExists(username))
        return false;
    auto &keys = store_.at(username).publicKeys;
    if (std::find(keys.begin(), keys.end(), pubkey) == keys.end())
        keys.push_back(pubkey);
    save();
    return true;
}


bool AuthStore::registerUser(const std::string &username, const std::string &password) {
    std::lock_guard lock(mutex_);

    if (userExists(username))
        return false;

    const auto salt = generateSalt();
    const auto hash = hashPassword(password, salt);

    store_[username] = Credentials{hash, salt};
    save();
    return true;
}


bool AuthStore::verifyPassword(const std::string &username, const std::string &password) {
    std::lock_guard lock(mutex_);

    if (!userExists(username))
        return false;

    const auto &creds = store_.at(username);
    return hashPassword(password, creds.salt) == creds.passwordHash;
}
