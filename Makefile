# Makefile for Client-Server Socket Programming Assignment
# Author: [Your Name]
# Date: August 27, 2025

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGET_SERVER = server
TARGET_CLIENT = client
SOURCES_SERVER = server.c
SOURCES_CLIENT = client.c

# Default target
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Compile server
$(TARGET_SERVER): $(SOURCES_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SOURCES_SERVER)

# Compile client
$(TARGET_CLIENT): $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SOURCES_CLIENT)

# Clean compiled files
clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)

# Run server (for testing)
run-server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

# Run client with example message (for testing)
run-client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT) "This is a message to be modified."

# Test both programs
test: $(TARGET_SERVER) $(TARGET_CLIENT)
	@echo "Starting server in background..."
	@./$(TARGET_SERVER) &
	@SERVER_PID=$$!; \
	sleep 1; \
	echo "Testing client..."; \
	./$(TARGET_CLIENT) "Hello World"; \
	echo "Stopping server..."; \
	kill $$SERVER_PID 2>/dev/null || true

.PHONY: all clean run-server run-client test