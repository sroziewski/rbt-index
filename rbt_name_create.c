#include "rbtlib/rbtree.h"

DEFINE_COMPARATOR_BY_FIELD(name, strcmp)

/**
 * The main function serves as the program's entry point. It initializes and
 * invokes the creation process of a red-black with key as a filename tree by calling createRbt.
 *
 * @param argc The number of command-line arguments provided to the program.
 * @param argv The array of command-line arguments. The first element is the program name.
 * @return Returns EXIT_SUCCESS to indicate successful program termination.
 */
int main(const int argc, char *argv[]) {
    const char *prefix = "rbt_name_";

    createRbt(argc, argv, insert_name, prefix);

    return EXIT_SUCCESS;
}
