/*
 * File: server.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: Mini Battleship Game Server (4x4 grid, single 2x1 ship)
 *              Simple multiplayer naval combat with usernames and visual interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define PORT 19845
#define BUFFER_SIZE 1024
#define GRID_SIZE 4
#define SHIP_SIZE 2
#define MAX_USERNAME 20

// Cell states
typedef enum {
    EMPTY = 0,
    SHIP = 1,
    HIT = 2,
    MISS = 3
} cell_state_t;

// Game states
typedef enum {
    WAITING_FOR_PLAYERS,
    WAITING_FOR_USERNAMES,
    PLACING_SHIPS,
    PLAYING,
    GAME_OVER
} game_state_t;

// Player structure
typedef struct {
    int socket;
    int player_id;
    char username[MAX_USERNAME];
    int has_username;
    cell_state_t grid[GRID_SIZE][GRID_SIZE];
    cell_state_t enemy_view[GRID_SIZE][GRID_SIZE];
    int ship_placed;
    int ship_hits;
} player_t;

// Game structure
typedef struct {
    player_t players[2];
    int current_player;
    game_state_t state;
    int players_connected;
} game_t;

// Global variables
int listen_fd = -1;
game_t game;
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

// ANSI color codes
const char* RESET = "\033[0m";
const char* BOLD = "\033[1m";
const char* RED = "\033[91m";
const char* GREEN = "\033[92m";
const char* YELLOW = "\033[93m";
const char* BLUE = "\033[94m";
const char* MAGENTA = "\033[95m";
const char* CYAN = "\033[96m";
const char* WHITE = "\033[97m";

void signal_handler(int sig) {
    (void)sig;  // Suppress unused parameter warning
    printf("\n%s%s🛑 Shutting down server...%s\n", BOLD, RED, RESET);
    if (listen_fd != -1) {
        close(listen_fd);
    }
    exit(0);
}

void init_game(void) {
    memset(&game, 0, sizeof(game_t));
    game.state = WAITING_FOR_PLAYERS;
    game.current_player = 0;
    
    for (int p = 0; p < 2; p++) {
        game.players[p].socket = -1;
        game.players[p].player_id = p;
        game.players[p].has_username = 0;
        game.players[p].ship_placed = 0;
        game.players[p].ship_hits = 0;
        strcpy(game.players[p].username, "");
        
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                game.players[p].grid[i][j] = EMPTY;
                game.players[p].enemy_view[i][j] = EMPTY;
            }
        }
    }
}

void send_message(int socket, const char* message) {
    send(socket, message, strlen(message), 0);
}

void send_colorful_grid(int socket, cell_state_t grid[GRID_SIZE][GRID_SIZE], int show_ships, const char* title) {
    char buffer[2048];
    char grid_str[1024] = "";
    
    snprintf(grid_str, sizeof(grid_str), 
        "%s%s╔════════════════════════════════════════════════╗%s\n"
        "%s%s║                    %s%-20s%s%s║%s\n"
        "%s%s╚════════════════════════════════════════════════╝%s\n\n",
        BOLD, CYAN, RESET,
        BOLD, CYAN, WHITE, title, CYAN, BOLD, RESET,
        BOLD, CYAN, RESET);
    
    // Column headers
    strcat(grid_str, "     ");
    for (int j = 0; j < GRID_SIZE; j++) {
        char col_header[16];
        snprintf(col_header, sizeof(col_header), "%s%s%c%s   ", BOLD, YELLOW, 'A' + j, RESET);
        strcat(grid_str, col_header);
    }
    strcat(grid_str, "\n\n");
    
    // Grid rows with fancy borders
    for (int i = 0; i < GRID_SIZE; i++) {
        char row[256];
        snprintf(row, sizeof(row), "  %s%s%d%s  ", BOLD, YELLOW, i + 1, RESET);
        strcat(grid_str, row);
        
        for (int j = 0; j < GRID_SIZE; j++) {
            char cell[32];
            switch (grid[i][j]) {
                case EMPTY:
                    snprintf(cell, sizeof(cell), "%s%s⬜%s ", BOLD, BLUE, RESET);
                    break;
                case SHIP:
                    if (show_ships) {
                        snprintf(cell, sizeof(cell), "%s%s🚢%s ", BOLD, GREEN, RESET);
                    } else {
                        snprintf(cell, sizeof(cell), "%s%s⬜%s ", BOLD, BLUE, RESET);
                    }
                    break;
                case HIT:
                    snprintf(cell, sizeof(cell), "%s%s💥%s ", BOLD, RED, RESET);
                    break;
                case MISS:
                    snprintf(cell, sizeof(cell), "%s%s💧%s ", BOLD, WHITE, RESET);
                    break;
            }
            strcat(grid_str, cell);
        }
        strcat(grid_str, "\n");
    }
    
    strcat(grid_str, "\n");
    strcat(grid_str, "Legend: ");
    char legend[256];
    snprintf(legend, sizeof(legend), 
        "%s⬜%s=Water %s🚢%s=Ship %s💥%s=Hit %s💧%s=Miss\n\n",
        BLUE, RESET, GREEN, RESET, RED, RESET, WHITE, RESET);
    strcat(grid_str, legend);
    
    snprintf(buffer, sizeof(buffer), "GRID\n%s", grid_str);
    send_message(socket, buffer);
}

void send_both_grids(int player_id) {
    player_t* player = &game.players[player_id];
    int enemy_id = 1 - player_id;
    
    char combined[4096];
    snprintf(combined, sizeof(combined),
        "%s%s╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗%s\n"
        "%s%s║                                          %s🎯 BATTLE STATUS 🎯%s                                              %s║%s\n"
        "%s%s╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝%s\n\n"
        "%s%s📋 YOUR GRID%s                              %s🎯 ENEMY GRID (%s)%s\n"
        "   (Shows your ship)                         (Shows your attacks)\n\n",
        BOLD, MAGENTA, RESET,
        BOLD, MAGENTA, WHITE, MAGENTA, BOLD, RESET,
        BOLD, MAGENTA, RESET,
        BOLD, GREEN, RESET, BOLD, game.players[enemy_id].username, RESET);
    
    // Your grid (left side)
    strcat(combined, "     ");
    for (int j = 0; j < GRID_SIZE; j++) {
        char col_header[16];
        snprintf(col_header, sizeof(col_header), "%s%s%c%s   ", BOLD, YELLOW, 'A' + j, RESET);
        strcat(combined, col_header);
    }
    
    // Enemy grid headers (right side)
    strcat(combined, "           ");
    for (int j = 0; j < GRID_SIZE; j++) {
        char col_header[16];
        snprintf(col_header, sizeof(col_header), "%s%s%c%s   ", BOLD, YELLOW, 'A' + j, RESET);
        strcat(combined, col_header);
    }
    strcat(combined, "\n\n");
    
    // Both grids side by side
    for (int i = 0; i < GRID_SIZE; i++) {
        char row[512];
        snprintf(row, sizeof(row), "  %s%s%d%s  ", BOLD, YELLOW, i + 1, RESET);
        strcat(combined, row);
        
        // Your grid
        for (int j = 0; j < GRID_SIZE; j++) {
            char cell[32];
            switch (player->grid[i][j]) {
                case EMPTY:
                    snprintf(cell, sizeof(cell), "%s%s⬜%s ", BOLD, BLUE, RESET);
                    break;
                case SHIP:
                    snprintf(cell, sizeof(cell), "%s%s🚢%s ", BOLD, GREEN, RESET);
                    break;
                case HIT:
                    snprintf(cell, sizeof(cell), "%s%s💥%s ", BOLD, RED, RESET);
                    break;
                case MISS:
                    snprintf(cell, sizeof(cell), "%s%s💧%s ", BOLD, WHITE, RESET);
                    break;
            }
            strcat(combined, cell);
        }
        
        // Space between grids
        snprintf(row, sizeof(row), "        %s%s%d%s  ", BOLD, YELLOW, i + 1, RESET);
        strcat(combined, row);
        
        // Enemy view
        for (int j = 0; j < GRID_SIZE; j++) {
            char cell[32];
            switch (player->enemy_view[i][j]) {
                case EMPTY:
                    snprintf(cell, sizeof(cell), "%s%s❔%s ", BOLD, MAGENTA, RESET);
                    break;
                case HIT:
                    snprintf(cell, sizeof(cell), "%s%s💥%s ", BOLD, RED, RESET);
                    break;
                case MISS:
                    snprintf(cell, sizeof(cell), "%s%s💧%s ", BOLD, WHITE, RESET);
                    break;
                default:
                    snprintf(cell, sizeof(cell), "%s%s❔%s ", BOLD, MAGENTA, RESET);
                    break;
            }
            strcat(combined, cell);
        }
        strcat(combined, "\n");
    }
    
    strcat(combined, "\n");
    
    char buffer[5120];
    snprintf(buffer, sizeof(buffer), "BOTH_GRIDS\n%s", combined);
    send_message(player->socket, buffer);
}

int validate_ship_placement(int player_id, int row, int col, int horizontal) {
    player_t* player = &game.players[player_id];
    
    // Check bounds
    if (horizontal) {
        if (col + SHIP_SIZE > GRID_SIZE) return 0;
    } else {
        if (row + SHIP_SIZE > GRID_SIZE) return 0;
    }
    
    // Check for overlaps
    for (int i = 0; i < SHIP_SIZE; i++) {
        int r = row + (horizontal ? 0 : i);
        int c = col + (horizontal ? i : 0);
        if (player->grid[r][c] != EMPTY) return 0;
    }
    
    return 1;
}

void place_ship(int player_id, int row, int col, int horizontal) {
    player_t* player = &game.players[player_id];
    
    for (int i = 0; i < SHIP_SIZE; i++) {
        int r = row + (horizontal ? 0 : i);
        int c = col + (horizontal ? i : 0);
        player->grid[r][c] = SHIP;
    }
    
    player->ship_placed = 1;
}

int process_attack(int attacker_id, int row, int col) {
    int defender_id = 1 - attacker_id;
    player_t* attacker = &game.players[attacker_id];
    player_t* defender = &game.players[defender_id];
    
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return -1;
    if (attacker->enemy_view[row][col] != EMPTY) return -1; // Already attacked
    
    if (defender->grid[row][col] == SHIP) {
        defender->grid[row][col] = HIT;
        attacker->enemy_view[row][col] = HIT;
        defender->ship_hits++;
        
        if (defender->ship_hits >= SHIP_SIZE) {
            return 2; // Ship sunk (game over)
        }
        return 1; // Hit
    } else {
        defender->grid[row][col] = MISS;
        attacker->enemy_view[row][col] = MISS;
        return 0; // Miss
    }
}

void broadcast_message(const char* message) {
    for (int i = 0; i < 2; i++) {
        if (game.players[i].socket != -1) {
            send_message(game.players[i].socket, message);
        }
    }
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int player_id = -1;
    
    pthread_mutex_lock(&game_mutex);
    
    if (game.players_connected >= 2) {
        send_message(client_socket, "ERROR Game is full\n");
        pthread_mutex_unlock(&game_mutex);
        close(client_socket);
        return NULL;
    }
    
    player_id = game.players_connected;
    game.players[player_id].socket = client_socket;
    game.players_connected++;
    
    char welcome_msg[256];
    snprintf(welcome_msg, sizeof(welcome_msg), 
        "WELCOME %s%s🎉 Welcome to Mini Battleship! 🎉%s\nPlease enter your username (max %d chars):\n", 
        BOLD, GREEN, RESET, MAX_USERNAME - 1);
    send_message(client_socket, welcome_msg);
    
    pthread_mutex_unlock(&game_mutex);
    
    while (1) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        
        buffer[bytes_received] = '\0';
        // Remove newline
        char* newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        printf("Player %d: %s\n", player_id + 1, buffer);
        
        pthread_mutex_lock(&game_mutex);
        
        // Handle username input
        if (!game.players[player_id].has_username && strlen(buffer) > 0) {
            strncpy(game.players[player_id].username, buffer, MAX_USERNAME - 1);
            game.players[player_id].username[MAX_USERNAME - 1] = '\0';
            game.players[player_id].has_username = 1;
            
            char user_confirm[256];
            snprintf(user_confirm, sizeof(user_confirm), 
                "USERNAME_SET %s%s⭐ Welcome, %s! ⭐%s\n", 
                BOLD, CYAN, game.players[player_id].username, RESET);
            send_message(client_socket, user_confirm);
            
            // Check if both players have usernames
            if (game.players[0].has_username && game.players[1].has_username && game.players_connected == 2) {
                game.state = PLACING_SHIPS;
                char start_msg[512];
                snprintf(start_msg, sizeof(start_msg),
                    "GAME_START %s%s🚢 Game Starting! 🚢%s\n"
                    "%s vs %s\n"
                    "Each player places ONE 2-space ship on a 4x4 grid.\n"
                    "Use: PLACE <pos> <H|V> (e.g., PLACE A1 H)\n",
                    BOLD, MAGENTA, RESET,
                    game.players[0].username, game.players[1].username);
                broadcast_message(start_msg);
                printf("Game started: %s vs %s\n", 
                    game.players[0].username, game.players[1].username);
            } else if (game.players_connected == 1) {
                // First player - show waiting message
                char waiting_msg[256];
                snprintf(waiting_msg, sizeof(waiting_msg),
                    "WAIT_PLAYER %s%sWaiting for another player to join...%s\n",
                    BOLD, YELLOW, RESET);
                send_message(client_socket, waiting_msg);
            }
            
            pthread_mutex_unlock(&game_mutex);
            continue;
        }
        
        char command[16], args[256];
        sscanf(buffer, "%s %[^\n]", command, args);
        
        if (strcmp(command, "PLACE") == 0) {
            if (game.state != PLACING_SHIPS) {
                send_message(client_socket, "ERROR Not in ship placement phase\n");
            } else if (game.players[player_id].ship_placed) {
                send_message(client_socket, "ERROR Ship already placed\n");
            } else {
                char pos[4], orientation[16];
                if (sscanf(args, "%s %s", pos, orientation) == 2) {
                    int col = pos[0] - 'A';
                    int row = pos[1] - '1';
                    int horizontal = (strcmp(orientation, "H") == 0);
                    
                    if (validate_ship_placement(player_id, row, col, horizontal)) {
                        place_ship(player_id, row, col, horizontal);
                        char success_msg[256];
                        snprintf(success_msg, sizeof(success_msg),
                            "SHIP_PLACED %s%s✅ Ship placed successfully!%s\n",
                            BOLD, GREEN, RESET);
                        send_message(client_socket, success_msg);
                        
                        send_colorful_grid(client_socket, game.players[player_id].grid, 1, "YOUR GRID");
                        
                        if (game.players[0].ship_placed && game.players[1].ship_placed) {
                            game.state = PLAYING;
                            char battle_msg[512];
                            snprintf(battle_msg, sizeof(battle_msg),
                                "BATTLE_START %s%s⚔️ BATTLE BEGINS! ⚔️%s\n"
                                "%s goes first!\n",
                                BOLD, RED, RESET, game.players[0].username);
                            broadcast_message(battle_msg);
                            
                            send_message(game.players[0].socket, "YOUR_TURN It's your turn! Use ATTACK <pos>\n");
                            send_message(game.players[1].socket, "WAIT_TURN Wait for your opponent's move...\n");
                        }
                    } else {
                        send_message(client_socket, "ERROR Invalid ship placement\n");
                    }
                } else {
                    send_message(client_socket, "ERROR Invalid format. Use: PLACE <pos> <H|V>\n");
                }
            }
        } else if (strcmp(command, "ATTACK") == 0) {
            if (game.state != PLAYING) {
                send_message(client_socket, "ERROR Not in battle phase\n");
            } else if (game.current_player != player_id) {
                send_message(client_socket, "ERROR Not your turn\n");
            } else {
                char pos[4];
                if (sscanf(args, "%s", pos) == 1) {
                    int col = pos[0] - 'A';
                    int row = pos[1] - '1';
                    
                    int result = process_attack(player_id, row, col);
                    if (result == -1) {
                        send_message(client_socket, "ERROR Invalid attack\n");
                    } else {
                        char result_msg[512];
                        char broadcast_msg[512];
                        
                        if (result == 2) { // Ship sunk - game over
                            snprintf(result_msg, sizeof(result_msg),
                                "WIN %s%s🎉 VICTORY! You sunk their ship! 🎉%s\n",
                                BOLD, GREEN, RESET);
                            send_message(client_socket, result_msg);
                            
                            snprintf(result_msg, sizeof(result_msg),
                                "LOSE %s%s💀 DEFEAT! Your ship was sunk! 💀%s\n",
                                BOLD, RED, RESET);
                            send_message(game.players[1 - player_id].socket, result_msg);
                            
                            snprintf(broadcast_msg, sizeof(broadcast_msg),
                                "GAME_OVER %s%s🏆 Game Over! %s wins! 🏆%s\n",
                                BOLD, YELLOW, game.players[player_id].username, RESET);
                            broadcast_message(broadcast_msg);
                            
                            game.state = GAME_OVER;
                        } else if (result == 1) { // Hit
                            snprintf(result_msg, sizeof(result_msg),
                                "HIT %s%s🎯 HIT at %s! 🎯%s\n",
                                BOLD, RED, pos, RESET);
                            send_message(client_socket, result_msg);
                            
                            snprintf(broadcast_msg, sizeof(broadcast_msg),
                                "ATTACK_RESULT %s attacked %s - HIT! 💥\n",
                                game.players[player_id].username, pos);
                            broadcast_message(broadcast_msg);
                            
                            // Same player continues after hit
                        } else { // Miss
                            snprintf(result_msg, sizeof(result_msg),
                                "MISS %s%s💧 MISS at %s 💧%s\n",
                                BOLD, BLUE, pos, RESET);
                            send_message(client_socket, result_msg);
                            
                            snprintf(broadcast_msg, sizeof(broadcast_msg),
                                "ATTACK_RESULT %s attacked %s - Miss 💧\n",
                                game.players[player_id].username, pos);
                            broadcast_message(broadcast_msg);
                            
                            // Switch turns on miss
                            game.current_player = 1 - game.current_player;
                        }
                        
                        // Send updated grids to both players
                        for (int i = 0; i < 2; i++) {
                            send_both_grids(i);
                        }
                        
                        if (game.state == PLAYING) {
                            if (result == 0) { // Only switch turn message on miss
                                send_message(game.players[game.current_player].socket, 
                                    "YOUR_TURN Your turn! Use ATTACK <pos>\n");
                                send_message(game.players[1 - game.current_player].socket, 
                                    "WAIT_TURN Wait for your opponent's move...\n");
                            } else { // Hit - same player continues
                                send_message(client_socket, "CONTINUE You hit! Go again! Use ATTACK <pos>\n");
                            }
                        }
                    }
                } else {
                    send_message(client_socket, "ERROR Invalid format. Use: ATTACK <pos>\n");
                }
            }
        } else if (strcmp(command, "GRID") == 0) {
            send_both_grids(player_id);
        } else if (strcmp(command, "QUIT") == 0) {
            break;
        }
        
        pthread_mutex_unlock(&game_mutex);
    }
    
    pthread_mutex_lock(&game_mutex);
    game.players_connected--;
    if (game.players_connected == 0) {
        init_game();
    }
    pthread_mutex_unlock(&game_mutex);
    
    close(client_socket);
    printf("Player %s disconnected\n", 
        strlen(game.players[player_id].username) > 0 ? 
        game.players[player_id].username : "Unknown");
    return NULL;
}

int main(void) {
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    
    signal(SIGINT, signal_handler);
    init_game();
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(1);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    if (listen(listen_fd, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }
    
    printf("%s%s🚢 Mini Battleship Server 🚢%s\n", BOLD, CYAN, RESET);
    printf("%s%sRunning on port %d%s\n", BOLD, GREEN, PORT, RESET);
    printf("%s%sWaiting for players to join...%s\n", BOLD, YELLOW, RESET);
    
    while (1) {
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
        
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }
        
        printf("%s%s⭐ New player connected from %s%s\n", 
            BOLD, GREEN, inet_ntoa(cliaddr.sin_addr), RESET);
        
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("Thread creation failed");
            close(*client_socket);
            free(client_socket);
        }
        pthread_detach(thread_id);
    }
    
    close(listen_fd);
    return 0;
}