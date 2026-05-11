#include "ui/ShopView.h"
#include "util/PlantUtils.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/event.hpp"

using namespace ftxui;
using namespace PlantUtils;

namespace {
    constexpr int VISIBLE_CARDS = 5;
    constexpr int CARD_WIDTH = 14;

    Color rarityColor(Rarity r) {
        switch (r) {
            case Rarity::Common: return Color::White;
            case Rarity::Rare: return Color::Cyan;
            case Rarity::Exotic: return Color::Yellow;
        }
        return Color::White;
    }

    Element makeShopCard(const PlantSpecies &species, bool selected) {
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
    const std::vector<PlantSpecies> &catalog,
    std::function<BuyResult(uint32_t speciesId)> onBuy
) {
    struct ShopState {
        int selected = 0;
        bool buyingMode = false;
        std::string message;
        bool messageIsError = false;
    };
    auto state = std::make_shared<ShopState>();

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

        int n = (int) catalog.size();
        int start = std::max(0, state->selected - VISIBLE_CARDS / 2);
        if (start + VISIBLE_CARDS > n) start = std::max(0, n - VISIBLE_CARDS);
        int end = std::min(n, start + VISIBLE_CARDS);

        std::vector<Element> cards;
        for (int i = start; i < end; i++)
            cards.push_back(makeShopCard(catalog[i], i == state->selected));

        Element bottomBar;
        if (state->buyingMode) {
            const auto &sel = catalog[state->selected];
            bottomBar = hbox({
                text("Buy ") | bold,
                text(sel.name) | bold | color(rarityColor(sel.rarity)),
                text(" for ") | bold,
                text(std::to_string(sel.price) + " coins") | bold | color(Color::Yellow),
                text("?  "),
                text("[Enter]") | color(Color::Green),
                text(" Confirm   "),
                text("[Esc]") | color(Color::GrayDark),
                text(" Cancel"),
            }) | hcenter;
        } else if (!state->message.empty()) {
            Color msgColor = state->messageIsError ? Color::Red : Color::Green;
            bottomBar = text(state->message) | center | color(msgColor);
        } else {
            const auto &sel = catalog[state->selected];
            std::string desc = sel.description + "  |  Income: " + std::to_string(sel.incomePerMinute) + "/min";
            bottomBar = text(desc) | center | color(Color::GrayDark);
        }

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
            bottomBar,
        }) | border | flex;
    });

    return CatchEvent(renderer, [&catalog, state, onBuy](Event e) {
        if (state->buyingMode) {
            if (e == Event::Return) {
                BuyResult result = onBuy(catalog[state->selected].id);
                state->buyingMode = false;
                if (result == BuyResult::Success) {
                    state->message = "Thanks for your purchase!";
                    state->messageIsError = false;
                } else if (result == BuyResult::NotEnoughCoins) {
                    state->message = "Not enough coins!";
                    state->messageIsError = true;
                } else {
                    state->message = "Plant not found.";
                    state->messageIsError = true;
                }
                return true;
            }
            if (e == Event::Escape) {
                state->buyingMode = false;
                return true;
            }
            return true;
        } else {
            int n = (int) catalog.size();
            if (n == 0) return false;
            if (e == Event::ArrowRight) {
                state->selected = std::min(n - 1, state->selected + 1);
                return true;
            }
            if (e == Event::ArrowLeft) {
                state->selected = std::max(0, state->selected - 1);
                return true;
            }
            if (e == Event::Return) {
                state->buyingMode = true;
                state->message = "";
                return true;
            }
            return false;
        }
    });
}
