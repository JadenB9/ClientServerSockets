# CS3310 Programming Assignment 1: Mini Battleship Game

**Course:** CS3310 - Operating Systems  
**Assignment:** P1 - Socket Programming  
**Student:** [Your Name]  
**Date:** August 27, 2025  
**Language:** C (ISO C99 Standard)  
**Platform:** Linux/Unix Systems

---

## Project Overview

This project implements a **Mini Battleship** multiplayer game using TCP sockets in C. Two players connect to a server, choose usernames, and engage in simple naval combat on a **4x4 grid** with **one 2-space ship each**. The game features **colorful visual interface**, **emojis**, **real-time updates**, and **intuitive gameplay** that makes network programming concepts easy to understand and fun to experience.

## Game Features

### Simple & Visual Gameplay
- **4x4 Grid**: Small, easy-to-see playing field (A1-D4)
- **Single Ship**: Each player places one 2-space ship
- **Colorful Interface**: ANSI colors and emojis for enhanced visuals
- **Real-time Updates**: Instant feedback and grid displays
- **Username System**: Players choose custom names before joining

### Visual Elements
- 🚢 Ships displayed as colorful ship emojis
- 💥 Hits shown with explosion emojis
- 💧 Misses shown with water droplet emojis
- ⬜ Water displayed as blue squares
- ❔ Unknown enemy areas shown as question marks
- 🎯 Attack notifications with target emojis

### Game Rules
1. **Connect & Choose Username**: Players connect and pick unique usernames
2. **Ship Placement**: Place ONE 2-space ship horizontally (H) or vertically (V)
3. **Turn-based Combat**: Take turns attacking enemy positions
4. **Hit & Continue**: If you hit, you get another turn
5. **Miss & Switch**: If you miss, turn switches to opponent
6. **Victory**: First player to hit both ship spaces wins!

## Project Structure

```
ClientServerSockets/
├── server.c              # Mini Battleship game server
├── client.c              # Interactive visual game client
├── README.md             # This documentation
├── v1_basic_messaging/   # Backup of original simple version
│   ├── server.c          # Original basic server
│   ├── client.c          # Original basic client
│   └── README.md         # Original documentation
├── server                # Compiled game server
└── client                # Compiled game client
```

## Visual Game Experience

### Connection Screen
```
╔══════════════════════════════════════════════════════════════╗
║                    🚢 MINI BATTLESHIP 🚢                     ║
║                      4x4 Grid • 1 Ship                      ║
╚══════════════════════════════════════════════════════════════╝

🎉 Welcome to Mini Battleship! 🎉
Please enter your username (max 19 chars):
👤 Username: 
```

### Ship Placement Phase
```
┌─ SHIP PLACEMENT HELP ────────────────────────────────────┐
│ Your ship is 2 spaces long                               │
│                                                         │
│ Examples:                                                │
│   PLACE A1 H  →  🚢🚢⬜⬜  (horizontal at A1-B1)        │
│   PLACE C2 V  →  ⬜⬜🚢⬜  (vertical at C2-C3)          │
│                  ⬜⬜🚢⬜                                │
│                                                         │
│ Grid positions: A1, A2, A3, A4, B1, B2, B3, B4, etc.   │
└─────────────────────────────────────────────────────────┘
```

### Battle View (Side-by-Side Grids)
```
╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
║                                          🎯 BATTLE STATUS 🎯                                              ║
╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝

📋 YOUR GRID                              🎯 ENEMY GRID (OpponentName)
   (Shows your ship)                         (Shows your attacks)

     A   B   C   D              A   B   C   D   
  1  🚢 🚢 ⬜ ⬜        1  ❔ ❔ ❔ ❔ 
  2  ⬜ ⬜ ⬜ ⬜        2  ❔ 💧 ❔ ❔ 
  3  ⬜ ⬜ ⬜ ⬜        3  ❔ ❔ ❔ ❔ 
  4  ⬜ ⬜ ⬜ ⬜        4  ❔ ❔ ❔ ❔ 

Legend: ⬜=Water 🚢=Ship 💥=Hit 💧=Miss
```

## Compilation Instructions

