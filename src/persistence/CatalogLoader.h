#pragma once

#include "core/PlantDef.h"
#include <filesystem>
#include <vector>

class CatalogLoader {
public:
    static std::vector<PlantDef> load(const std::filesystem::path& path);
};
