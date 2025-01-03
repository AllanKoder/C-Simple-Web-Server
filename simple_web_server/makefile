CC = gcc
CFLAGS = -Wall -Wextra -I. -Ihttp -Ifile_system

# List all source files
SRC = web_server.c \
	main.c \
	http/request.c \
	http/response.c \
	file_system/file_explorer.c

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
