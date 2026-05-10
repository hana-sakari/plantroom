#include "ui/App.h"
#include "InventoryView.h"
#include "MarketView.h"
#include "RoomView.h"
#include "ShopView.h"
#include "StatusBar.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>
#include <atomic>

using namespace ftxui;

BuyResult App::onBuyPlant(uint32_t speciesId) {
    auto res = conn_.buy(speciesId);
    if (res.ok) {
        user_ = conn_.getState();
        return BuyResult::Success;
    }
    if (res.error == "not enough coins") return BuyResult::NotEnoughCoins;
    return BuyResult::NotFound;
}

void App::onPlacePlant(uint8_t slot, uint32_t inventoryIdx) {
    auto res = conn_.place(slot, inventoryIdx);
    if (res.ok) user_ = conn_.getState();
}

void App::onRemovePlant(uint8_t slot) {
    auto res = conn_.remove(slot);
    if (res.ok) user_ = conn_.getState();
}

App::App(ServerConnection &conn) : conn_(conn) {
    catalog_ = conn_.getShopList();
    user_ = conn_.getState();
    market_ = conn_.getMarketListings();
}

void App::run() {
    auto screen = ScreenInteractive::Fullscreen();
    std::atomic<bool> running{true};

    int tab = 0;
    int focusedArea = 0;
    const std::vector<std::string> tabNames = {" Inventory ", " Room ", " Shop ", " Market "};
    const auto tabToggle = Toggle(&tabNames, &tab);

    auto inventoryView = MakeInventoryView(user_, catalog_);
    auto roomView = MakeRoomView(
        user_,
        catalog_,
        [this](uint8_t slot, int inventoryIdx) {
            onPlacePlant(slot, static_cast<uint32_t>(inventoryIdx));
        },
        [this](uint8_t slot) {
            onRemovePlant(slot);
        }
    );
    auto shopView = MakeShopView(
        catalog_,
        [this](uint32_t speciesId) -> BuyResult {
            return onBuyPlant(speciesId);
        });

    auto marketView = MakeMarketView(
        market_,
        catalog_,
        user_,
        user_.name,
        [this](uint32_t listingId) -> ActionResult {
            auto res = conn_.buyFromMarket(listingId);
            if (res.ok) {
                user_ = conn_.getState();
                market_ = conn_.getMarketListings();
            }
            return res;
        },
        [this](uint32_t inventoryIdx, uint32_t price) -> ActionResult {
            auto res = conn_.postToMarket(inventoryIdx, price);
            if (res.ok) {
                user_ = conn_.getState();
                market_ = conn_.getMarketListings();
            }
            return res;
        },
        [this](uint32_t listingId) -> ActionResult {
            auto res = conn_.removeFromMarket(listingId);
            if (res.ok) {
                user_ = conn_.getState();
                market_ = conn_.getMarketListings();
            }
            return res;
        });

    auto tabContent = Container::Tab({inventoryView, roomView, shopView, marketView}, &tab);
    std::vector<Component> views = {inventoryView, roomView, shopView, marketView};

    auto renderer = Renderer([&] {
        auto tabBar = tabToggle->Render();
        if (focusedArea == 0)
            tabBar = hbox({text(" > ") | color(Color::Green), tabBar, text(" < ") | color(Color::Green)});
        else
            tabBar = hbox({text("   "), tabBar, text("   ")});

        std::string hint;
        if (focusedArea == 0) {
            hint = "  Down: enter view     Left/Right: switch tabs  ";
        } else if (tab == 0) {
            hint = "  Up: back to tabs     Left/Right: scroll inventory  ";
        } else if (tab == 1) {
            hint = "  Up: back to tabs     Left/Right: navigate slots     Enter: place / remove  ";
        } else if (tab == 2) {
            hint = "  Up: back to tabs     Left/Right: scroll shop     Enter: buy  ";
        } else {
            hint = "  Up: back to tabs     Left/Right: navigate     Enter: buy     p: post  ";
        }

        return vbox({
            RenderStatusBar(user_.name, (int) user_.coins),
            tabBar,
            text(hint) | hcenter | color(Color::GrayDark),
            separator(),
            tabContent->Render() | flex,
            separator(),
            text("  q  quit  ") | color(Color::GrayDark),
        });
    });

    auto withNav = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Custom) {
            user_ = conn_.getState();
            if (tab == 3) market_ = conn_.getMarketListings();
            return true;
        }
        if (e == Event::Character('q')) {
            conn_.save();
            screen.ExitLoopClosure()();
            return true;
        }
        if (e == Event::ArrowDown && focusedArea == 0) {
            focusedArea = 1;
            return true;
        }
        if (e == Event::ArrowUp && focusedArea == 1) {
            focusedArea = 0;
            return true;
        }

        if (focusedArea == 0) {
            if (e == Event::ArrowLeft || e == Event::ArrowRight)
                tabToggle->OnEvent(e);
            return true;
        }

        if (tab >= 0 && tab < (int) views.size())
            return views[tab]->OnEvent(e);
        return false;
    });

    std::thread tickThread([&]() {
        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            screen.PostEvent(Event::Custom);
        }
    });

    try {
        screen.Loop(withNav);
    } catch (const std::exception &) {
        // socket died mid-session
    }

    running = false;
    tickThread.join();
}
