#include "ui/MarketView.h"
#include "util/PlantUtils.h"
#include <cctype>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;
using namespace PlantUtils;

namespace {
    constexpr int VISIBLE_CARDS = 5;
    constexpr int CARD_WIDTH = 16;

    Element makeListingCard(const MarketListing &l, const std::vector<PlantSpecies> &catalog, bool selected) {
        const PlantSpecies *sp = findSpecies(catalog, l.plant.speciesId);
        std::string name = sp ? sp->name : "Unknown";
        std::string stage = stageName(l.plant.growthStage);

        auto card = vbox({
            text(name) | center | bold,
            text(stage) | center | color(Color::GrayLight),
            filler(),
            separator(),
            text(l.seller) | center | color(Color::Cyan),
            text(std::to_string(l.price) + " coins") | center | color(Color::Yellow),
        }) | border | size(WIDTH, EQUAL, CARD_WIDTH);

        if (selected) card = card | color(Color::Green);
        return card;
    }

    Element makeInventoryCard(const Plant &p, const std::vector<PlantSpecies> &catalog, bool selected) {
        const PlantSpecies *sp = findSpecies(catalog, p.speciesId);
        std::string name = sp ? sp->name : "Unknown";
        std::string stage = stageName(p.growthStage);

        auto card = vbox({
            text(name) | center | bold,
            text(stage) | center | color(Color::GrayLight),
        }) | border | size(WIDTH, EQUAL, CARD_WIDTH);

        if (selected) card = card | color(Color::Green);
        return card;
    }
} // namespace

