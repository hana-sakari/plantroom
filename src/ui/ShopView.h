#pragma once

#include <ftxui/component/component.hpp>
#include "core/PlantSpecies.h"
#include <vector>

ftxui::Component MakeShopView(const std::vector<PlantSpecies>& catalog);
