/*
 * File: client.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: Battleship Game Client
 *              Interactive client for playing multiplayer Battleship
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 19845
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int sockfd;
int game_active = 1;
int my_turn = 0;

void print_instructions() {
    printf("\n=== BATTLESHIP GAME ===\n");
    printf("Commands:\n");
    printf("  PLACE <size> <pos> <H|V> - Place a ship (e.g., PLACE 4 A1 H)\n");
    printf("  ATTACK <pos>             - Attack enemy position (e.g., ATTACK B3)\n");
    printf("  GRID                     - Show your grid\n");
    printf("  ENEMY                    - Show enemy grid\n");
    printf("  HELP                     - Show this help\n");
    printf("  QUIT                     - Exit game\n");
    printf("\nShip sizes: Carrier(4), Battleship(3), Destroyer(2), Destroyer(2), Submarine(1), Submarine(1)\n");
    printf("Grid positions: A1-H8 (columns A-H, rows 1-8)\n");
    printf("Orientation: H = Horizontal, V = Vertical\n");
    printf("========================\n\n");
}

void print_ship_placement_guide() {
    printf("\n=== SHIP PLACEMENT PHASE ===\n");
    printf("Place your 6 ships on the 8x8 grid:\n");
    printf("1. Carrier (4 spaces)    - PLACE 4 <pos> <H|V>\n");
    printf("2. Battleship (3 spaces) - PLACE 3 <pos> <H|V>\n");
    printf("3. Destroyer (2 spaces)  - PLACE 2 <pos> <H|V>\n");
    printf("4. Destroyer (2 spaces)  - PLACE 2 <pos> <H|V>\n");
    printf("5. Submarine (1 space)   - PLACE 1 <pos> <H|V>\n");
    printf("6. Submarine (1 space)   - PLACE 1 <pos> <H|V>\n");
    printf("\nExample: PLACE 4 A1 H (places carrier horizontally starting at A1)\n");
    printf("============================\n");
}

void* receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (game_active) {
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Disconnected from server\n");
            game_active = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        char command[16], message[512];
        if (sscanf(buffer, "%s %[^\n]", command, message) >= 1) {
            if (strcmp(command, "WELCOME") == 0) {
                printf("Connected! %s\n", message);
                print_instructions();
            } else if (strcmp(command, "WAIT") == 0) {
                printf("Waiting for another player to join...\n");
            } else if (strcmp(command, "START") == 0) {
                printf("\nGame starting! %s\n", message);
                print_ship_placement_guide();
            } else if (strcmp(command, "OK") == 0) {
                printf("✓ %s\n", message);
            } else if (strcmp(command, "READY") == 0) {
                printf("All ships placed! %s\n", message);
            } else if (strcmp(command, "BATTLE") == 0) {
                printf("\n🚢 BATTLE BEGINS! %s\n", message);
                my_turn = (strstr(message, "Your turn") != NULL);
                if (my_turn) {
                    printf("💥 It's your turn! Use ATTACK <pos> to fire!\n");
                } else {
                    printf("⏳ Wait for your opponent's move...\n");
                }
            } else if (strcmp(command, "HIT") == 0) {
                printf("🎯 HIT at %s!\n", message);
            } else if (strcmp(command, "MISS") == 0) {
                printf("💧 MISS at %s\n", message);
            } else if (strcmp(command, "TURN") == 0) {
                printf("\n💥 Your turn! Attack with: ATTACK <pos>\n");
                my_turn = 1;
            } else if (strcmp(command, "WIN") == 0) {
                printf("\n🎉 VICTORY! %s\n", message);
                game_active = 0;
            } else if (strcmp(command, "LOSE") == 0) {
                printf("\n💀 DEFEAT! %s\n", message);
                game_active = 0;
            } else if (strcmp(command, "ERROR") == 0) {
                printf("❌ Error: %s\n", message);
            } else if (strcmp(command, "GRID") == 0) {
                printf("\n=== YOUR GRID ===\n");
                printf("%s", buffer + 5); // Skip "GRID\n"
                printf("=================\n");
            } else {
                printf("%s", buffer);
            }
        } else {
            printf("%s", buffer);
        }
        
        fflush(stdout);
    }
    
    return NULL;
}

void send_command(const char* command) {
    send(sockfd, command, strlen(command), 0);
}

int main(void) {
    struct sockaddr_in servaddr;
    char input[256];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }
    
    printf("Connecting to Battleship server at %s:%d...\n", SERVER_IP, PORT);
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Thread creation failed");
        exit(1);
    }
    
    printf("Type 'HELP' for commands or 'QUIT' to exit\n");
    
    while (game_active) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "QUIT") == 0) {
            send_command("QUIT\n");
            game_active = 0;
            break;
        } else if (strcmp(input, "HELP") == 0) {
            print_instructions();
            continue;
        }
        
        // Add newline for server protocol
        strcat(input, "\n");
        send_command(input);
        
        // Reset turn flag after sending command
        if (strncmp(input, "ATTACK", 6) == 0) {
            my_turn = 0;
        }
    }
    
    pthread_cancel(recv_thread);
    close(sockfd);
    printf("Goodbye!\n");
    
    return 0;
}