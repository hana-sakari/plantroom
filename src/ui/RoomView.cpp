#include "ui/RoomView.h"
#include <ftxui/component/component.hpp>
#include <functional>

#include "ftxui/component/event.hpp"

using namespace ftxui;

auto MakeRoomView(const User &user, std::function<void(uint8_t slot, uint32_t plantId)> onPlace) -> ftxui::Component {
    uint8_t selectedSlot = 0;
    auto renderer = Renderer([&] {
        std::vector<Element> slots;
        for (int i = 0; i < 6; i++) {
            std::string content = user.slots_[i] ? user.slots_[i]->speciesId : "[empty]";

            Element slot = text(content) | border;
            if (i == selectedSlot) {
                slot = slot | color(Color::Yellow) | bold;
            }
            slots.push_back(slot);
        }

        return vbox({
            hbox({slots[0], separator(), slots[1], separator(), slots[2]}),
            separator(),
            hbox({slots[3], separator(), slots[4], separator(), slots[5]}),
        }) | border;
    });


    return CatchEvent(renderer, [&](Event e) {
        if (e == Event::ArrowRight) selectedSlot = std::min(SLOT_COUNT-1 , selectedSlot + 1);
        if (e == Event::ArrowLeft) selectedSlot = std::max(0, selectedSlot - 1);
        if (e == Event::Return) {
            onPlace(selectedSlot, 123); // TODO: get plantId from inventory
        }
        return false;
    });
}
