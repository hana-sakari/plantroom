# Specification

## Theme: Terminal-based interactive application

### User interaction

Users interact through a terminal UI navigated with arrow keys and Enter. The interface updates in real time within a session — coin balance, inventory, and plant growth stages refresh automatically every second without any input from the user.

### What the program does

PlantRoom is a multiplayer terminal game where users grow plants and trade them. Each user connects via SSH, which automatically launches the game client. The client connects to a background server process that manages all shared state.

After logging in or registering, the user can:

- Buy plants from the shop using in-game coins
- Place plants into one of six room slots and watch them grow through stages
- Move plants back to inventory and post them on the marketplace
- Buy plants from other users on the marketplace
- Earn coins passively while the app is open, and receive offline income for time spent away

All actions persist across sessions via per-user save files on the server.

### Core features

- Terminal UI built with FTXUI
- Automatic session start via SSH using `AuthorizedKeysCommand`
- Password authentication with SHA-256 + salt hashing
- SSH public key login — on first connection the key is registered, on subsequent connections login is automatic with no UI shown
- Persistent user data stored as JSON
- Multi-user support with simultaneous SSH connections
- Marketplace where users can post and buy plants from each other
- Passive coin income with offline progress calculated on login

## Architecture

The project is split into two executables: `plantroom-server` and `plantroom` (the client).

The server runs as a background daemon and holds all shared state in memory. It accepts connections over a Unix domain socket and handles each client in its own thread. Shared data is protected by per-user mutexes and a separate mutex for the marketplace. A background tick thread runs every second to update coin balances for all online users.

The client is a thin FTXUI frontend that sends commands to the server and renders the responses. It holds no authoritative state of its own.

Communication uses a simple newline-delimited JSON protocol over the socket.

**Server modules:** `PlantRoomServer`, `Session`, `SharedState`, `AuthStore`, `Protocol`

**Client modules:** `ServerConnection`, `App`, `LoginView`, `RoomView`, `ShopView`, `InventoryView`, `MarketView`, `StatusBar`

**Shared core:** `User`, `Plant`, `PlantSpecies`, `Shop`, `MarketListing`, `IncomeEngine`

**Persistence:** `SaveManager` (per-user JSON saves), `CatalogLoader` (reads `plants.json`)

## Third-party libraries

- nlohmann/json — JSON parsing and serialization
- FTXUI — terminal UI rendering and event handling
- OpenSSL (libcrypto) — SHA-256 password hashing

All libraries except OpenSSL are fetched automatically by CMake via FetchContent. OpenSSL must be installed on the system.

## SSH setup

The SSH integration uses OpenSSH's `AuthorizedKeysCommand` feature. When a user connects, sshd calls the `plantroom-keys` script with the connecting key. The script looks up the key in `auth.json`:

- If the key belongs to a known user, it outputs an `authorized_keys` entry with `command="plantroom --user <username>"`, which makes sshd launch the client and skip the login screen.
- If the key is unknown, it outputs an entry with `command="plantroom"` (no username), so the user sees the login/register screen. After logging in, the client registers the key automatically for future connections.

The `scripts/` directory contains `plantroom-keys` and `sshd_plantroom.conf` with the required sshd directives.
