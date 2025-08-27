/*
 * File: client.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: Mini Battleship Game Client
 *              Interactive client with enhanced visuals, colors, and username system
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
#define BUFFER_SIZE 4096
#define SERVER_IP "127.0.0.1"

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

int sockfd;
int game_active = 1;
int waiting_for_username = 1;

void clear_screen(void) {
    printf("\033[2J\033[H");
}

void print_banner(void) {
    printf("%s%s", BOLD, CYAN);
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    🚢 MINI BATTLESHIP 🚢                     ║\n");
    printf("║                      4x4 Grid • 1 Ship                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", RESET);
}

void print_instructions(void) {
    printf("%s%s┌─ GAME RULES ─────────────────────────────────────────────┐%s\n", BOLD, YELLOW, RESET);
    printf("%s│%s Each player places ONE ship (2 spaces) on a 4x4 grid     %s│%s\n", YELLOW, WHITE, YELLOW, RESET);
    printf("%s│%s Ships can be placed horizontally (H) or vertically (V)   %s│%s\n", YELLOW, WHITE, YELLOW, RESET);
    printf("%s│%s Take turns attacking enemy positions                     %s│%s\n", YELLOW, WHITE, YELLOW, RESET);
    printf("%s│%s First to sink opponent's ship wins!                      %s│%s\n", YELLOW, WHITE, YELLOW, RESET);
    printf("%s└─────────────────────────────────────────────────────────┘%s\n\n", YELLOW, RESET);
    
    printf("%s%s┌─ COMMANDS ───────────────────────────────────────────────┐%s\n", BOLD, GREEN, RESET);
    printf("%s│%s PLACE <pos> <H|V> - Place ship (e.g., PLACE A1 H)       %s│%s\n", GREEN, WHITE, GREEN, RESET);
    printf("%s│%s ATTACK <pos>      - Attack position (e.g., ATTACK B3)   %s│%s\n", GREEN, WHITE, GREEN, RESET);
    printf("%s│%s GRID              - Show both grids                      %s│%s\n", GREEN, WHITE, GREEN, RESET);
    printf("%s│%s HELP              - Show this help                       %s│%s\n", GREEN, WHITE, GREEN, RESET);
    printf("%s│%s QUIT              - Exit game                            %s│%s\n", GREEN, WHITE, GREEN, RESET);
    printf("%s└─────────────────────────────────────────────────────────┘%s\n\n", GREEN, RESET);
}

void print_placement_help(void) {
    printf("%s%s┌─ SHIP PLACEMENT HELP ────────────────────────────────────┐%s\n", BOLD, MAGENTA, RESET);
    printf("%s│%s Your ship is 2 spaces long                               %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s                                                         %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s Examples:                                                %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s   PLACE A1 H  →  🚢🚢⬜⬜  (horizontal at A1-B1)        %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s   PLACE C2 V  →  ⬜⬜🚢⬜  (vertical at C2-C3)          %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s                  ⬜⬜🚢⬜                                %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s                                                         %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s│%s Grid positions: A1, A2, A3, A4, B1, B2, B3, B4, etc.   %s│%s\n", MAGENTA, WHITE, MAGENTA, RESET);
    printf("%s└─────────────────────────────────────────────────────────┘%s\n\n", MAGENTA, RESET);
}

void* receive_messages(void* arg) {
    (void)arg;  // Suppress unused parameter warning
    char buffer[BUFFER_SIZE];
    
    while (game_active) {
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("\n%s%s🔌 Disconnected from server%s\n", BOLD, RED, RESET);
            game_active = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        char command[32], message[BUFFER_SIZE];
        if (sscanf(buffer, "%s %[^\n]", command, message) >= 1) {
            if (strcmp(command, "WELCOME") == 0) {
                clear_screen();
                print_banner();
                printf("%s\n", message + 8); // Skip "WELCOME " prefix
                waiting_for_username = 1;
            } else if (strcmp(command, "USERNAME_SET") == 0) {
                printf("%s\n", message + 13); // Skip "USERNAME_SET " prefix
                waiting_for_username = 0;
            } else if (strcmp(command, "WAIT_PLAYER") == 0) {
                printf("%s\n", message + 12); // Skip "WAIT_PLAYER " prefix
            } else if (strcmp(command, "GAME_START") == 0) {
                clear_screen();
                print_banner();
                printf("%s\n", message + 11); // Skip "GAME_START " prefix
                print_placement_help();
                printf("%s%s💡 Place your ship now!%s\n", BOLD, GREEN, RESET);
            } else if (strcmp(command, "SHIP_PLACED") == 0) {
                printf("%s\n", message + 12); // Skip "SHIP_PLACED " prefix
            } else if (strcmp(command, "BATTLE_START") == 0) {
                clear_screen();
                print_banner();
                printf("%s\n", message + 13); // Skip "BATTLE_START " prefix
                print_instructions();
            } else if (strcmp(command, "YOUR_TURN") == 0) {
                printf("\n%s%s🎯 YOUR TURN!%s Attack with: %sATTACK <pos>%s\n", 
                    BOLD, GREEN, RESET, BOLD, RESET);
                printf("%s> %s", BOLD, RESET);
                fflush(stdout);
            } else if (strcmp(command, "WAIT_TURN") == 0) {
                printf("\n%s%s⏳ WAITING...%s %s\n", BOLD, YELLOW, RESET, message + 10);
            } else if (strcmp(command, "CONTINUE") == 0) {
                printf("\n%s%s🔥 KEEP FIRING!%s %s\n", BOLD, RED, RESET, message + 9);
                printf("%s> %s", BOLD, RESET);
                fflush(stdout);
            } else if (strcmp(command, "HIT") == 0) {
                printf("\n%s\n", message + 4); // Skip "HIT " prefix
            } else if (strcmp(command, "MISS") == 0) {
                printf("\n%s\n", message + 5); // Skip "MISS " prefix
            } else if (strcmp(command, "WIN") == 0) {
                printf("\n%s%s", BOLD, GREEN);
                printf("╔══════════════════════════════════════════════════════════════╗\n");
                printf("║                        🎉 VICTORY! 🎉                        ║\n");
                printf("║                   You sunk their ship!                       ║\n");
                printf("╚══════════════════════════════════════════════════════════════╝\n");
                printf("%s\n", RESET);
                game_active = 0;
            } else if (strcmp(command, "LOSE") == 0) {
                printf("\n%s%s", BOLD, RED);
                printf("╔══════════════════════════════════════════════════════════════╗\n");
                printf("║                        💀 DEFEAT 💀                         ║\n");
                printf("║                   Your ship was sunk!                       ║\n");
                printf("╚══════════════════════════════════════════════════════════════╝\n");
                printf("%s\n", RESET);
                game_active = 0;
            } else if (strcmp(command, "GAME_OVER") == 0) {
                printf("\n%s\n", message + 10); // Skip "GAME_OVER " prefix
            } else if (strcmp(command, "ATTACK_RESULT") == 0) {
                printf("%s%s📢 %s%s\n", BOLD, CYAN, message + 14, RESET);
            } else if (strcmp(command, "ERROR") == 0) {
                printf("\n%s%s❌ Error: %s%s\n", BOLD, RED, message + 6, RESET);
            } else if (strcmp(command, "GRID") == 0) {
                printf("\n%s", buffer + 5); // Skip "GRID\n"
            } else if (strcmp(command, "BOTH_GRIDS") == 0) {
                clear_screen();
                print_banner();
                printf("%s", buffer + 11); // Skip "BOTH_GRIDS\n"
                printf("%s> %s", BOLD, RESET);
                fflush(stdout);
            } else {
                printf("%s", buffer);
            }
        } else {
            printf("%s", buffer);
        }
        
        if (!waiting_for_username && game_active) {
            fflush(stdout);
        }
    }
    
    return NULL;
}

void send_command(const char* command) {
    send(sockfd, command, strlen(command), 0);
}

void print_prompt(void) {
    if (waiting_for_username) {
        printf("%s%s👤 Username: %s", BOLD, CYAN, RESET);
    } else {
        printf("%s> %s", BOLD, RESET);
    }
    fflush(stdout);
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
    
    printf("%s%s🔗 Connecting to Mini Battleship server at %s:%d...%s\n", 
        BOLD, CYAN, SERVER_IP, PORT, RESET);
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Thread creation failed");
        exit(1);
    }
    
    // Wait a moment for the welcome message
    usleep(100000);
    
    while (game_active) {
        print_prompt();
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "QUIT") == 0 || strcmp(input, "quit") == 0) {
            send_command("QUIT\n");
            game_active = 0;
            break;
        } else if (strcmp(input, "HELP") == 0 || strcmp(input, "help") == 0) {
            if (!waiting_for_username) {
                print_instructions();
            }
            continue;
        } else if (strcmp(input, "CLEAR") == 0 || strcmp(input, "clear") == 0) {
            clear_screen();
            print_banner();
            continue;
        }
        
        // Convert to uppercase for commands (but preserve username case)
        if (!waiting_for_username) {
            // Only convert command part to uppercase
            char* space = strchr(input, ' ');
            if (space) {
                *space = '\0';
                for (int i = 0; input[i]; i++) {
                    if (input[i] >= 'a' && input[i] <= 'z') {
                        input[i] = input[i] - 'a' + 'A';
                    }
                }
                *space = ' ';
            } else {
                for (int i = 0; input[i]; i++) {
                    if (input[i] >= 'a' && input[i] <= 'z') {
                        input[i] = input[i] - 'a' + 'A';
                    }
                }
            }
        }
        
        // Add newline for server protocol
        strcat(input, "\n");
        send_command(input);
        
        if (waiting_for_username) {
            waiting_for_username = 0; // Will be reset by server response if needed
        }
    }
    
    pthread_cancel(recv_thread);
    close(sockfd);
    
    printf("\n%s%s", BOLD, CYAN);
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                       👋 GOODBYE! 👋                         ║\n");
    printf("║                   Thanks for playing!                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", RESET);
    
    return 0;
}