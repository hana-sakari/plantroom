#include "core/Game.h"
#include "core/Shop.h"
#include "persistence/CatalogLoader.h"
#include "ui/App.h"

int main(int argc, char *argv[]) {
    auto species = CatalogLoader::load("./../data/plants.json");
    Shop shop(species);
    Game game(shop);

    App app("bob", game);
    app.run();
    return 0;
}
