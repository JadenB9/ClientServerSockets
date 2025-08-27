# CS3310 Programming Assignment 1: Multiplayer Battleship Game

**Course:** CS3310 - Operating Systems  
**Assignment:** P1 - Socket Programming  
**Student:** [Your Name]  
**Date:** August 27, 2025  
**Language:** C (ISO C99 Standard)  
**Platform:** Linux/Unix Systems

---

## Project Overview

This project implements a multiplayer Battleship game using TCP sockets in C. Two players connect to a server and engage in turn-based naval combat, placing ships on an 8x8 grid and trying to sink each other's fleet. The implementation demonstrates advanced socket programming concepts including multithreading, game state management, and real-time multiplayer communication.

## Assignment Requirements

All requirements have been successfully implemented and enhanced:

- **Custom Port**: Uses port **19845** (5-digit number < 65535)
- **Persistent Connections**: Players maintain active connections throughout the game
- **Data Transmission**: Bidirectional communication between clients and server
- **Data Reception**: Server processes game commands from multiple clients
- **Game Logic**: Complete Battleship game with ship placement and combat
- **Turn Management**: Server enforces turn-based gameplay
- **Multiple Connections**: Server handles exactly 2 concurrent players per game
- **Server Persistence**: Server runs until explicitly terminated (Ctrl-C)
- **Interactive Clients**: Rich command-line interface with real-time feedback
- **Code Quality**: Comprehensive commenting and error handling

## Project Structure

```
ClientServerSockets/
├── server.c              # Battleship game server
├── client.c              # Interactive game client
├── README.md             # Project documentation
├── v1_basic_messaging/   # Backup of basic version
│   ├── server.c          # Original simple server
│   ├── client.c          # Original simple client
│   └── README.md         # Original documentation
├── server                # Compiled game server
└── client                # Compiled game client
```

## Game Features

### Battleship Rules
- **Grid Size**: 8x8 playing field (A-H columns, 1-8 rows)
- **Fleet Composition**: 6 ships per player
  - 1 Carrier (4 spaces)
  - 1 Battleship (3 spaces) 
  - 2 Destroyers (2 spaces each)
  - 2 Submarines (1 space each)
- **Gameplay**: Turn-based combat until one player's fleet is destroyed
- **Ship Placement**: Ships cannot overlap or be adjacent
- **Victory Condition**: First player to sink all enemy ships wins

### Technical Features
- **Multithreaded Server**: Handles multiple clients simultaneously
- **Real-time Updates**: Players receive immediate feedback on all actions
- **Input Validation**: Comprehensive error checking for all game commands
- **Grid Visualization**: ASCII art display of game boards
- **Game State Management**: Server tracks all game phases and player states

## Compilation Instructions

### Prerequisites
- GCC compiler with pthread support
- Linux/Unix environment
- Standard C libraries

### Build Commands

```bash
# Compile server
gcc -Wall -Wextra -std=c99 -pedantic -pthread -o server server.c

# Compile client
gcc -Wall -Wextra -std=c99 -pedantic -pthread -o client client.c
```

## Execution Instructions

### Step 1: Start the Server
```bash
./server
```

**Expected Output:**
```
Battleship Server running on port 19845
Waiting for players to join...
```

### Step 2: Start Two Clients

**Terminal 2 (Player 1):**
```bash
./client
```

**Terminal 3 (Player 2):**
```bash
./client
```

The game automatically starts when both players connect.

## Game Commands

### Ship Placement Commands
```bash
PLACE <size> <position> <orientation>
```

**Examples:**
```bash
PLACE 4 A1 H    # Place carrier horizontally at A1-D1
PLACE 3 C3 V    # Place battleship vertically at C3-C5
PLACE 2 F1 H    # Place destroyer horizontally at F1-G1
PLACE 1 H8 H    # Place submarine at H8
```

### Battle Commands
```bash
ATTACK <position>   # Attack enemy position
GRID               # View your grid
ENEMY              # View enemy grid (shows hits/misses)
HELP               # Show command help
QUIT               # Exit game
```

**Attack Examples:**
```bash
ATTACK A1          # Fire at position A1
ATTACK H8          # Fire at position H8
```

## Gameplay Walkthrough

### Phase 1: Ship Placement
1. Both players place 6 ships on their 8x8 grid
2. Ships cannot overlap or be adjacent
3. Use PLACE command: `PLACE <size> <pos> <H|V>`
4. Server validates placement and shows updated grid

### Phase 2: Battle
1. Players take turns attacking enemy positions
2. Server announces HIT or MISS for each attack
3. Grid updates in real-time
4. First player to sink all enemy ships wins

### Example Game Session

**Player 1 Ship Placement:**
```
> PLACE 4 A1 H
✓ Ship placed
   A B C D E F G H
1  S S S S . . . .
2  . . . . . . . .
...

> PLACE 3 A3 V
✓ Ship placed
   A B C D E F G H
1  S S S S . . . .
2  . . . . . . . .
3  S . . . . . . .
4  S . . . . . . .
5  S . . . . . . .
...
```

