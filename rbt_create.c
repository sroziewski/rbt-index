#include "rbtlib/rbtree.h"
#include "shared/shared.h"

DEFINE_COMPARATOR_BY_FIELD(name, strcmp)
DEFINE_COMPARATOR_BY_FIELD(path, strcmp)
DEFINE_NUMERIC_COMPARATOR(size)

/**
 * The main function serves as the program's entry point. It initializes and
 * invokes the creation process of a red-black with key as a filename tree by calling createRbt.
 *
 * @param argc The number of command-line arguments provided to the program.
 * @param argv The array of command-line arguments. The first element is the program name.
 * @return Returns EXIT_SUCCESS to indicate successful program termination.
 */
int main(const int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: --name, --size or --path <filename.lst>\n");
        return EXIT_FAILURE;
    }

    bool all = false;
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
    } else if (strcmp(argv[1], "--all") == 0) {
        all = true;
    } else {
        fprintf(stderr, "Invalid argument. Use --name, --size or --path <filename.lst>.\n");
        return EXIT_FAILURE;
    }
    if (argc == 3 && strcmp(argv[1], "--list") == 0) {
        listSharedMemoryEntities(prefix);
        exit(EXIT_SUCCESS);
    }
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

    if (all) {
        createRbt(argc, argv, insert_name, "rbt_name_");
        createRbt(argc, argv, insert_size, "rbt_size_");
        createRbt(argc, argv, insert_path, "rbt_path_");
    } else {
        createRbt(argc, argv, insert_fn, prefix);
    }

    return EXIT_SUCCESS;
}
