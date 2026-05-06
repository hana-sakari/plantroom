#pragma once
#include <string>

#include "AuthStore.h"
#include "SharedState.h"

class Session {
public:
    Session(int fd, SharedState &sharedState, AuthStore &authStore);

    void run();

private:
    int fd;
    SharedState &sharedState;
    std::string username;
    AuthStore &authStore;
};
