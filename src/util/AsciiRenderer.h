#pragma once

#include <string>
#include <vector>

class AsciiRenderer {
public:
    static std::string render(const std::string &roomTemplate,
        const std::vector<std::string> &slotContents);
};
