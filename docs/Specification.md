
# Specification

## Theme: Terminal-based interactive application

### User interaction

User will interact with the program by text commands/selections(for example moving with arrows and pressing enter for choosing).

User will get real time visible updates in a single session, so for example inventory updates, balance changes and so on.

### Functionality and program discription

After connecting via ssh and sending their public key the user is able to:

- Interact with the app

- Make some actions that persist across sessions

- Earn fake in-app currency

- Spend currency to obtain items

- View obtained items

### Core features:

- terminal UI

- Automatic user session start via SSH

- Persistent user data

- Inventory management

### Probable future expansion

- Multi-user support (simultaneous SSH connections)

- Shared “global state”

## Architecture (not final version)

- SSH wrapper layer

- main application

- user database (json)

- item catalog

- session manager

## Third party libraries

- nlohmann for working with JSON

- FTXUI - for terminal UI (or fmt/cpp-terminal as a fall-back, but preferably FTXUI)

- Cmake

Everything is portable and works on any OS that supports SSH and a C++ runtime

## SSH connection

For now the idea is:

1) User generates an SSH key (if they don’t already have one)

2) User connects via ssh gameuser@your-server-ip

3) They paste their public key

4) On my computer their key is saved into authorized_keys with the command that restricts them to only entering my app

5) They connect via ssh myapp

6) The user can interact with the app