#include "ui/InventoryView.h"
#include "util/PlantUtils.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

using namespace ftxui;
using namespace PlantUtils;

namespace {

constexpr int VISIBLE_CARDS = 5;
constexpr int CARD_WIDTH = 14;

Element makePlantCard(const Plant& plant, const std::vector<PlantSpecies>& catalog, bool selected) {
    std::string name = std::to_string(plant.speciesId);
    std::string art = "  [ ]  ";

    if (const auto* sp = findSpecies(catalog, plant.speciesId)) {
        name = sp->name;
        int idx = static_cast<int>(plant.growthStage);
        if (idx < (int)sp->asciiArt.size())
            art = sp->asciiArt[idx];
    }

    auto card = vbox({
        text(art) | center,
        filler(),
        separator(),
        text(name) | center | bold,
        text(stageName(plant.growthStage)) | center,
    }) | border | size(WIDTH, EQUAL, CARD_WIDTH);

    if (selected) card = card | color(Color::Green);
    return card;
}

} // namespace

Component MakeInventoryView(const User& user, const std::vector<PlantSpecies>& catalog) {
    struct State { int selected = 0; };
    auto state = std::make_shared<State>();

    auto renderer = Renderer([&user, &catalog, state] {
        if (user.plantsInventory.empty()) {
            return vbox({
                text("Your Inventory") | bold | center,
                separator(),
                filler(),
                text("No plants yet! Visit the Shop to buy some.") | center | color(Color::GrayDark),
                filler(),
            }) | border | flex;
        }

        int n = (int)user.plantsInventory.size();
        int start = std::max(0, state->selected - VISIBLE_CARDS / 2);
        if (start + VISIBLE_CARDS > n) start = std::max(0, n - VISIBLE_CARDS);
        int end = std::min(n, start + VISIBLE_CARDS);

        std::vector<Element> cards;
        for (int i = start; i < end; i++)
            cards.push_back(makePlantCard(user.plantsInventory[i], catalog, i == state->selected));

        std::string counter = std::to_string(state->selected + 1) + " / " + std::to_string(n);

        return vbox({
            text("Your Inventory") | bold | center,
            separator(),
            filler(),
            hbox({
                text(start > 0 ? " < " : "   ") | color(Color::GrayDark),
                hbox(std::move(cards)),
                text(end < n ? " > " : "   ") | color(Color::GrayDark),
            }) | hcenter,
            filler(),
            separator(),
            text(counter) | center | color(Color::GrayDark),
        }) | border | flex;
    });

    return CatchEvent(renderer, [&user, state](Event e) {
        int n = (int)user.plantsInventory.size();
        if (n == 0) return false;
        if (e == Event::ArrowRight) { state->selected = std::min(n - 1, state->selected + 1); return true; }
        if (e == Event::ArrowLeft)  { state->selected = std::max(0,     state->selected - 1); return true; }
        return false;
    });
}
