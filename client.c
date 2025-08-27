/*
 * File: client.c
 * Author: [Your Name]
 * Date: August 27, 2025
 * Description: TCP Socket Client that sends a message (from command line argument)
 *              to the server, receives the transformed response, displays it, and exits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345          // Must match server port
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"  // Localhost

/*
 * Function: main
 * Purpose: Main client function - connects to server, sends message, receives response
 * Parameters: argc - argument count, argv - argument vector
 */
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s \"message to send\"\n", argv[0]);
        printf("Example: %s \"This is a message to be modified.\"\n", argv[0]);
        return 1;
    }
    
    // Check message length
    if (strlen(argv[1]) >= BUFFER_SIZE) {
        printf("Error: Message too long. Maximum length is %d characters.\n", BUFFER_SIZE - 1);
        return 1;
    }
    
    printf("Connecting to server at %s:%d...\n", SERVER_IP, PORT);
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        close(sockfd);
        return 1;
    }
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        printf("Make sure the server is running on port %d\n", PORT);
        close(sockfd);
        return 1;
    }
    
    printf("Connected to server successfully.\n");
    printf("Sending message: \"%s\"\n", argv[1]);
    
    // Send message to server
    if (write(sockfd, argv[1], strlen(argv[1])) < 0) {
        perror("Write failed");
        close(sockfd);
        return 1;
    }
    
    // Clear buffer for receiving response
    memset(buffer, 0, BUFFER_SIZE);
    
    // Receive response from server
    bytes_received = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (bytes_received < 0) {
        perror("Read failed");
        close(sockfd);
        return 1;
    }
    else if (bytes_received == 0) {
        printf("Server closed the connection.\n");
        close(sockfd);
        return 1;
    }
    
    // Null-terminate the received message
    buffer[bytes_received] = '\0';
    
    // Display server response
    printf("Server response: \"%s\"\n", buffer);
    
    // Close connection and exit
    close(sockfd);
    printf("Disconnected from server.\n");
    
    return 0;
}