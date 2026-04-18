#include "ui/App.h"
#include "InventoryView.h"
#include "RoomView.h"
#include "ShopView.h"
#include "StatusBar.h"

#include "ftxui/component/screen_interactive.hpp"

using namespace ftxui;

App::App(const std::string &username, Game& game) :
        game(game), saveManager("data/saves"){
    try {
        user = saveManager.load(username);
    }
    catch (const std::exception& e) {
        user = {0, username, 0, 0, {}, {}};
    }
}

void App::run() {
    auto screen = ScreenInteractive::Fullscreen();

    auto roomView = MakeRoomView(
        user,
        [this](uint8_t slot, uint32_t plantId) {
            onPlacePlant(slot, plantId);
        });

    screen.Loop(roomView);



}
