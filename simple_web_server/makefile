CC = gcc
CFLAGS = -Wall -Wextra -I.

# List all source files
SRC = web_server.c \
	request/request.c \
	socket/socket_helper.c

# Generate object file names
OBJ = $(SRC:.c=.o)

# Name of the executable
EXEC = web_server

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(EXEC)

# Phony targets
.PHONY: all clean
