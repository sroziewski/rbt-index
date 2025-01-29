#include "rbtlib/rbtree.h"
#include "shared/shared.h"

DEFINE_COMPARATOR_BY_FIELD(name, strcmp)
DEFINE_COMPARATOR_BY_FIELD(path, strcmp)
DEFINE_COMPARATOR_BY_FIELD(hash, strcmp)
DEFINE_NUMERIC_COMPARATOR(size)

#define USAGE_MSG "Usage: --name, --size, --path, --all, --hash <filename.lst>, or --list <filename.lst>\n"

void print_usage_and_exit() {
    fprintf(stderr, "%s", USAGE_MSG);
    exit(EXIT_FAILURE);
}

/**
 * Parses command-line arguments to set up the configuration.
 *
 * @param argc Number of command-line arguments.
 * @param argv Pointer to argument strings.
 * @param config Output configuration structure populated based on parsed arguments.
 */
void *parse_arguments(const int argc, char *argv[], Config *config) {
    if (argc < 2) {
        print_usage_and_exit();
    }

    const char *filename = NULL;
    config->all = false;
    config->skipCheck = false;
    config->save = false; // Initialize the "save" field to false
    config->insert_fn = NULL;
    config->prefix = NULL;

    // Handle operation flags
    if (strcmp(argv[1], "--name") == 0) {
        config->prefix = "rbt_name_";
        config->insert_fn = insert_name;
    } else if (strcmp(argv[1], "--size") == 0) {
        config->prefix = "rbt_size_";
        config->insert_fn = insert_size;
    } else if (strcmp(argv[1], "--path") == 0) {
        config->prefix = "rbt_path_";
        config->insert_fn = insert_path;
    } else if (strcmp(argv[1], "--hash") == 0) {
        config->prefix = "rbt_hash_";
        config->insert_fn = insert_hash;
    } else if (strcmp(argv[1], "--all") == 0) {
        config->all = true;
    } else if (strcmp(argv[1], "--load") == 0) {
        config->skipCheck = true;
    } else if (strcmp(argv[1], "--list") == 0 && argc == 3) {
        listSharedMemoryEntities(argv[2]);
        exit(EXIT_SUCCESS);
    } else {
        print_usage_and_exit();
    }
    if (argc >= 3) {
        config->filename = strdup(argv[2]);
    } else {
        print_usage_and_exit();
    }
    if (argc == 4 && strcmp(argv[3], "--save") == 0) {
        config->save = true;
    }
}

/**
 * Handles input file checks if skipCheck is false.
 *
 * @param filename The name of the input file (if provided).
 */
void handle_input_file_checks(const char *filename) {
    if (!filename) {
        fprintf(stderr, "Error: No filename provided for the operation.\n");
        exit(EXIT_FAILURE);
    }

    char **filenames = malloc(2 * sizeof(char *)); // For filename and NULL terminator
    char **rootDirectories = NULL;

    if (!filenames) {
        perror("Failed to allocate memory for filenames");
        exit(EXIT_FAILURE);
    }

    filenames[0] = (char *) filename;
    filenames[1] = NULL;

    int rootCount = 0;
    check_input_files(filenames, &rootDirectories, &rootCount);

    // Free allocated memory
    free(filenames);
    for (int i = 0; i < rootCount; ++i) {
        free(rootDirectories[i]);
    }
    free(rootDirectories);
}

/**
 * Main entry point for the program.
 */
int main(const int argc, char *argv[]) {
    Config config;
    parse_arguments(argc, argv, &config);

    if (config.filename) {
        handle_input_file_checks(config.filename);
    }
    if (config.all) {
        createRbt(argc, argv, insert_name, "rbt_name_", config);
        createRbt(argc, argv, insert_size, "rbt_size_", config);
        createRbt(argc, argv, insert_path, "rbt_path_", config);
        createRbt(argc, argv, insert_hash, "rbt_hash_", config);
    } else {
        createRbt(argc, argv, config.insert_fn, config.prefix, config);
    }

    return EXIT_SUCCESS;
}
