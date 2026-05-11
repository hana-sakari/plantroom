#include "ui/RoomView.h"
#include "util/PlantUtils.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

using namespace ftxui;
using namespace PlantUtils;

namespace {
    constexpr int PICKER_VISIBLE = 5;
    constexpr int PICKER_CARD_W = 14;

    Element makePickerCard(const Plant &plant, const std::vector<PlantSpecies> &catalog, bool selected) {
        std::string name = std::to_string(plant.speciesId);
        std::string art = "  [ ]  ";
        if (const auto *sp = findSpecies(catalog, plant.speciesId)) {
            name = sp->name;
            int idx = static_cast<int>(plant.growthStage);
            if (idx < (int) sp->asciiArt.size()) art = sp->asciiArt[idx];
        }

        auto card = vbox({
            text(art) | center,
            separator(),
            text(name) | center | bold,
            text(stageName(plant.growthStage)) | center,
        }) | border | size(WIDTH, EQUAL, PICKER_CARD_W);

        if (selected) card = card | color(Color::Green);
        return card;
    }
} // namespace

Component MakeRoomView(
    const User &user,
    const std::vector<PlantSpecies> &catalog,
    std::function<void(uint8_t slot, int inventoryIdx)> onPlace,
    std::function<void(uint8_t slot)> onRemove
) {
    struct RoomState {
        int selectedSlot = 0;
        bool pickingMode = false;
        int selectedInventory = 0;
    };
    auto state = std::make_shared<RoomState>();

    auto renderer = Renderer([&user, &catalog, state] {
        //slot grid
        std::vector<Element> slots;
        for (int i = 0; i < SLOT_COUNT; i++) {
            Element slot;
            if (user.slots_[i].has_value()) {
                const auto &plant = *user.slots_[i];
                const auto *sp = findSpecies(catalog, plant.speciesId);
                std::string art = "  [ ]  ";
                if (sp) {
                    int idx = static_cast<int>(plant.growthStage);
                    if (idx < (int) sp->asciiArt.size()) art = sp->asciiArt[idx];
                }
                slot = text(art) | center | border | size(WIDTH, EQUAL, 14);
            } else {
                slot = text("") | border | size(WIDTH, EQUAL, 14);
            }
            if (i == state->selectedSlot)
                slot = slot | (state->pickingMode ? color(Color::Cyan) : color(Color::Yellow)) | bold;
            slots.push_back(slot);
        }

        auto grid = vbox({
            hbox({slots[0], separator(), slots[1], separator(), slots[2]}),
            separator(),
            hbox({slots[3], separator(), slots[4], separator(), slots[5]}),
        });

        //picker panel
        if (state->pickingMode) {
            int n = (int) user.plantsInventory.size();

            Element pickerBody;
            if (n == 0) {
                pickerBody = text("Inventory is empty — buy plants in the Shop first.") | center | color(
                    Color::GrayDark);
            } else {
                int sel = state->selectedInventory;
                int start = std::max(0, sel - PICKER_VISIBLE / 2);
                if (start + PICKER_VISIBLE > n) start = std::max(0, n - PICKER_VISIBLE);
                int end = std::min(n, start + PICKER_VISIBLE);

                std::vector<Element> cards;
                for (int i = start; i < end; i++)
                    cards.push_back(makePickerCard(user.plantsInventory[i], catalog, i == sel));

                pickerBody = hbox({
                    text(start > 0 ? " < " : "   ") | color(Color::GrayDark),
                    hbox(std::move(cards)),
                    text(end < n ? " > " : "   ") | color(Color::GrayDark),
                }) | hcenter;
            }

            auto picker = vbox({
                text("Place in slot " + std::to_string(state->selectedSlot + 1) + " — choose a plant:") | bold | center,
                separator(),
                pickerBody,
                separator(),
                hbox({
                    text(" Enter: confirm ") | color(Color::Green),
                    separator(),
                    text(" Esc: cancel ") | color(Color::GrayDark),
                }) | hcenter,
            }) | border;

            return vbox({grid, separator(), picker}) | border | flex;
        }

        // normal mode hint
        return vbox({
            grid,
            separator(),
            text(" Enter: place / remove    ← →: navigate ") | hcenter | color(Color::GrayDark),
        }) | border | flex;
    });


    return CatchEvent(renderer, [&user, state, onPlace, onRemove](Event e) {
        if (state->pickingMode) {
            int n = static_cast<int>(user.plantsInventory.size());
            if (e == Event::ArrowRight) {
                if (n > 0) state->selectedInventory = std::min(n - 1, state->selectedInventory + 1);
                return true;
            }
            if (e == Event::ArrowLeft) {
                state->selectedInventory = std::max(0, state->selectedInventory - 1);
                return true;
            }
            if (e == Event::Return) {
                if (n > 0) onPlace(static_cast<uint8_t>(state->selectedSlot), state->selectedInventory);
                state->pickingMode = false;
                return true;
            }
            if (e == Event::Escape) {
                state->pickingMode = false;
                return true;
            }
            return true; // ignore everything else while picker is open
        }

        if (e == Event::ArrowRight) {
            state->selectedSlot = std::min(SLOT_COUNT - 1, state->selectedSlot + 1);
            return true;
        }
        if (e == Event::ArrowLeft) {
            state->selectedSlot = std::max(0, state->selectedSlot - 1);
            return true;
        }
        if (e == Event::Return) {
            if (user.slots_[state->selectedSlot].has_value()) {
                onRemove(static_cast<uint8_t>(state->selectedSlot));
            } else {
                state->selectedInventory = 0;
                state->pickingMode = true;
            }
            return true;
        }
        return false;
    });
}