Component MakeMarketView(
    const std::vector<MarketListing> &listings,
    const std::vector<PlantSpecies> &catalog,
    const User &user,
    const std::string &currentUsername,
    std::function<ActionResult(uint32_t)> onBuy,
    std::function<ActionResult(uint32_t, uint32_t)> onPost,
    std::function<ActionResult(uint32_t)> onRemove) {
    struct State {
        int selected = 0;
        bool confirmBuy = false;
        bool confirmRemove = false;
        bool postMode = false;
        int postIdx = 0;
        std::string priceStr = "10";
        std::string message;
        bool msgIsError = false;
    };
    auto s = std::make_shared<State>();

    auto renderer = Renderer([&, s] {
        Element listingSection;
        if (listings.empty()) {
            listingSection = text("No listings yet.") | center | color(Color::GrayDark);
        } else {
            int n = (int) listings.size();
            int start = std::max(0, s->selected - VISIBLE_CARDS / 2);
            if (start + VISIBLE_CARDS > n) start = std::max(0, n - VISIBLE_CARDS);
            int end = std::min(n, start + VISIBLE_CARDS);

            std::vector<Element> cards;
            for (int i = start; i < end; i++)
                cards.push_back(makeListingCard(listings[i], catalog, i == s->selected));

            listingSection = hbox({
                text(start > 0 ? " < " : "   ") | color(Color::GrayDark),
                hbox(std::move(cards)),
                text(end < n ? " > " : "   ") | color(Color::GrayDark),
            }) | hcenter;
        }

        Element bottomBar;
        if (s->confirmRemove && !listings.empty()) {
            const auto &l = listings[s->selected];
            const PlantSpecies *sp = findSpecies(catalog, l.plant.speciesId);
            std::string name = sp ? sp->name : "plant";
            bottomBar = hbox({
                text("Remove ") | bold,
                text(name) | bold | color(Color::Red),
                text(" from market? Plant returns to inventory.  "),
                text("[Enter]") | color(Color::Red), text(" Confirm   "),
                text("[Esc]") | color(Color::GrayDark), text(" Cancel"),
            }) | hcenter;
        } else if (s->confirmBuy && !listings.empty()) {
            const auto &l = listings[s->selected];
            const PlantSpecies *sp = findSpecies(catalog, l.plant.speciesId);
            std::string name = sp ? sp->name : "plant";
            bottomBar = hbox({
                text("Buy ") | bold,
                text(name) | bold | color(Color::Green),
                text(" from " + l.seller + " for "),
                text(std::to_string(l.price) + " coins") | color(Color::Yellow),
                text("?  "),
                text("[Enter]") | color(Color::Green), text(" Confirm   "),
                text("[Esc]") | color(Color::GrayDark), text(" Cancel"),
            }) | hcenter;
        } else if (s->postMode) {
            if (user.plantsInventory.empty()) {
                bottomBar = text("Your inventory is empty.") | center | color(Color::GrayDark);
            } else {
                int n = (int) user.plantsInventory.size();
                int start = std::max(0, s->postIdx - VISIBLE_CARDS / 2);
                if (start + VISIBLE_CARDS > n) start = std::max(0, n - VISIBLE_CARDS);
                int end = std::min(n, start + VISIBLE_CARDS);

                std::vector<Element> cards;
                for (int i = start; i < end; i++)
                    cards.push_back(makeInventoryCard(user.plantsInventory[i], catalog, i == s->postIdx));

                bottomBar = vbox({
                    hbox({
                        text(start > 0 ? " < " : "   ") | color(Color::GrayDark),
                        hbox(std::move(cards)),
                        text(end < n ? " > " : "   ") | color(Color::GrayDark),
                    }) | hcenter,
                    hbox({
                        text("  Price: "),
                        text(s->priceStr.empty() ? "_" : s->priceStr) | bold | color(Color::Yellow),
                        text(" coins   type digits, Backspace to edit   "),
                        text("[Enter]") | color(Color::Green), text(" Post   "),
                        text("[Esc]") | color(Color::GrayDark), text(" Cancel"),
                    }) | hcenter,
                });
            }
        } else if (!s->message.empty()) {
            bottomBar = text(s->message) | center | color(s->msgIsError ? Color::Red : Color::Green);
        } else if (!listings.empty()) {
            bool ownListing = listings[s->selected].seller == currentUsername;
            bottomBar = hbox({
                text(ownListing ? "[Enter] Remove listing   " : "[Enter] Buy   "),
                text("[p] Post   [Left/Right] Navigate"),
            }) | hcenter | color(Color::GrayDark);
        } else {
            bottomBar = text("[p] Post a plant") | hcenter | color(Color::GrayDark);
        }

        return vbox({
            text("Market") | bold | center,
            separator(),
            filler(),
            listingSection,
            filler(),
            separator(),
            s->postMode ? text("Post a plant") | bold | hcenter : text("") | size(HEIGHT, EQUAL, 0),
            bottomBar,
        }) | border | flex;
    });

    return CatchEvent(renderer, [&, s, onBuy, onPost, onRemove](Event e) {
        s->message = "";

        if (s->confirmRemove) {
            if (e == Event::Return) {
                auto res = onRemove(listings[s->selected].id);
                s->confirmRemove = false;
                if (s->selected >= (int) listings.size() - 1) s->selected = std::max(0, s->selected - 1);
                if (res.ok) {
                    s->message = "Listing removed.";
                    s->msgIsError = false;
                } else {
                    s->message = res.error;
                    s->msgIsError = true;
                }
                return true;
            }
            if (e == Event::Escape) {
                s->confirmRemove = false;
                return true;
            }
            return true;
        }

        if (s->confirmBuy) {
            if (e == Event::Return) {
                auto res = onBuy(listings[s->selected].id);
                s->confirmBuy = false;
                if (res.ok) {
                    s->message = "Purchase complete!";
                    s->msgIsError = false;
                } else {
                    s->message = res.error;
                    s->msgIsError = true;
                }
                return true;
            }
            if (e == Event::Escape) {
                s->confirmBuy = false;
                return true;
            }
            return true;
        }

        if (s->postMode) {
            int inv = (int) user.plantsInventory.size();
            if (e == Event::ArrowRight && inv > 0) {
                s->postIdx = std::min(inv - 1, s->postIdx + 1);
                return true;
            }
            if (e == Event::ArrowLeft && inv > 0) {
                s->postIdx = std::max(0, s->postIdx - 1);
                return true;
            }
            if (e == Event::Backspace) {
                if (!s->priceStr.empty()) s->priceStr.pop_back();
                return true;
            }
            if (e.is_character() && std::isdigit(e.character()[0]) && s->priceStr.size() < 6) {
                s->priceStr += e.character()[0];
                return true;
            }
            if (e == Event::Return && inv > 0) {
                uint32_t price = s->priceStr.empty() ? 0 : static_cast<uint32_t>(std::stoul(s->priceStr));
                if (price == 0) {
                    s->message = "Price must be at least 1";
                    s->msgIsError = true;
                    return true;
                }
                auto res = onPost(s->postIdx, price);
                s->postMode = false;
                s->postIdx = 0;
                s->priceStr = "10";
                if (res.ok) {
                    s->message = "Plant posted!";
                    s->msgIsError = false;
                } else {
                    s->message = res.error;
                    s->msgIsError = true;
                }
                return true;
            }
            if (e == Event::Escape) {
                s->postMode = false;
                s->priceStr = "10";
                return true;
            }
            return true;
        }

        int n = (int) listings.size();
        if (e == Event::ArrowRight) {
            if (n > 0) s->selected = std::min(n - 1, s->selected + 1);
            return true;
        }
        if (e == Event::ArrowLeft) {
            if (n > 0) s->selected = std::max(0, s->selected - 1);
            return true;
        }
        if (e == Event::Character('p')) {
            s->postMode = true;
            s->priceStr = "10";
            return true;
        }
        if (e == Event::Return && n > 0) {
            if (listings[s->selected].seller == currentUsername)
                s->confirmRemove = true;
            else
                s->confirmBuy = true;
            return true;
        }
        return false;
    });
}
