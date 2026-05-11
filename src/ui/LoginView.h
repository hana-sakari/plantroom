#pragma once
#include "client/ServerConnection.h"
#include <string>

// Shows login/register screen, blocks until success, returns the logged-in username
std::string RunLoginFlow(ServerConnection &conn);
