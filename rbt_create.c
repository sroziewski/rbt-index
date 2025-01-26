#include "rbtlib/rbtree.h"
#include "shared/shared.h"

DEFINE_COMPARATOR_BY_FIELD(name, strcmp)
DEFINE_COMPARATOR_BY_FIELD(path, strcmp)
DEFINE_COMPARATOR_BY_FIELD(hash, strcmp)
DEFINE_NUMERIC_COMPARATOR(size)

/**
 * Main entry point for the program. Processes the command-line arguments to determine
 * the operation type (e.g., working with name, size, or path) and invokes the appropriate
 * functions to process input files, create red-black trees (RBT), or list shared memory
 * entities.
 *
 * @param argc The number of arguments passed to the program.
 * @param argv The array of arguments passed to the program. The first argument specifies
 *             the operation type (--name, --size, --path, or --all), and subsequent
 *             arguments may include filenames or other related options.
 *
 * @return Returns EXIT_SUCCESS (0) if the program completes successfully or
 *         EXIT_FAILURE (1) if an error occurs, such as invalid arguments or memory
 *         allocation failure.
 */
int main(const int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: --name, --size, --path, --all or --hash <filename.lst>\n");
        return EXIT_FAILURE;
    }

    bool all = false;
    bool skipCheck = false;
    const char *prefix = NULL;
    void (*insert_fn)(Node **root, FileInfo key) = NULL;

    if (strcmp(argv[1], "--name") == 0) {
        prefix = "rbt_name_";
        insert_fn = insert_name;
    } else if (strcmp(argv[1], "--size") == 0) {
        prefix = "rbt_size_";
        insert_fn = insert_size;
    } else if (strcmp(argv[1], "--path") == 0) {
        prefix = "rbt_path_";
        insert_fn = insert_path;
    }
    else if (strcmp(argv[1], "--hash") == 0) {
        prefix = "rbt_hash_";
        insert_fn = insert_path;
    }
    else if (strcmp(argv[1], "--all") == 0) {
        all = true;
    } else if (strcmp(argv[1], "--load") == 0) {
        skipCheck = true;
    } else {
        fprintf(stderr, "Invalid argument. Use --name, --size or --path <filename.lst>.\n");
        return EXIT_FAILURE;
    }
    if (argc == 3 && strcmp(argv[1], "--list") == 0) {
        listSharedMemoryEntities(prefix);
        exit(EXIT_SUCCESS);
    }
    if (!skipCheck) {
        char **filenames = malloc(2 * sizeof(char *)); // For 2 elements: argv[2] and NULL
        char **rootDirectories = NULL;

        if (!filenames) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        char *filename = argv[2];
        filenames[0] = filename;
        filenames[1] = NULL;

        int rootCount = 0;
        check_input_files(filenames, &rootDirectories, &rootCount);
    }
    if (all) {
        createRbt(argc, argv, insert_name, "rbt_name_");
        createRbt(argc, argv, insert_size, "rbt_size_");
        createRbt(argc, argv, insert_path, "rbt_path_");
        createRbt(argc, argv, insert_hash, "rbt_hash_");
    } else {
        createRbt(argc, argv, insert_fn, prefix);
    }

    return EXIT_SUCCESS;
}
