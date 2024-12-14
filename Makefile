CC = gcc
CFLAGS = -Wall -pthread -g 
CLIENT = client
SERVER = server
CLIENT_SRC = client.c
SERVER_SRC = server.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Default target to build both client and server
all: $(CLIENT) $(SERVER)

# Rule to build the client executable
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CLIENT_OBJ) -o $(CLIENT)

# Rule to build the server executable
$(SERVER): $(SERVER_OBJ)
	$(CC) $(SERVER_OBJ) -o $(SERVER)

# Rule to compile client source files
$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $(CLIENT_SRC) -o $(CLIENT_OBJ)

# Rule to compile server source files
$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $(SERVER_SRC) -o $(SERVER_OBJ)

# Clean up compiled files
clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT) $(SERVER)

# Rebuild everything from scratch
rebuild: clean all