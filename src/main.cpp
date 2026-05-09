#include "client/ServerConnection.h"
#include "ui/App.h"
#include "ui/LoginView.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

static std::string sshPublicKey() {
    const char *path = std::getenv("SSH_USER_AUTH");
    if (!path) return {};
    std::ifstream f(path);
    std::string line;
    if (!std::getline(f, line)) return {};
    // file format: "publickey <type> <blob>"
    const std::string prefix = "publickey ";
    if (line.rfind(prefix, 0) == 0)
        return line.substr(prefix.size());
    return {};
}

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

        auto pubkey = sshPublicKey();
        if (!pubkey.empty())
            conn.addPublicKey(pubkey);
    }

    App app(conn);
    app.run();
    return 0;
}
