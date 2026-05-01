#include "core/Game.h"
#include "core/Shop.h"
#include "persistence/CatalogLoader.h"
#include "ui/App.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
static std::filesystem::path exeDir() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
}
#else
#include <unistd.h>
static std::filesystem::path exeDir() {
    return std::filesystem::canonical("/proc/self/exe").parent_path();
}
#endif

int main(int argc, char* argv[]) {
    auto base = exeDir();
    auto species = CatalogLoader::load(base / "data" / "plants.json");
    Shop shop(species);
    Game game(shop);

    App app("bob", game, base / "data" / "saves");
    app.run();
    return 0;
}
