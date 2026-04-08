#pragma once

#include "core/PlantSpecies.h"
#include <filesystem>
#include <vector>

class CatalogLoader {
public:
    static std::vector<PlantSpecies> load(const std::filesystem::path& path);
};
