#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include "rbtlib/rbtree.h"
#include "rbtlib/search.h"

int main(const int argc, char *argv[]) {
    struct timespec start, end;
    initialize_threads();
    char *name;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            name = strdup(argv[i + 1]);
            break;
        }
    }

    const size_t targetSize = 2340;  // Example size to search for
    const char *targetType = "T_FILM";  // Example file type to search for

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

    const char *targetName = "zaba.*";
    // Capture start time
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1; ++i) {
        search_tree_by_filename_and_type(root, targetName, targetType);
        // Print progress every 100 iterations
        if (i % 100 == 0) {
            printf("Completed %d iterations...\n", i);
        }
    }
    // Capture end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    // Calculate elapsed time in seconds
    double elapsed_time = (end.tv_sec - start.tv_sec) +
                          (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Elapsed time: %.9f seconds\n", elapsed_time);
    // Clean up
    munmap(ptr, shm_stat.st_size);
    close(shm_fd);

    // Free deserialized nodes
    freeNode(root);

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
