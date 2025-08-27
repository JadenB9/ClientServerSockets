/*
 * File: server.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: Multiplayer Battleship Game Server
 *              Manages two-player Battleship games with ship placement and turn-based combat
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
#define GRID_SIZE 8
#define MAX_SHIPS 6

// Ship types
typedef enum {
    CARRIER = 4,
    BATTLESHIP = 3, 
    DESTROYER = 2,
    SUBMARINE = 1
} ship_size_t;

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
    PLACING_SHIPS,
    PLAYING,
    GAME_OVER
} game_state_t;

// Player structure
typedef struct {
    int socket;
    int player_id;
    char name[32];
    cell_state_t grid[GRID_SIZE][GRID_SIZE];
    cell_state_t enemy_view[GRID_SIZE][GRID_SIZE];
    int ships_remaining;
    int ships_placed;
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

void signal_handler(int sig) {
    printf("\nShutting down server...\n");
    if (listen_fd != -1) {
        close(listen_fd);
    }
    exit(0);
}

void init_game() {
    memset(&game, 0, sizeof(game_t));
    game.state = WAITING_FOR_PLAYERS;
    game.current_player = 0;
    
    for (int p = 0; p < 2; p++) {
        game.players[p].socket = -1;
        game.players[p].player_id = p;
        game.players[p].ships_remaining = MAX_SHIPS;
        game.players[p].ships_placed = 0;
        
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

void send_grid(int socket, cell_state_t grid[GRID_SIZE][GRID_SIZE], int show_ships) {
    char buffer[1024];
    char grid_str[512] = "";
    
    strcat(grid_str, "   A B C D E F G H\n");
    
    for (int i = 0; i < GRID_SIZE; i++) {
        char row[64];
        snprintf(row, sizeof(row), "%d  ", i + 1);
        strcat(grid_str, row);
        
        for (int j = 0; j < GRID_SIZE; j++) {
            char cell[4];
            switch (grid[i][j]) {
                case EMPTY: strcpy(cell, ". "); break;
                case SHIP: strcpy(cell, show_ships ? "S " : ". "); break;
                case HIT: strcpy(cell, "X "); break;
                case MISS: strcpy(cell, "O "); break;
            }
            strcat(grid_str, cell);
        }
        strcat(grid_str, "\n");
    }
    
    snprintf(buffer, sizeof(buffer), "GRID\n%s", grid_str);
    send_message(socket, buffer);
}

int validate_ship_placement(int player_id, int row, int col, int size, int horizontal) {
    player_t* player = &game.players[player_id];
    
    // Check bounds
    if (horizontal) {
        if (col + size > GRID_SIZE) return 0;
    } else {
        if (row + size > GRID_SIZE) return 0;
    }
    
    // Check for overlaps and adjacent ships
    for (int i = 0; i < size; i++) {
        int r = row + (horizontal ? 0 : i);
        int c = col + (horizontal ? i : 0);
        
        if (player->grid[r][c] != EMPTY) return 0;
        
        // Check adjacent cells
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int nr = r + dr, nc = c + dc;
                if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                    if (player->grid[nr][nc] == SHIP) return 0;
                }
            }
        }
    }
    
    return 1;
}

void place_ship(int player_id, int row, int col, int size, int horizontal) {
    player_t* player = &game.players[player_id];
    
    for (int i = 0; i < size; i++) {
        int r = row + (horizontal ? 0 : i);
        int c = col + (horizontal ? i : 0);
        player->grid[r][c] = SHIP;
    }
    
    player->ships_placed++;
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
        
        // Check if ship is sunk
        int ship_sunk = 1;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (defender->grid[i][j] == SHIP) {
                    ship_sunk = 0;
                    break;
                }
            }
            if (!ship_sunk) break;
        }
        
        if (ship_sunk) defender->ships_remaining--;
        
        return 1; // Hit
    } else {
        defender->grid[row][col] = MISS;
        attacker->enemy_view[row][col] = MISS;
        return 0; // Miss
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
    
    snprintf(buffer, sizeof(buffer), "WELCOME Player %d\n", player_id + 1);
    send_message(client_socket, buffer);
    
    if (game.players_connected == 2) {
        game.state = PLACING_SHIPS;
        send_message(game.players[0].socket, "START Ship placement phase\n");
        send_message(game.players[1].socket, "START Ship placement phase\n");
        printf("Game started with 2 players\n");
    } else {
        send_message(client_socket, "WAIT Waiting for another player\n");
    }
    
    pthread_mutex_unlock(&game_mutex);
    
    while (1) {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        
        buffer[bytes_received] = '\0';
        printf("Player %d: %s", player_id + 1, buffer);
        
        pthread_mutex_lock(&game_mutex);
        
        char command[16], args[256];
        sscanf(buffer, "%s %[^\n]", command, args);
        
        if (strcmp(command, "PLACE") == 0) {
            if (game.state != PLACING_SHIPS) {
                send_message(client_socket, "ERROR Not in ship placement phase\n");
            } else {
                char pos[4], orientation[16];
                int size;
                if (sscanf(args, "%d %s %s", &size, pos, orientation) == 3) {
                    int col = pos[0] - 'A';
                    int row = pos[1] - '1';
                    int horizontal = (strcmp(orientation, "H") == 0);
                    
                    if (validate_ship_placement(player_id, row, col, size, horizontal)) {
                        place_ship(player_id, row, col, size, horizontal);
                        send_message(client_socket, "OK Ship placed\n");
                        send_grid(client_socket, game.players[player_id].grid, 1);
                        
                        if (game.players[player_id].ships_placed == MAX_SHIPS) {
                            send_message(client_socket, "READY Waiting for opponent\n");
                            
                            if (game.players[0].ships_placed == MAX_SHIPS && 
                                game.players[1].ships_placed == MAX_SHIPS) {
                                game.state = PLAYING;
                                send_message(game.players[0].socket, "BATTLE Game starts! Your turn\n");
                                send_message(game.players[1].socket, "BATTLE Game starts! Wait for your turn\n");
                            }
                        }
                    } else {
                        send_message(client_socket, "ERROR Invalid ship placement\n");
                    }
                } else {
                    send_message(client_socket, "ERROR Invalid format. Use: PLACE <size> <pos> <H|V>\n");
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
                        char result_msg[64];
                        if (result == 1) {
                            snprintf(result_msg, sizeof(result_msg), "HIT %s\n", pos);
                            if (game.players[1 - player_id].ships_remaining == 0) {
                                game.state = GAME_OVER;
                                send_message(client_socket, "WIN You win!\n");
                                send_message(game.players[1 - player_id].socket, "LOSE You lose!\n");
                            } else {
                                game.current_player = 1 - game.current_player;
                            }
                        } else {
                            snprintf(result_msg, sizeof(result_msg), "MISS %s\n", pos);
                            game.current_player = 1 - game.current_player;
                        }
                        
                        send_message(client_socket, result_msg);
                        send_message(game.players[1 - player_id].socket, result_msg);
                        
                        if (game.state == PLAYING) {
                            send_message(game.players[game.current_player].socket, "TURN Your turn\n");
                        }
                    }
                } else {
                    send_message(client_socket, "ERROR Invalid format. Use: ATTACK <pos>\n");
                }
            }
        } else if (strcmp(command, "GRID") == 0) {
            send_grid(client_socket, game.players[player_id].grid, 1);
        } else if (strcmp(command, "ENEMY") == 0) {
            send_grid(client_socket, game.players[player_id].enemy_view, 0);
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
    printf("Player %d disconnected\n", player_id + 1);
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
    
    printf("Battleship Server running on port %d\n", PORT);
    printf("Waiting for players to join...\n");
    
    while (1) {
        int* client_socket = malloc(sizeof(int));
        *client_socket = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
        
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }
        
        printf("New player connected from %s\n", inet_ntoa(cliaddr.sin_addr));
        
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