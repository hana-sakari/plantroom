#pragma once

#include "core/User.h"
#include <filesystem>
#include <string>


class SaveManager {
public:
    explicit SaveManager(std::filesystem::path saveDir);

    void save(const User &user) const;

    User load(const std::string &name) const;

private:
    std::filesystem::path saveDir_;
};
