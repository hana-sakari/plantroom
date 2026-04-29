#include "Game.h"

#include <utility>

Game::Game(Shop shop) : shop_(std::move(shop)) { }

