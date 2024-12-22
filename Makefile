# Compiler and flags
CC = gcc
CFLAGS = -Wextra -g -lmagic -fopenmp

# Target executables
TARGET = rbt_name_create
LIST_FILES_TARGET = list_files

# Source files
SRCS = rbt_name_create.c rbtree.c
LIST_FILES_SRCS = list_files.c lfiles.c  # Add the source file for building list_files

# Object files
OBJS = $(SRCS:.c=.o)
LIST_FILES_OBJS = $(LIST_FILES_SRCS:.c=.o)

# Default target (build all executables)
all: $(TARGET) $(LIST_FILES_TARGET)

# Rule to build the main target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build the list_files target
$(LIST_FILES_TARGET): $(LIST_FILES_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(CFLAGS)

# Rule to build object files for list_files
list_files.o: list_files.c
	$(CC) $(CFLAGS) -c $<

# Run the program with specific arguments
run_load: $(TARGET)
	./$(TARGET) --load /home/simon/playground/test.out.abc.rbt

run_save: $(TARGET)
	./$(TARGET) /home/simon/playground/test.out.abc --save

run_save_mem: $(TARGET)
	./$(TARGET) /home/simon/playground/test.out.abc

# Optional run target for list_files
run_list_files: $(LIST_FILES_TARGET)
	./$(LIST_FILES_TARGET) /home/simon/playground -o /home/simon/playground/playground.lst > /home/simon/playground/playground.lst.out

# Clean up build files
clean:
	rm -f $(OBJS) $(LIST_FILES_OBJS) $(TARGET) $(LIST_FILES_TARGET)