#include "rbtlib/rbtree.h"

DEFINE_NUMERIC_COMPARATOR(size)

/**
 * @brief Entry point function for the program that initializes and creates a red-black tree (RBT) with key as a file size using provided arguments.
 *
 * @param argc The number of arguments passed to the program.
 * @param argv An array of character pointers listing all the arguments provided to the program.
 * @return An exit status indicating the success or failure of the program execution.
 *         Returns EXIT_SUCCESS on successful execution.
 */
int main(const int argc, char *argv[]) {
    const char *prefix = "shared_memory_fsize_";

    createRbt(argc, argv, insert_filesize, prefix);

    return EXIT_SUCCESS;
}
