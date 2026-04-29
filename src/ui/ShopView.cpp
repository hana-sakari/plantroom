#include "ui/ShopView.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

using namespace ftxui;

namespace {

constexpr int VISIBLE_CARDS = 5;
constexpr int CARD_WIDTH = 14;

Color rarityColor(Rarity r) {
    switch (r) {
        case Rarity::Common: return Color::White;
        case Rarity::Rare:   return Color::Cyan;
        case Rarity::Exotic: return Color::Yellow;
    }
    return Color::White;
}

std::string rarityName(Rarity r) {
    switch (r) {
        case Rarity::Common: return "Common";
        case Rarity::Rare:   return "Rare";
        case Rarity::Exotic: return "Exotic";
    }
    return "?";
}

Element makeShopCard(const PlantSpecies& species, bool selected) {
    // Always show seedling art in shop
    std::string art = "  [ ]  ";
    if (!species.asciiArt.empty())
        art = species.asciiArt[0];

    auto card = vbox({
        text(art) | center,
        filler(),
        separator(),
        text(species.name) | center | bold,
        text(rarityName(species.rarity)) | center | color(rarityColor(species.rarity)),
        text(std::to_string(species.price) + " coins") | center | color(Color::Yellow),
    }) | border | size(WIDTH, EQUAL, CARD_WIDTH);

    if (selected) card = card | color(Color::Green);
    return card;
}

} // namespace

Component MakeShopView(
    const std::vector<PlantSpecies>& catalog,
    std::function<void(uint32_t speciesId)> onBuy
    ) {
    struct State { int selected = 0; };
    auto state = std::make_shared<State>();

    auto renderer = Renderer([&catalog, state] {
        if (catalog.empty()) {
            return vbox({
                text("Shop") | bold | center,
                separator(),
                filler(),
                text("No plants available.") | center | color(Color::GrayDark),
                filler(),
            }) | border | flex;
        }

        int n = (int)catalog.size();
        int start = std::max(0, state->selected - VISIBLE_CARDS / 2);
        if (start + VISIBLE_CARDS > n) start = std::max(0, n - VISIBLE_CARDS);
        int end = std::min(n, start + VISIBLE_CARDS);

        std::vector<Element> cards;
        for (int i = start; i < end; i++)
            cards.push_back(makeShopCard(catalog[i], i == state->selected));

        const auto& sel = catalog[state->selected];
        std::string desc = sel.description + "  |  Income: " + std::to_string(sel.incomePerMinute) + "/min";

        return vbox({
            text("Shop") | bold | center,
            separator(),
            filler(),
            hbox({
                text(start > 0 ? " < " : "   ") | color(Color::GrayDark),
                hbox(std::move(cards)),
                text(end < n ? " > " : "   ") | color(Color::GrayDark),
            }) | hcenter,
            filler(),
            separator(),
            text(desc) | center | color(Color::GrayDark),
        }) | border | flex;
    });

    return CatchEvent(renderer, [&catalog, state](Event e) {
        int n = (int)catalog.size();
        if (n == 0) return false;
        if (e == Event::ArrowRight) { state->selected = std::min(n - 1, state->selected + 1); return true; }
        if (e == Event::ArrowLeft)  { state->selected = std::max(0,     state->selected - 1); return true; }
        return false;
    });
}
