#include "ui/App.h"
#include "InventoryView.h"
#include "RoomView.h"
#include "ShopView.h"
#include "StatusBar.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

void App::onBuyPlant(uint32_t speciesId) {
    auto buyResult = game.getShop().buy(speciesId, user);
    //TODO: change ui based on the result of buying
}

void App::onPlacePlant(uint8_t slot, uint32_t inventoryIdx) {
    game.placePlant(user, slot, inventoryIdx);
}

void App::onRemovePlant(uint8_t slot) {
    game.removePlant(user, slot);
}

App::App(const std::string &username, Game &game) : game(game), saveManager("data/saves") {
    try {
        user = saveManager.load(username);
    } catch (const std::exception &) {
        user = {0, username, 0, 0, {}, {}};
    }
}

void App::run() {
    auto screen = ScreenInteractive::Fullscreen();

    int tab = 0;
    int focusedArea = 0; // 0 = tab bar, 1 = content
    const std::vector<std::string> tabNames = {" Inventory ", " Room ", " Shop "};
    const auto tabToggle = Toggle(&tabNames, &tab);

    const auto &catalog = game.getShop().getAvailable();

    auto inventoryView = MakeInventoryView(user, catalog);
    auto roomView = MakeRoomView(
        user,
        catalog,
        [this](uint8_t slot, int inventoryIdx) {
            onPlacePlant(slot, static_cast<uint32_t>(inventoryIdx));
        },
        [this](uint8_t slot) {
            onRemovePlant(slot);
        }
    );
    auto shopView = MakeShopView(
        catalog,
        [this](const uint32_t speciesId) {
            onBuyPlant(speciesId);
        });

    // Used for rendering only, Container::Tab drops keyboard events unless FTXUI
    // considers it "focused" which it never is
    auto tabContent = Container::Tab({inventoryView, roomView, shopView}, &tab);
    // Direct handles indexed in tab order, events are routed here manually
    std::vector<Component> views = {inventoryView, roomView, shopView};

    auto renderer = Renderer([&] {
        auto tabBar = tabToggle->Render();
        if (focusedArea == 0)
            tabBar = hbox({text(" > ") | color(Color::Green), tabBar, text(" < ") | color(Color::Green)});
        else
            tabBar = hbox({text("   "), tabBar, text("   ")});

        std::string hint = focusedArea == 0
                           ? "  Down: enter view     Left/Right: switch tabs  "
                           : "  Up: back to tabs     Left/Right: scroll / select  ";

        return vbox({
            RenderStatusBar(user.name, (int) user.coins),
            tabBar,
            text(hint) | hcenter | color(Color::GrayDark),
            separator(),
            tabContent->Render() | flex,
            separator(),
            text("  q  quit  ") | color(Color::GrayDark),
        });
    });

    auto withNav = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        // Switch focus areas with Up/Down
        if (e == Event::ArrowDown && focusedArea == 0) {
            focusedArea = 1;
            return true;
        }
        if (e == Event::ArrowUp && focusedArea == 1) {
            focusedArea = 0;
            return true;
        }

        if (focusedArea == 0) {
            // Route Left/Right to the toggle, consume everything else
            if (e == Event::ArrowLeft || e == Event::ArrowRight)
                tabToggle->OnEvent(e);
            return true;
        }

        // focusedArea == 1: route directly to the active view, bypassing Container::Tab
        if (tab >= 0 && tab < (int) views.size())
            return views[tab]->OnEvent(e);
        return false;
    });

    screen.Loop(withNav);
}
