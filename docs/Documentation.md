# Documentation

## Building on Windows

This application uses POSIX sockets and is intended to run on a Linux server, so it does not build natively on Windows. 
The intended way to use it is to connect via `ssh plantroom@<server-ip>` to a Linux machine running the server. 
Since no remote server is available for this project right now, Windows testing is not supported.

## Building on Linux

### Prerequisites

You need CMake 3.20 or newer, a C++20-capable compiler (GCC 11+ works), git, and OpenSSL development headers.

On Fedora:
```
sudo dnf install cmake gcc-c++ git openssl-devel
```

On Ubuntu/Debian:
```
sudo apt install cmake g++ git libssl-dev
```

### Build

```
cd project/src
cmake -B build -S .
cmake --build build
```

CMake will automatically download FTXUI and nlohmann/json on the first build. This takes a few minutes the first time.

The build produces two executables in `build/`:
- `plantroom-server` — the server daemon
- `plantroom` — the client

It also copies the `data/` folder into `build/data/`, which is where the server reads and writes everything at runtime.

## Running

Open two terminals. In the first, start the server:

```
./build/plantroom-server
```

In the second, start the client:

```
./build/plantroom
```

The server must be running before the client connects. The server keeps running until you kill it (Ctrl+C). 
When a client disconnects, their data is saved automatically.

## SSH setup

This is only needed if you want the full SSH-based login flow, where users connect with `ssh` and the game launches automatically.
For local testing the manual steps above are enough.

The `scripts/` directory contains two files.

`sshd_plantroom.conf` configures sshd to use the key lookup script. Copy it to `/etc/ssh/sshd_config.d/` and restart sshd:

```
sudo cp scripts/sshd_plantroom.conf /etc/ssh/sshd_config.d/
sudo systemctl restart sshd
```

`plantroom-keys` is the script sshd calls for each incoming connection. 
It reads `auth.json` and decides whether to launch the client directly or show the login screen.
Edit the two path variables at the top of the script to point to your `auth.json` and the built `plantroom` binary:

```bash
AUTH_JSON="/path/to/build/data/auth.json"
BINARY="/path/to/build/plantroom"
```

Then install it where sshd expects it:

```
sudo cp scripts/plantroom-keys /usr/local/bin/plantroom-keys
sudo chmod 755 /usr/local/bin/plantroom-keys
```

After this, users connect with:

```
ssh plantroom@<server-ip>
```

On first connection they see the register/login screen. On subsequent connections with the same key, the game launches directly.

### Testing SSH locally

If you don't have a remote server, you can test the full SSH flow on your own machine by SSHing to localhost. 
This requires a few extra steps.

Create the `plantroom` Linux user:

```
sudo useradd plantroom
sudo mkdir -p /home/plantroom
sudo chown plantroom:plantroom /home/plantroom
```

Make sure you have an SSH key pair. If not, generate one:

```
ssh-keygen -t ed25519
```

Install the script and config as described above, then start the server and SSH to yourself:

```
./build/plantroom-server &
ssh plantroom@localhost
```

## Game overview

**Coins** are earned passively at a rate determined by the plants currently in your room slots.
Coins accumulate while the app is open and offline progress is applied when you log back in.

**Buying plants** — go to the Shop tab and press Enter on a plant to buy it. It goes into your inventory.

**Placing plants** — go to the Room tab, select an empty slot, then select a plant from the inventory list that appears.
Plants grow through three stages: Seedling, Growing, Mature. Growth takes real time measured in seconds.

**Removing plants** — in the Room tab, select an occupied slot and press Enter to move the plant back to inventory. 
Accumulated growth time is preserved.

**Marketplace** — in the Market tab you can post a plant from your inventory with a price, or buy plants posted by other users.

**Quitting** — press `q` to save and exit cleanly. The save also happens automatically when the connection closes.

## Data files

The server reads from `data/` in its working directory (the build folder when running locally).

`data/plants.json` — the full species catalog with growth durations, income rates, and ASCII art per growth stage.

`data/shop.json` — list of species IDs available in the shop. Edit this to control what players can buy.

`data/auth.json` — stores usernames, hashed passwords, salts, and registered public keys. Created automatically on first registration.

`data/market.json` — persists marketplace listings across server restarts.

`data/saves/<username>.json` — one file per user with their coins, inventory, and room slots.
