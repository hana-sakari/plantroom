#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <vector>

using namespace ftxui;

int main(int argc, char *argv[]) {
    auto screen = ScreenInteractive::Fullscreen();

    int tab = 0;

    std::vector<std::string> tabNames = {" Your Room ", " SHOP ", " Inventory "};

    auto tabToggle = Toggle(&tabNames, &tab);
    auto layout = Container::Vertical({tabToggle});

    auto statusBar = hbox({
        text(" PlantRoom ") | bold,
        separator(),
        text(" Player: Cat "),
        separator(),
        text(" Coins: 13 ") | color(Color::Yellow),
    }) | border;


    auto renderer = Renderer(layout, [&] {
        Element content;
        if (tab == 0) {
            content = vbox({
                text("Your Room") | bold | center,
                separator(),
                paragraph(
                    "  +-----------------------------------+\n"
                    "  |  [slot 1]        [slot 2]        |\n"
                    "  |                                  |\n"
                    "  |    [slot 3]    [slot 4]          |\n"
                    "  |                                  |\n"
                    "  |  [slot 5]        [slot 6]        |\n"
                    "  +-----------------------------------+"
                ) | center
            });

        }
        else if (tab == 1) {
            content = text("You can buy new plants here ") | center;
        }
        else {
            content = text("Your plants are stored here") | center;
        }


        return vbox({
            statusBar,
            separator(),
            text("PlantRoom") | bold | center,
            separator(),
            tabToggle->Render() | center,
            separator(),
            content | flex,
            separator(),
            text("Press q to quit") | center | dim,
        }) | border;
    });

    auto app = CatchEvent(renderer, [&](Event e) {
        if (e == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });


    screen.Loop(app);


    return 0;
}
