#include "rbtlib/rbtree.h"

DEFINE_COMPARATOR_BY_FIELD(filename, strcmp)

int main(const int argc, char *argv[]) {
    const char *prefix = "shared_memory_fname_";

    createRbt(argc, argv, insert_filename, prefix);

    return EXIT_SUCCESS;
}
