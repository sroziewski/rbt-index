#include "rbtlib/rbtree.h"

DEFINE_COMPARATOR_BY_FIELD(filename, strcmp)
DEFINE_COMPARATOR_BY_FIELD(filepath, strcmp)
DEFINE_NUMERIC_COMPARATOR(filesize)

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
        insert_fn = insert_filename;
    } else if (strcmp(argv[1], "--size") == 0) {
        prefix = "rbt_size_";
        insert_fn = insert_filesize;
    } else if (strcmp(argv[1], "--path") == 0) {
        prefix = "rbt_path_";
        insert_fn = insert_filepath;
    } else if (strcmp(argv[1], "--all") == 0) {
        all = true;
    } else {
        fprintf(stderr, "Invalid argument. Use --name, --size or --path <filename.lst>.\n");
        return EXIT_FAILURE;
    }

    if (all) {
        createRbt(argc, argv, insert_filename, "rbt_name_");
        createRbt(argc, argv, insert_filesize, "rbt_size_");
        createRbt(argc, argv, insert_filepath, "rbt_path_");
    } else {
        createRbt(argc, argv, insert_fn, prefix);
    }

    return EXIT_SUCCESS;
}