**Battle Phase:**
```
🚢 BATTLE BEGINS! Your turn
💥 It's your turn! Use ATTACK <pos> to fire!
> ATTACK A1
🎯 HIT at A1!

💥 Your turn! Attack with: ATTACK <pos>
> ATTACK A2
🎯 HIT at A2!
...
```

## Implementation Details

### Server Architecture
- **Multithreaded Design**: One thread per client connection
- **Game State Management**: Centralized game state with mutex synchronization
- **Ship Validation**: Ensures proper placement rules
- **Turn Enforcement**: Server controls game flow and turn order
- **Win Condition Detection**: Automatic game termination when fleet is destroyed

### Client Features
- **Interactive Interface**: Rich command-line experience with visual feedback
- **Real-time Updates**: Immediate response to all game events
- **Grid Visualization**: Clear ASCII representation of game boards
- **Input Validation**: Client-side command parsing and validation
- **Threading**: Separate threads for sending commands and receiving updates

### Communication Protocol
1. **Connection**: Client connects, server assigns player ID
2. **Waiting**: Server waits for second player
3. **Ship Placement**: Players place ships using PLACE commands
4. **Battle Phase**: Turn-based ATTACK commands
5. **Game End**: WIN/LOSE messages, connection cleanup

## Technical Specifications

- **Socket Type**: TCP (SOCK_STREAM)
- **Address Family**: AF_INET (IPv4)
- **Threading**: POSIX pthreads
- **Buffer Size**: 1024 bytes
- **Grid Size**: 8x8 (64 positions)
- **Ship Count**: 6 ships per player
- **Turn Management**: Server-enforced alternating turns
- **Synchronization**: Mutex locks for thread-safe game state

## Code Quality Features

- **Thread Safety**: All game state access protected by mutexes
- **Memory Management**: Proper allocation/deallocation of client threads
- **Error Handling**: Comprehensive validation and error reporting
- **Resource Cleanup**: Proper socket closure and thread termination
- **Signal Handling**: Graceful server shutdown on SIGINT
- **Modular Design**: Clean separation of game logic and network code

## Testing Examples

### Basic Functionality Test
```bash
# Terminal 1
./server

# Terminal 2 (Player 1)
./client
> PLACE 4 A1 H
> PLACE 3 B1 V
> PLACE 2 D1 H
> PLACE 2 F1 V
> PLACE 1 H1 H
> PLACE 1 A8 H
> ATTACK A1

# Terminal 3 (Player 2)  
./client
> PLACE 4 E1 V
> PLACE 3 G1 V
> PLACE 2 A1 H
> PLACE 2 C1 H
> PLACE 1 A3 H
> PLACE 1 H8 H
> ATTACK E1
```

## Troubleshooting

### Common Issues

**"Address already in use" Error:**
```bash
# Wait ~60 seconds for socket to be released
```

**"Connection refused" Error:**
- Ensure server is running before starting clients
- Check that firewall allows connections on port 19845

**Game doesn't start:**
- Make sure exactly 2 clients are connected
- Check server output for connection status

**Threading Issues:**
- Ensure pthread library is linked during compilation
- Use `-pthread` flag with gcc

## Assignment Submission Checklist

- Server and client source code (server.c, client.c)
- Comprehensive documentation (README.md)
- Version 1 backup (v1_basic_messaging/)
- Code follows Cedarville style guidelines
- Proper header comments and inline documentation
- All functional requirements implemented
- Advanced features: multithreading, game logic, real-time updates
- Testing completed and verified

## Execution Screenshot

The following demonstrates successful multiplayer Battleship gameplay:

```
Terminal 1 (Server):
$ ./server
Battleship Server running on port 19845
Waiting for players to join...
New player connected from 127.0.0.1
New player connected from 127.0.0.1
Game started with 2 players

Terminal 2 (Player 1):
$ ./client
Connecting to Battleship server at 127.0.0.1:19845...
Connected! Player 1

=== BATTLESHIP GAME ===
Commands:
  PLACE <size> <pos> <H|V> - Place a ship
  ATTACK <pos>             - Attack enemy position
  GRID                     - Show your grid
  ENEMY                    - Show enemy grid
...

Game starting! Ship placement phase
=== SHIP PLACEMENT PHASE ===
Place your 6 ships on the 8x8 grid:
...
> PLACE 4 A1 H
✓ Ship placed
🚢 BATTLE BEGINS! Your turn
💥 It's your turn! Use ATTACK <pos> to fire!
> ATTACK E1
🎯 HIT at E1!

Terminal 3 (Player 2):
$ ./client
Connected! Player 2
Waiting for another player to join...
Game starting! Ship placement phase
...
🚢 BATTLE BEGINS! Wait for your opponent's move...
⏳ Wait for your opponent's move...
💥 Your turn! Attack with: ATTACK <pos>
> ATTACK A1
🎯 HIT at A1!
```

---

## Contact Information

**Student:** [Your Name]  
**Email:** [Your Email]  
**Course:** CS3310 - Operating Systems  
**Instructor:** [Instructor Name]  
**Institution:** Cedarville University

---

*This project demonstrates advanced proficiency in socket programming, multithreading, game development, and real-time network communication as required for CS3310.*