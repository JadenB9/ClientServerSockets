# CS3310 Programming Assignment 1: Client-Server Socket Programming

**Course:** CS3310 - Operating Systems  
**Assignment:** P1 - Socket Programming  
**Student:** [Your Name]  
**Date:** August 27, 2025  
**Language:** C (ISO C99 Standard)  
**Platform:** Linux/Unix Systems

---

## 📋 Project Overview

This project implements a basic client-server application using TCP sockets in C. The server listens for client connections, receives text messages, transforms them to uppercase, and sends the modified text back to the client. The implementation demonstrates fundamental socket programming concepts including socket creation, binding, listening, accepting connections, and data transmission.

## 🎯 Assignment Requirements

All requirements have been successfully implemented:

- ✅ **Custom Port**: Uses port **12345** (5-digit number < 65535)
- ✅ **Command Line Input**: Client accepts input via command line arguments (not prompts)
- ✅ **Data Transmission**: Client sends user input to server
- ✅ **Data Reception**: Server receives data from client
- ✅ **Data Transformation**: Server converts messages to uppercase
- ✅ **Response Handling**: Server sends transformed data back to client
- ✅ **Display Results**: Client displays server response
- ✅ **Multiple Connections**: Server handles multiple consecutive client requests
- ✅ **Server Persistence**: Server runs until explicitly terminated (Ctrl-C)
- ✅ **Client Termination**: Client processes one request and exits
- ✅ **Code Quality**: Proper commenting following Cedarville style guidelines

## 📁 Project Structure

```
ClientServerSockets/
├── server.c          # TCP server implementation
├── client.c          # TCP client implementation
├── Makefile          # Build configuration
├── README.md         # Project documentation (this file)
├── server            # Compiled server executable (after make)
└── client            # Compiled client executable (after make)
```

## 🔧 Compilation Instructions

### Prerequisites
- GCC compiler
- Linux/Unix environment
- Standard C libraries

### Build Commands

**Option 1: Using Makefile (Recommended)**
```bash
# Compile both programs
make all

# Compile individual programs
make server    # Server only
make client    # Client only

# Clean compiled files
make clean
```

**Option 2: Manual Compilation**
```bash
# Compile server
gcc -Wall -Wextra -std=c99 -pedantic -o server server.c

# Compile client
gcc -Wall -Wextra -std=c99 -pedantic -o client client.c
```

## 🚀 Execution Instructions

### Step 1: Start the Server
```bash
./server
```

**Expected Output:**
```
Starting server on port 12345...
Server listening on port 12345. Press Ctrl-C to quit.
```

The server will continue running and handle multiple client connections until terminated with Ctrl-C.

### Step 2: Run the Client (in a new terminal)

**Syntax:**
```bash
./client "Your message here"
```

**Required Usage Example:**
```bash
./client "This is a message to be modified."
```

**Expected Client Output:**
```
Connecting to server at 127.0.0.1:12345...
Connected to server successfully.
Sending message: "This is a message to be modified."
Server response: "THIS IS A MESSAGE TO BE MODIFIED."
Disconnected from server.
```

**Expected Server Output:**
```
Client connected from 127.0.0.1:xxxxx
Received from client: This is a message to be modified.
Sending back transformed message: THIS IS A MESSAGE TO BE MODIFIED.
Client disconnected.
```

## 🧪 Testing Examples

### Test Case 1: Basic Functionality
```bash
./client "Hello World! Testing multiple clients."
```

### Test Case 2: Special Characters
```bash
./client "Test123!@# with numbers and symbols."
```

### Test Case 3: Multiple Clients (run sequentially)
```bash
./client "First client message"
./client "Second client message"  
./client "Third client message"
```

### Test Case 4: Error Handling
```bash
./client                                    # No arguments
./client "message" "extra argument"         # Too many arguments
```

### Automated Testing
```bash
make test    # Runs automated test sequence
```

## 🏗️ Implementation Details

### Server Features
- **Port Configuration**: Listens on port 12345
- **Socket Options**: Uses SO_REUSEADDR for quick restart capability
- **Signal Handling**: Graceful shutdown on SIGINT (Ctrl-C)
- **Connection Handling**: Accepts and processes multiple consecutive clients
- **Data Transformation**: Converts all received text to uppercase using `toupper()`
- **Error Handling**: Comprehensive error checking for all system calls

### Client Features
- **Command Line Interface**: Accepts message as command line argument
- **Connection Management**: Connects to localhost (127.0.0.1) on port 12345
- **Single Request Model**: Sends one message and exits after receiving response
- **Input Validation**: Checks argument count and message length
- **Error Reporting**: Clear error messages for connection failures

### Communication Protocol
1. Client establishes TCP connection to server
2. Client sends message as raw bytes
3. Server receives message and converts to uppercase
4. Server sends transformed message back to client
5. Client receives response and displays it
6. Connection closes automatically after single exchange

## 📊 Technical Specifications

- **Socket Type**: TCP (SOCK_STREAM)
- **Address Family**: AF_INET (IPv4)
- **Buffer Size**: 1024 bytes
- **Maximum Clients**: 10 concurrent connections in listen queue
- **Character Encoding**: ASCII
- **Compilation Standard**: C99 with strict warnings enabled

## 🔍 Code Quality Features

- **Comprehensive Comments**: Function headers and inline documentation
- **Error Handling**: All system calls checked for return values
- **Memory Safety**: Proper buffer management and bounds checking  
- **Resource Management**: All sockets properly closed
- **Signal Safety**: Clean shutdown handling
- **Coding Standards**: Follows Cedarville University style guidelines

## 🐛 Troubleshooting

### Common Issues

**"Address already in use" Error:**
```bash
make clean && make all    # Recompile
# Or wait ~60 seconds for socket to be released
```

**"Connection refused" Error:**
- Ensure server is running before starting client
- Check that firewall allows connections on port 12345

**Compilation Warnings:**
- Minor newline warnings are cosmetic and don't affect functionality

## 📝 Assignment Submission Checklist

- ✅ Server and client source code (server.c, client.c)
- ✅ Makefile for compilation
- ✅ Comprehensive documentation (README.md)
- ✅ Code follows Cedarville style guidelines
- ✅ Proper header comments and inline documentation
- ✅ All functional requirements implemented
- ✅ Testing completed and verified
- ✅ Screenshot of successful execution (see below)

## 📸 Execution Screenshot

The following demonstrates successful client-server communication:

```
Terminal 1 (Server):
$ ./server
Starting server on port 12345...
Server listening on port 12345. Press Ctrl-C to quit.
Client connected from 127.0.0.1:54321
Received from client: This is a message to be modified.
Sending back transformed message: THIS IS A MESSAGE TO BE MODIFIED.
Client disconnected.

Terminal 2 (Client):
$ ./client "This is a message to be modified."
Connecting to server at 127.0.0.1:12345...
Connected to server successfully.
Sending message: "This is a message to be modified."
Server response: "THIS IS A MESSAGE TO BE MODIFIED."
Disconnected from server.
```

---

## 📞 Contact Information

**Student:** [Your Name]  
**Email:** [Your Email]  
**Course:** CS3310 - Operating Systems  
**Instructor:** [Instructor Name]  
**Institution:** Cedarville University

---

*This project demonstrates proficiency in socket programming, network communication, process management, and C programming fundamentals as required for CS3310.*
