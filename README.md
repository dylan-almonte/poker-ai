# Poker AI Server-Client Model

This repository contains a poker game with a server-client model. The server is implemented in C++ and the client is implemented in Go.

## Components

- **C++ Poker Engine Server**: The main poker game logic runs on the server side, handling game state, card dealing, betting rounds, and determining winners.
- **Go Client**: A command-line client for players to connect to the server and play the game.

## Directory Structure

```
poker-ai/
├── src/                  # Main source code
│   ├── engine/           # C++ engine code
│   │   ├── include/      # Engine include files
│   │   └── src/          # Engine source files
│   ├── server/           # Server implementation
│   │   ├── include/      # Header files
│   │   └── src/          # Source files
│   ├── client/           # Client implementations
│   │   └── go/           # Go client implementation
│   │       ├── poker_client.go
│   │       └── go.mod
│   ├── ai/               # AI implementations
│   └── include/          # Shared include files
├── test/                 # Test files
│   ├── engine/           # Engine tests
│   └── CMakeLists.txt    # Test build configuration
└── CMakeLists.txt        # Main build configuration
```

## Building the Server

The server is built using CMake:

```bash
# Navigate to the engine directory
cd poker-ai/engine

# Create a build directory
mkdir -p build && cd build

# Configure CMake
cmake ..

# Build
make

# The server binary will be located at engine/interface/server/bin/poker_server_main
```

## Running the Server

After building, you can run the poker server:

```bash
# Navigate to the engine directory
cd poker-ai/engine

# Run the poker server
./interface/server/bin/poker_server_main [port] [min_players] [max_players] [starting_chips]
```

Parameters:
- `port`: The port to listen on (default: 8080)
- `min_players`: Minimum number of players required to start a game (default: 2)
- `max_players`: Maximum number of players allowed in a game (default: 9)
- `starting_chips`: Number of chips each player starts with (default: 1000)

## Building and Running the Go Client

```bash
# Navigate to the go client directory
cd poker-ai/clients/go

# Build the client
go build -o poker_client poker_client.go

# Run the client
./poker_client <player_name> [server_ip] [port]
```

Parameters:
- `player_name`: Your player name (required)
- `server_ip`: IP address of the server (default: 127.0.0.1)
- `port`: Port of the server (default: 8080)

## Client Commands

Once connected to the server, you can use the following commands in the Go client:

- `ready` - Set yourself as ready to play
- `notready` - Set yourself as not ready
- `fold` - Fold your hand
- `check` - Check
- `call` - Call the current bet
- `raise <amount>` - Raise by the specified amount
- `quit` - Disconnect and exit

## Protocol

The server and client communicate using JSON messages over TCP. Here are some examples of the messages:

### Server to Client Messages

1. Welcome message (when client connects):
```json
{"type":"welcome","player_id":1}
```

2. Game state update:
```json
{"type":"game_state","stage":2,"current_player":1,"dealer_position":0,"players":[{"id":1,"name":"Player1","chips":1000,"is_connected":true,"is_active":true,"is_ready":true}],"pot":100}
```

3. Private cards:
```json
{"type":"private_cards","cards":["AH","KS"]}
```

4. Public cards:
```json
{"type":"public_cards","cards":["2H","7C","TD","AS","QH"]}
```

### Client to Server Messages

1. Register player:
```json
{"type":"register","name":"Player1"}
```

2. Ready state:
```json
{"type":"ready","ready":true}
```

3. Player action:
```json
{"type":"action","action":"call"}
```

4. Player raise:
```json
{"type":"action","action":"raise","amount":50}
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

# Poker AI with Deep CFR

A poker AI implementation using Deep Counterfactual Regret Minimization (Deep CFR) with a custom neural network library.

## Goals

- Implement a poker engine
- Implement a poker AI using Deep CFR

I want to make this as modular as possible so that I can experiment with different architectures and algorithms, then test them in the poker engine against each other.

I also wanted to learn more about poker theory and deep learning in general, so that's the main motivation for this project. 

## Research
Please refer to [paper](papers/) for papers that are referenced
Useful [website](https://www.cs.cmu.edu/~sandholm/cs15-888F24/)

### TODO

- [ ] Fix bug where all players go all in at the same time
- [ ] Make general game interface for AI to use
- [ ] Make an engine to play against the AI
- [ ] Adjust game engine to be thread safe

# Contributing

to start
`git clone https://github.com/dylan-almonte/poker-ai`

Make a branch to start developing
`git checkout -b <branchname>`

https://github.com/charmbracelet/bubbletea
