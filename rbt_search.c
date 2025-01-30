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
    args->size_lower_bound = 0;
    args->size_upper_bound = 0;
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

        else if (!strcmp(argv[i], "--size") && i + 1 < argc) {
            const char *size_arg = argv[++i];

            if (strchr(size_arg, '-')) {
                // Handle lower-bound size (e.g., "100k-", "10M-")
                if (size_arg[strlen(size_arg) - 1] == '-') {
                    char size_str[256];
                    strncpy(size_str, size_arg, strlen(size_arg) - 1);
                    size_str[strlen(size_arg) - 1] = '\0'; // Remove trailing '-'
                    args->size_upper_bound = parse_size(size_str);
                }
                // Handle size ranges (e.g., "10M-100M")
                else {
                    char *dash = strchr(size_arg, '-');
                    if (!dash || dash == size_arg || dash == size_arg + strlen(size_arg) - 1) {
                        fprintf(stderr, "Invalid range for --size: %s\n", size_arg);
                        exit(EXIT_FAILURE);
                    }
                    // Extract lower and upper bounds
                    char lower_str[256], upper_str[256];
                    strncpy(lower_str, size_arg, dash - size_arg);
                    lower_str[dash - size_arg] = '\0';
                    strcpy(upper_str, dash + 1);

                    args->size_lower_bound = parse_size(upper_str);
                    args->size_upper_bound = parse_size(lower_str);

                    // Validate sizes
                    if (args->size_lower_bound > args->size_upper_bound) {
                        fprintf(stderr, "Invalid size range: lower bound is larger than upper bound.\n");
                        exit(EXIT_FAILURE);
                    }
                }
            } else {
                // Handle upper-bound size (e.g., "100k", "10M", "1G")
                args->size_lower_bound = parse_size(size_arg);
            }
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

    bool (*match_function)(const char *, char **) = NULL;

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
    long long totalCount = arguments.names_count > 1 || arguments.paths_count > 1 ? -1 : 0;
    search_tree(root, arguments, match_function, &results, &totalCount);
    if (arguments.names_count > 1 || arguments.paths_count > 1) {
        print_results(&results);
    }
    else {
        printf("\nTotal nodes found: %lld\n", totalCount);
    }
    cleanup_map_results(&results);

    return 0;
}
