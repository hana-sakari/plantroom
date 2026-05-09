#include "ui/LoginView.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

std::string RunLoginFlow(ServerConnection &conn) {
    std::string username;
    std::string password;
    std::string errorMsg;
    std::string loggedInAs;

    auto screen = ScreenInteractive::Fullscreen();

    InputOption passwordOpt;
    passwordOpt.password = true;

    auto usernameInput = Input(&username, "username");
    auto passwordInput = Input(&password, "password", passwordOpt);

    auto tryLogin = [&] {
        errorMsg.clear();
        if (username.empty() || password.empty()) {
            errorMsg = "username and password required";
            return;
        }
        auto res = conn.loginPass(username, password);
        if (res.ok) {
            loggedInAs = username;
            screen.ExitLoopClosure()();
        } else {
            errorMsg = res.error;
            password.clear();
        }
    };

    auto tryRegister = [&] {
        errorMsg.clear();
        if (username.empty() || password.empty()) {
            errorMsg = "username and password required";
            return;
        }
        auto res = conn.registerUser(username, password);
        if (res.ok) {
            loggedInAs = username;
            screen.ExitLoopClosure()();
        } else {
            errorMsg = res.error;
        }
    };

    auto loginBtn    = Button("  Login  ",    tryLogin);
    auto registerBtn = Button("  Register  ", tryRegister);

    auto form = Container::Vertical({
        usernameInput,
        passwordInput,
        Container::Horizontal({loginBtn, registerBtn}),
    });

    auto renderer = Renderer(form, [&] {
        return vbox({
            filler(),
            vbox({
                text("PlantRoom") | bold | center,
                separator(),
                hbox({text(" Username : "), usernameInput->Render() | flex}),
                hbox({text(" Password : "), passwordInput->Render() | flex}),
                separator(),
                hbox({loginBtn->Render(), text("  "), registerBtn->Render()}) | center,
                separator(),
                text(errorMsg.empty() ? "Tab to navigate   Enter to confirm" : errorMsg)
                    | center
                    | color(errorMsg.empty() ? Color::GrayDark : Color::Red),
            }) | border | size(WIDTH, EQUAL, 50) | center,
            filler(),
        });
    });

    screen.Loop(renderer);
    return loggedInAs;
}
