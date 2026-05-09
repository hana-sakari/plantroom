#include "client/ServerConnection.h"
#include "ui/App.h"
#include "ui/LoginView.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string sshUser;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--user") {
            sshUser = argv[i + 1];
            break;
        }
    }

    ServerConnection conn("/tmp/plantroom.sock");

    if (!sshUser.empty()) {
        auto res = conn.loginKey(sshUser);
        if (!res.ok) {
            std::cerr << "Login failed: " << res.error << "\n";
            return 1;
        }
    } else {
        std::string username = RunLoginFlow(conn);
        if (username.empty()) return 1;
    }

    App app(conn);
    app.run();
    return 0;
}
