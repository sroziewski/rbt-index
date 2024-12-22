#include "rbtlib/rbtree.h"

int main(int argc, char *argv[]) {
    const char *name = "shared_memory_fname_playground.lst.rbt.mem";
    size_t targetSize = 75988048;  // Example size to search for
    const char *targetType = "T_DIR";  // Example file type to search for

    // Parse command line arguments
    int shouldClose = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--close") == 0) {
            shouldClose = 1;
        }
    }

    // Open shared memory
    const int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        exit(EXIT_FAILURE);
    }

    // Get the size of the shared memory
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("Failed to get the shared memory size");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    void *ptr = mmap(0, shm_stat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Deserialize the tree from shared memory
    size_t offset = 0;
    Node *root = deserialize_node(ptr, &offset);

    // Search for files with the specified size and type
    search_tree_for_size_and_type(root, targetSize, targetType);
    const char *targetName = "externalsites";
    search_tree_for_name_and_type(root, targetName, targetType);

    // Clean up
    munmap(ptr, shm_stat.st_size);
    close(shm_fd);

    // Free deserialized nodes
    free_node(root);

    // Optionally remove the shared memory
    if (shouldClose) {
        if (shm_unlink(name) == -1) {
            perror("Failed to remove shared memory");
            exit(EXIT_FAILURE);
        }
        printf("Shared memory successfully removed.\n");
    }

    return 0;
}