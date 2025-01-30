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
    args->names = NULL;
    args->names_count = 0;
    args->size = 0;
    args->paths = NULL;
    args->paths_count = 0;
    args->type = NULL;
    args->hash = NULL;
    // Iterate through the arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc) {
            args->mem_filename = argv[++i]; // Required argument
        } else if (!strcmp(argv[i], "-n")) {
            // Handle multiple names
            i++;
            // Allocate memory for names array (initially NULL)
            args->names = malloc((argc - i) * sizeof(char *));
            if (args->names == NULL) {
                fprintf(stderr, "Memory allocation failed.\n");
                exit(EXIT_FAILURE);
            }
            while (i < argc && argv[i][0] != '-') {
                args->names[args->names_count] = argv[i];
                args->names_count++;
                i++;
            }
            i--; // Step back to process next argument correctly
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            char *endptr = NULL;
            const long value = strtol(argv[++i], &endptr, 10);
            if (*endptr != '\0' || value < 0 || value > INT_MAX) {
                fprintf(stderr, "Invalid value for -s (size): %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            args->size = (int) value; // Safely assign to integer (after validation)
        }
        else if (!strcmp(argv[i], "-p")) {
            // Handle multiple paths
            i++;
            // Allocate memory for paths array (initially NULL)
            args->paths = malloc((argc - i) * sizeof(char *));
            if (args->paths == NULL) {
                fprintf(stderr, "Memory allocation failed for paths.\n");
                exit(EXIT_FAILURE);
            }
            while (i < argc && argv[i][0] != '-') {
                args->paths[args->paths_count] = argv[i];
                args->paths_count++;
                i++;
            }
            i--; // Step back to process the next argument correctly
        }
        else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
            args->type = argv[++i];
        } else if (!strcmp(argv[i], "-h") && i + 1 < argc) {
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

    bool (*match_function)(const FileInfo *, char **) = NULL;

    Arguments arguments = {0};
    parse_arguments(argc, argv, &arguments);

    printf("Memory Filename: %s\n", arguments.mem_filename);

    if (arguments.names != NULL) {
        if (arguments.names_count > 0) {
            printf("Names (%d):\n", arguments.names_count);
            for (int i = 0; i < arguments.names_count; i++) {
                printf("  - %s\n", arguments.names[i]);
            }
            match_function = match_by_name;
        }
    }
    if (arguments.paths != NULL) {
        if (arguments.paths_count > 0) {
            printf("Paths (%d):\n", arguments.paths_count);
            for (int i = 0; i < arguments.paths_count; i++) {
                printf("  - %s\n", arguments.paths[i]);
            }
            match_function = match_by_path;
        }
    }
    if (arguments.size) printf("Size: %d\n", arguments.size);
    if (arguments.type) printf("Type: %s\n", arguments.type);

    Node *root = load_tree_from_shared_memory(arguments.mem_filename);
    MapResults results = {NULL, 0};
    // search_tree_by_filename_and_type(root, arguments, &results);
    // search_tree_by_name(root, arguments, &results);
    search_tree(root, arguments, match_function, &results);
    print_results(&results);
    cleanup_map_results(&results);

    return 0;
}
