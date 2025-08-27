/*
 * File: server.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: TCP Socket Server that receives messages from clients,
 *              transforms them to uppercase, and sends back the result.
 *              Handles multiple consecutive client connections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>

#define PORT 19845          // Custom 5-digit port number
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Global variables for cleanup
int listen_fd = -1;

/*
 * Function: signal_handler
 * Purpose: Handle SIGINT (Ctrl-C) gracefully by closing the server socket
 */
void signal_handler(int sig)
{
    (void)sig; // Suppress unused parameter warning
    printf("\nServer shutting down gracefully...\n");
    if (listen_fd != -1) {
        close(listen_fd);
    }
    exit(0);
}

/*
 * Function: transform_message
 * Purpose: Transform input message to uppercase
 * Parameters: message - string to transform
 */
void transform_message(char* message)
{
    int i = 0;
    while (message[i] != '\0') {
        message[i] = toupper(message[i]);
        i++;
    }
}

/*
 * Function: handle_client
 * Purpose: Handle communication with a single client
 * Parameters: comm_fd - client socket file descriptor
 */
void handle_client(int comm_fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    // Clear buffer
    memset(buffer, 0, BUFFER_SIZE);
    
    // Read message from client
    bytes_received = read(comm_fd, buffer, BUFFER_SIZE - 1);
    
    if (bytes_received > 0) {
        // Null-terminate the received message
        buffer[bytes_received] = '\0';
        
        printf("Received from client: %s\n", buffer);
        
        // Transform the message (convert to uppercase)
        transform_message(buffer);
        
        printf("Sending back transformed message: %s\n", buffer);
        
        // Send transformed message back to client
        write(comm_fd, buffer, strlen(buffer));
    }
    else {
        printf("No data received from client or connection closed.\n");
    }
}

/*
 * Function: main
 * Purpose: Main server function - creates socket, binds, listens, and handles clients
 */
int main(void)
{
    int comm_fd;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t client_len = sizeof(clientaddr);
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    
    printf("Starting server on port %d...\n", PORT);
    
    // Create socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(listen_fd);
        exit(1);
    }
    
    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    // Bind socket to address
    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(listen_fd);
        exit(1);
    }
    
    // Start listening for connections
    if (listen(listen_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(listen_fd);
        exit(1);
    }
    
    printf("Server listening on port %d. Press Ctrl-C to quit.\n", PORT);
    
    // Main server loop - handle multiple consecutive client connections
    while (1) {
        // Accept client connection
        comm_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &client_len);
        
        if (comm_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected from %s:%d\n", 
               inet_ntoa(clientaddr.sin_addr), 
               ntohs(clientaddr.sin_port));
        
        // Handle the client request
        handle_client(comm_fd);
        
        // Close client connection
        close(comm_fd);
        printf("Client disconnected.\n\n");
    }
    
    // This code should never be reached due to signal handler
    close(listen_fd);
    return 0;
}
