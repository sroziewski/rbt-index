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

    const char *prefix = NULL;

    // Check for command line arguments
    if (strcmp(argv[1], "--name") == 0) {
        prefix = "rbt_name_";
        createRbt(argc, argv, insert_filename, prefix);
    } else if (strcmp(argv[1], "--size") == 0) {
        prefix = "rbt_size_";
        createRbt(argc, argv, insert_filesize, prefix);
    }
    else if (strcmp(argv[1], "--path") == 0) {
        prefix = "rbt_path_";
        createRbt(argc, argv, insert_filepath, prefix);
    }
    else {
        fprintf(stderr, "Invalid argument. Use --name, --size or --path <filename.lst>.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
