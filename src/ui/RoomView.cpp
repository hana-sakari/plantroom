#include "ui/RoomView.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

using namespace ftxui;

Component MakeRoomView(const User& user, std::function<void(uint8_t slot, uint32_t plantId)> onPlace) {
    auto selected = std::make_shared<int>(0);

    auto renderer = Renderer([&user, selected] {
        std::vector<Element> slots;
        for (int i = 0; i < SLOT_COUNT; i++) {
            std::string content = user.slots_[i] ? user.slots_[i]->speciesId : "[empty]";
            Element slot = text(content) | border;
            if (i == *selected) slot = slot | color(Color::Yellow) | bold;
            slots.push_back(slot);
        }
        return vbox({
            hbox({slots[0], separator(), slots[1], separator(), slots[2]}),
            separator(),
            hbox({slots[3], separator(), slots[4], separator(), slots[5]}),
        }) | border | flex;
    });

    return CatchEvent(renderer, [selected, onPlace](Event e) {
        if (e == Event::ArrowRight) {
            *selected = std::min(SLOT_COUNT - 1, *selected + 1);
            return true;
        }
        if (e == Event::ArrowLeft) {
            *selected = std::max(0, *selected - 1);
            return true;
        }
        if (e == Event::Return) {
            onPlace((uint8_t)*selected, 123); // TODO: pick plantId from inventory
            return true;
        }
        return false;
    });
}