### Prerequisites
- GCC compiler with pthread support
- Terminal with ANSI color support
- Linux/Unix environment

### Build Commands

```bash
# Compile server with threading support
gcc -Wall -Wextra -std=c99 -pedantic -pthread -o server server.c

# Compile client with threading support  
gcc -Wall -Wextra -std=c99 -pedantic -pthread -o client client.c
```

## Execution Instructions

### Step 1: Start the Server
```bash
./server
```

**Expected Output:**
```
🚢 Mini Battleship Server 🚢
Running on port 19845
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

The game automatically starts when both players have chosen usernames!

## Game Commands

### Username Phase
- Simply type your desired username and press Enter
- Usernames can be up to 19 characters long

### Ship Placement Commands
```bash
PLACE <position> <orientation>
```

**Examples:**
```bash
PLACE A1 H    # Place ship horizontally at A1-B1
PLACE C2 V    # Place ship vertically at C2-C3
PLACE D4 H    # Place ship horizontally at D4 (invalid - goes out of bounds!)
```

### Battle Commands
```bash
ATTACK <position>   # Attack enemy position
GRID               # Show both your grid and enemy grid
HELP               # Show command help
CLEAR              # Clear screen and show banner
QUIT               # Exit game
```

**Attack Examples:**
```bash
ATTACK A1          # Attack position A1
ATTACK C3          # Attack position C3
```

## Complete Gameplay Example

### Game Start
```
Player 1 connects:
👤 Username: Alice
⭐ Welcome, Alice! ⭐
Waiting for another player to join...

Player 2 connects:
👤 Username: Bob
⭐ Welcome, Bob! ⭐

🚢 Game Starting! 🚢
Alice vs Bob
Each player places ONE 2-space ship on a 4x4 grid.
Use: PLACE <pos> <H|V> (e.g., PLACE A1 H)
```

### Ship Placement
```
Alice: PLACE A1 H
✅ Ship placed successfully!

Bob: PLACE C2 V  
✅ Ship placed successfully!

⚔️ BATTLE BEGINS! ⚔️
Alice goes first!
```

### Battle Phase
```
Alice's Turn:
🎯 YOUR TURN! Attack with: ATTACK <pos>
> ATTACK B2
💧 MISS at B2 💧

Bob's Turn:
🎯 YOUR TURN! Attack with: ATTACK <pos>
> ATTACK A1
🎯 HIT at A1! 🎯
🔥 KEEP FIRING! You hit! Go again! Use ATTACK <pos>
> ATTACK B1
🎯 HIT at B1! 🎯

