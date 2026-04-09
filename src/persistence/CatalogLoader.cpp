#include "persistence/CatalogLoader.h"
#include <fstream>

using json = nlohmann::json;

auto CatalogLoader::load(const std::filesystem::path &path) -> std::vector<PlantSpecies> {
    std::ifstream ifs(path);
    json j;
    ifs >> j;
    auto species = j.get<std::vector<PlantSpecies>>();
    return species;
}
