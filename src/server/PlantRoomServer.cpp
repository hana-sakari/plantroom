#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/un.h>
#include <cstring>
#include <chrono>
#include <thread>

#include "server/PlantRoomServer.h"

#include "server/Session.h"

PlantRoomServer::PlantRoomServer(const std::string &socketPath,
    const std::filesystem::path &dataDir)
    : socketPath_(socketPath), sharedState_(dataDir), authStore_(dataDir / "auth.json") {
}

PlantRoomServer::~PlantRoomServer() {
    stop();
    if (tickThread_.joinable())
        tickThread_.join();
    if (serverFd_ >= 0)
        close(serverFd_);
}

void PlantRoomServer::run() {
    serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);
    unlink(socketPath_.c_str());

    bind(serverFd_, (sockaddr *) &addr, sizeof(addr));
    chmod(socketPath_.c_str(), 0777);

    listen(serverFd_, 10); //10 - max clients queued waiting to be accepted

    running_ = true;
    tickThread_ = std::thread(&PlantRoomServer::tickLoop, this);
    acceptLoop();
}

void PlantRoomServer::stop() {
    running_ = false;
    if (serverFd_ >= 0)
        close(serverFd_);
}


void PlantRoomServer::acceptLoop() {
    while (running_) {
        int clientFd = accept(serverFd_, nullptr, nullptr);
        if (clientFd < 0)
            continue; //accept failed

        std::thread([clientFd, this]() {
            Session session(clientFd, sharedState_, authStore_);
            session.run();
            close(clientFd);
        }).detach();
    }
}

void PlantRoomServer::tickLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sharedState_.tickAllOnlineUsers();
    }
}
