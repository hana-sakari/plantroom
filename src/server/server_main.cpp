#include "server/PlantRoomServer.h"

int main() {
    PlantRoomServer server("/tmp/plantroom.sock", "data");
    server.run();
    return 0;
}
