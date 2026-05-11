#include "ui/StatusBar.h"

#include <format>

using namespace ftxui;


ftxui::Element RenderStatusBar(const std::string &playerName, int coins) {
    auto statusBar = hbox({
        text(" PlantRoom ") | bold,
        separator(),
        text(std::format(" Player: {}", playerName)),
        separator(),
        text(std::format(" Coins: {}", coins)) | color(Color::Yellow),
    }) | border;

    return statusBar;
}
