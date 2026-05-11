#pragma once

#include <ftxui/component/component.hpp>
#include "core/User.h"
#include "core/PlantSpecies.h"
#include <vector>

ftxui::Component MakeInventoryView(const User &user, const std::vector<PlantSpecies> &catalog);
