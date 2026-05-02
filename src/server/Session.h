#pragma once
#include <string>
#include "SharedState.h"

class Session {
public:
    Session(int fd, SharedState &sharedState);

    void run();

private:
    int fd;
    SharedState &sharedState;
    std::string username;
};
