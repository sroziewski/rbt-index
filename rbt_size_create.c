#include "rbtlib/rbtree.h"

DEFINE_NUMERIC_COMPARATOR(filesize)

int main(const int argc, char *argv[]) {
    const char *prefix = "shared_memory_fsize_";

    createRbt(argc, argv, insert_filesize, prefix);

    return EXIT_SUCCESS;
}
