# Variables
CC = gcc              	# Compiler
CFLAGS = -Wall   	# Flags for compiler warnings
TARGET = main	  	# Output program name

# Rule to compile the program
all: $(TARGET)

# Rule for building the executable
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

# Rule to clean up the build files
clean:
	rm -f $(TARGET)