🎉 VICTORY! You sunk their ship! 🎉
╔══════════════════════════════════════════════════════════════╗
║                        🎉 VICTORY! 🎉                        ║
║                   You sunk their ship!                       ║
╚══════════════════════════════════════════════════════════════╝
```

## Technical Implementation

### Server Features
- **Multithreaded**: One thread per client connection
- **Username Management**: Validates and stores player names
- **Game State Machine**: Tracks connection → username → placement → battle → game over
- **Visual Grid Generation**: Creates colorful ASCII art grids with emojis
- **Turn Management**: Enforces proper turn order and hit/miss rules
- **Broadcast Messaging**: Sends updates to both players simultaneously

### Client Features  
- **Screen Management**: Clear screen and redraw for clean visuals
- **Color Support**: Full ANSI color and emoji rendering
- **Interactive Prompts**: Context-aware prompts (username vs game commands)
- **Real-time Updates**: Separate thread for receiving server messages
- **Input Processing**: Smart command parsing and case conversion
- **Visual Feedback**: Immediate response to all game events

### Communication Protocol
1. **Connection**: Client connects, server requests username
2. **Username**: Client sends name, server confirms and waits for second player  
3. **Game Start**: Server broadcasts game start when both players ready
4. **Ship Placement**: Players send PLACE commands, server validates and confirms
5. **Battle Phase**: Turn-based ATTACK commands with immediate hit/miss feedback
6. **Victory**: Server announces winner and ends game

## Technical Specifications

- **Socket Type**: TCP (SOCK_STREAM) for reliable communication
- **Grid Size**: 4x4 (16 total positions)
- **Ship Size**: 2 spaces (horizontal or vertical)
- **Port**: 19845
- **Threading**: POSIX pthreads for concurrent client handling  
- **Colors**: ANSI escape codes for terminal colors
- **Emojis**: Unicode emoji characters for visual elements
- **Buffer Size**: 4096 bytes for large visual grid data
- **Username Limit**: 19 characters maximum

## Why This Design?

### Educational Benefits
- **Simple Rules**: Easy to understand game mechanics
- **Visual Feedback**: Immediate understanding of network communication
- **Small Scale**: 4x4 grid makes state changes obvious
- **Quick Games**: Matches finish in 2-8 moves typically

### Technical Learning
- **Socket Programming**: TCP client-server architecture
- **Multithreading**: Concurrent client handling with thread safety
- **Protocol Design**: Custom message-based communication
- **State Management**: Game state machines and synchronization
- **User Interface**: Terminal-based visual programming

## Assignment Requirements Fulfilled

- **Custom Port**: Uses port 19845 (5-digit number < 65535) ✓
- **Persistent Connections**: Players stay connected throughout entire game ✓  
- **Bidirectional Communication**: Real-time client-server messaging ✓
- **Multiple Clients**: Server handles exactly 2 concurrent players ✓
- **Interactive Interface**: Rich visual command-line experience ✓
- **Game Logic**: Complete turn-based gameplay with win conditions ✓
- **Error Handling**: Comprehensive input validation and error messages ✓
- **Threading**: Safe concurrent programming with mutexes ✓
- **Code Quality**: Clean, well-documented, and properly structured ✓

## Enhanced Features Beyond Requirements

- **Visual Interface**: Colorful grids with emojis and ANSI art
- **Username System**: Personalized player identification  
- **Screen Management**: Clear screen redraws for clean presentation
- **Smart Input**: Case-insensitive commands and input validation
- **Game Flow**: Intuitive progression from connection to victory
- **Broadcast Updates**: Real-time notifications to all players
- **Help System**: Built-in command reference and game rules
- **Graceful Shutdown**: Clean connection termination and resource cleanup

## Troubleshooting

### Common Issues

**Colors/Emojis not displaying:**
- Ensure your terminal supports ANSI colors and Unicode
- Try a modern terminal like Terminal.app, iTerm2, or GNOME Terminal

**"Address already in use" Error:**
```bash
# Kill any existing server process
lsof -ti:19845 | xargs kill
# Or wait 60 seconds for socket cleanup
```

**Connection refused:**
- Make sure server is running before starting clients
- Check firewall settings for port 19845

**Game doesn't start:**
- Ensure both players have entered usernames
- Check server output for connection status

**Threading issues:**
- Compile with `-pthread` flag
- Ensure POSIX threads are available on your system

## Testing Examples

### Quick Test Game
```bash
# Terminal 1: Start server
./server

# Terminal 2: Player 1
./client
# Username: TestPlayer1  
# PLACE A1 H
# ATTACK C1

# Terminal 3: Player 2  
./client
# Username: TestPlayer2
# PLACE C1 V
# ATTACK A1
```

### Visual Test
```bash
# Test visual elements
./client
# Try: HELP, CLEAR, GRID commands
# Notice colors and emoji rendering
```

## Assignment Submission Checklist

- Server and client source code (server.c, client.c) ✓
- Enhanced visual interface with colors and emojis ✓
- Username system implementation ✓
- Simplified 4x4 grid gameplay ✓
- Comprehensive documentation (README.md) ✓
- Version history preservation (v1_basic_messaging/) ✓
- Code follows Cedarville style guidelines ✓
- Proper header comments and documentation ✓
- Advanced features: threading, visuals, user experience ✓
- Testing completed and verified ✓

---

## Contact Information

**Student:** [Your Name]  
**Email:** [Your Email]  
**Course:** CS3310 - Operating Systems  
**Instructor:** [Instructor Name]  
**Institution:** Cedarville University

---

*This project demonstrates advanced socket programming, multithreading, visual interface design, and user experience principles while maintaining educational clarity and engagement.*