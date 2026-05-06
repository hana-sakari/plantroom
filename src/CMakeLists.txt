cmake_minimum_required(VERSION 3.20)
project(PlantRoom VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG v6.1.9
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json ftxui)

add_executable(plantroom
        main.cpp
        core/Game.cpp
        core/User.cpp
        core/Plant.cpp
        core/PlantSpecies.cpp
        core/Shop.cpp
        core/IncomeEngine.cpp
        ui/App.cpp
        ui/RoomView.cpp
        ui/ShopView.cpp
        ui/InventoryView.cpp
        ui/StatusBar.cpp
        persistence/SaveManager.cpp
        persistence/CatalogLoader.cpp
        util/Timer.cpp
        util/AsciiRenderer.cpp
)

target_include_directories(plantroom PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(plantroom PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
        nlohmann_json::nlohmann_json
)

if (WIN32)
    set_target_properties(plantroom PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif ()
add_executable(plantroom-server
        server/server_main.cpp
        server/SharedState.cpp
        core/User.cpp
        core/Plant.cpp
        core/PlantSpecies.cpp
        core/IncomeEngine.cpp
        persistence/CatalogLoader.cpp
        persistence/SaveManager.cpp
        server/Session.cpp
        server/Protocol.cpp
        server/PlantRoomServer.cpp
        server/AuthStore.cpp
        server/AuthStore.h
)

target_include_directories(plantroom-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)

target_link_libraries(plantroom-server PRIVATE
        nlohmann_json::nlohmann_json
        OpenSSL::Crypto
)
