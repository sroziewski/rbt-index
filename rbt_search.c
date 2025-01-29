#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include "rbtlib/rbtree.h"
#include "rbtlib/search.h"

void parse_arguments(const int argc, char *argv[], Arguments *args) {
    // Initialize all struct members to default values
    args->mem_filename = NULL;
    args->name = NULL;
    args->size = 0;
    args->path = NULL;
    args->type = NULL;
    args->hash = NULL;

    // Iterate through the arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc) {
            args->mem_filename = argv[++i]; // Required argument
        } else if (!strcmp(argv[i], "-n") && i + 1 < argc) {
            args->name = argv[++i];
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            char *endptr = NULL;
            const long value = strtol(argv[++i], &endptr, 10);
            if (*endptr != '\0' || value < 0 || value > INT_MAX) {
                fprintf(stderr, "Invalid value for -s (size): %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            args->size = (int) value; // Safely assign to integer (after validation)
        } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            args->path = argv[++i];
        } else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
            args->type = argv[++i];
        } else if (!strcmp(argv[i], "-h") && i + 1 < argc) {
            // Added "hash" argument
            args->hash = argv[++i];
        } else {
            fprintf(stderr, "Unknown or improperly formatted argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    // Ensure the required argument -f (mem_filename) is provided
    if (args->mem_filename == NULL) {
        fprintf(stderr, "Error: -f <memory_filename> is mandatory.\n");
        exit(EXIT_FAILURE);
    }
}

int main(const int argc, char *argv[]) {
    struct timespec start, end;
    initialize_threads();

    Arguments arguments = {0};
    parse_arguments(argc, argv, &arguments);

    // Output parsed arguments
    printf("Memory Filename: %s\n", arguments.mem_filename);
    if (arguments.name) printf("Name: %s\n", arguments.name);
    if (arguments.size) printf("Size: %d\n", arguments.size);
    if (arguments.path) printf("Path: %s\n", arguments.path);
    if (arguments.type) printf("Type: %s\n", arguments.type);

    Node *root = load_tree_from_shared_memory(arguments.mem_filename);
    search_tree_by_filename_and_type(root, arguments.name, arguments.type);

    return 0;
}
