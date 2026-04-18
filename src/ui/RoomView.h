#pragma once

#include <ftxui/component/component.hpp>
#include "core/User.h"

ftxui::Component MakeRoomView(
    const User &user,
    std::function<void(uint8_t slot, uint32_t plantId)> onPlace
);
