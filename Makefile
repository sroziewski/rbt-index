# Compiler and flags
CC = gcc
CFLAGS = -Wextra -g -fopenmp -pedantic
LDFLAGS_RBT_CREATE = -lcrypto -lssl  # Linker flags for OpenSSL (only for rbt_create)
LDFLAGS_RBT_SEARCH = -lcrypto -lssl
LDFLAGS_LIST_FILES = -lmagic  # Linker flags for list_files (libmagic)

# Target executables
RBT_TARGET = rbt_create
LIST_FILES_TARGET = list_files
RBT_SEARCH_TARGET = rbt_search

# Directories
RBTLIB_DIR = rbtlib
FLIB_DIR = flib
SHARED_DIR = shared

# Object files
RBT_TREE = $(RBTLIB_DIR)/rbtree.o $(SHARED_DIR)/shared.o
RBT_CREATE_OBJS = rbt_create.o $(RBTLIB_DIR)/rbtree.o $(SHARED_DIR)/shared.o
LIST_FILES_OBJ = list_files.o $(FLIB_DIR)/lfiles.o $(SHARED_DIR)/shared.o
RBT_SEARCH_OBJS = rbt_search.o $(RBTLIB_DIR)/rbtree.o $(SHARED_DIR)/shared.o $(RBTLIB_DIR)/search.o

# Default target (build all executables)
all: $(RBT_TARGET) $(LIST_FILES_TARGET) $(RBT_SEARCH_TARGET)

# Rule to build the 'rbt_create' target
$(RBT_TARGET): $(RBT_CREATE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_RBT_CREATE)

# Rule to build the 'list_files' target
$(LIST_FILES_TARGET): $(LIST_FILES_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_LIST_FILES)

# Rule to build the 'rbt_search' target
$(RBT_SEARCH_TARGET): $(RBT_SEARCH_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS_RBT_SEARCH)

# Rules to build shared object files
$(SHARED_DIR)/shared.o: $(SHARED_DIR)/shared.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rules to build rbtlib object files
$(RBTLIB_DIR)/rbtree.o: $(RBTLIB_DIR)/rbtree.c $(SHARED_DIR)/shared.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS_RBT_CREATE)

$(FLIB_DIR)/lfiles.o: $(FLIB_DIR)/lfiles.c $(SHARED_DIR)/shared.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build object files for list_files
list_files.o: list_files.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Run the `rbt_name_create` program with specific arguments
run_load: $(RBT_TARGET)
	./$(RBT_TARGET) --load /home/simon/playground/test.out.abc.rbt

run_save: $(RBT_TARGET)
	./$(RBT_TARGET) /home/simon/playground/test.out.abc --save

run_save_mem: $(RBT_TARGET)
	./$(RBT_TARGET) /home/simon/playground/test.out.abc

# Optional run target for list_files
run_list_files: $(LIST_FILES_TARGET)
	./$(LIST_FILES_TARGET) /home/simon/playground -o /home/simon/playground/playground.lst > /home/simon/playground/playground.lst.out

# Clean up build files
clean:
	rm -f $(LIST_FILES_OBJ) $(RBT_TARGET) $(LIST_FILES_TARGET) $(RBT_SEARCH_TARGET) $(RBT_SEARCH_OBJS)
