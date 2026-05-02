#pragma once
#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

#include "SharedState.h"

class PlantRoomServer {
public:
    PlantRoomServer(const std::string &socketPath, const std::filesystem::path &dataDir);

    ~PlantRoomServer();

    void run();

    void stop(); // signals shutdown from another thread

private:
    void acceptLoop();

    void tickLoop(); // every second calls sharedState_.tickAllOnlineUsers()

    std::string socketPath_;
    SharedState sharedState_;
    int serverFd_ = -1;
    std::atomic<bool> running_{false};
    std::thread tickThread_;
};
