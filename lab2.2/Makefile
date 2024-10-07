# Makefile for pthread_sum program

# Run with sudo make / sudo make clean

CC = gcc

CFLAGS = -Wall -pthread

TARGET = pthread_sum

all: $(TARGET)

$(TARGET): pthread_sum.o
	$(CC) $(CFLAGS) -o $(TARGET) pthread_sum.o

pthread_sum.o: pthread_sum.c
	$(CC) $(CFLAGS) -c pthread_sum.c

clean:
	rm -f *.o $(TARGET)


