# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# Target executable
TARGET = rbt_name_create

# Source files
SRCS = rbt_name_create.c rbtree.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build object files
%.o: %.c rbtree.h
	$(CC) $(CFLAGS) -c $<

# Run the program with specific arguments
run_load: $(TARGET)
	./$(TARGET) --load /home/simon/playground/test.out.abc.rbt

run_save: $(TARGET)
	./$(TARGET) /home/simon/playground/test.out.abc --save

run_save_mem: $(TARGET)
	./$(TARGET) /home/simon/playground/test.out.abc

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)