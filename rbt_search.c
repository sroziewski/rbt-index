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

int MAX_THREADS = 1;

// Global thread counter
int active_threads = 0;
pthread_mutex_t thread_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Struct for passing arguments to threads
typedef struct SearchArgs {
    Node *root;
    const char *namePattern;
    const char *targetType;
} SearchArgs;

// Function declarations
void *search_tree_thread(void *args);
void search_tree_for_name_and_type(Node *root, const char *namePattern, const char *targetType);
void initialize_threads();

void search_tree_for_name_and_type2(Node *root, const char *namePattern, const char *targetType) {
    if (root == NULL) {
        return;
    }
    // printf("Active threads: %d\n", active_threads);
    // Compile the regular expression for the name pattern
    regex_t regex;
    const int ret = regcomp(&regex, namePattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        char errbuf[128];
        regerror(ret, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex compilation error: %s\n", errbuf);
        return;
    }

    // Check if the current node matches the name regex and file type
    if (regexec(&regex, root->key.filename, 0, NULL, 0) == 0 && strcmp(root->key.filetype, targetType) == 0) {
        // printf("Found file: %s (Type: %s, Size: %zu, Path: %s)\n",
               // root->key.filename, root->key.filetype, root->key.filesize, root->key.filepath);
    }

    // Free the regex memory after usage
    regfree(&regex);

    // Initiate thread creation or run sequentially if max threads are reached
    pthread_t leftThread, rightThread;
    SearchArgs leftArgs = { root->left, namePattern, targetType };
    SearchArgs rightArgs = { root->right, namePattern, targetType };

    int create_left_thread = 0, create_right_thread = 0;

    // Check and increment the global thread counter
    pthread_mutex_lock(&thread_counter_mutex);
    if (active_threads < MAX_THREADS) {
        active_threads++;
        create_left_thread = 1;
    }
    if (active_threads < MAX_THREADS) {
        active_threads++;
        create_right_thread = 1;
    }
    pthread_mutex_unlock(&thread_counter_mutex);

    // Create or execute the left subtree search
    if (create_left_thread) {
        pthread_create(&leftThread, NULL, search_tree_thread, &leftArgs);
    } else {
        search_tree_for_name_and_type2(root->left, namePattern, targetType);
    }

    // Create or execute the right subtree search
    if (create_right_thread) {
        pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
    } else {
        search_tree_for_name_and_type2(root->right, namePattern, targetType);
    }

    // Join threads if they were created
    if (create_left_thread) {
        pthread_join(leftThread, NULL);
        pthread_mutex_lock(&thread_counter_mutex);
        active_threads--;
        pthread_mutex_unlock(&thread_counter_mutex);
    }
    if (create_right_thread) {
        pthread_join(rightThread, NULL);
        pthread_mutex_lock(&thread_counter_mutex);
        active_threads--;
        pthread_mutex_unlock(&thread_counter_mutex);
    }
}

void *search_tree_thread(void *args) {
    SearchArgs *searchArgs = (SearchArgs *)args;
    search_tree_for_name_and_type2(searchArgs->root, searchArgs->namePattern, searchArgs->targetType);
    return NULL;
}

int main(int argc, char *argv[]) {
    struct timespec start, end;
    initialize_threads();
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
    // Capture start time
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 300; ++i) {
        search_tree_for_name_and_type2(root, targetName, targetType);
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

void initialize_threads() {
    int cores = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of cores
    if (cores <= 0) {
        perror("Failed to determine the number of processors");
        MAX_THREADS = 1;  // Fallback to a single thread if detection fails
    } else if (cores >= 14) {
        MAX_THREADS = 14; // Limit to 16 threads if 16 or more cores are available
    } else {
        MAX_THREADS = 6;  // Otherwise, use up to 8 threads
    }
    printf("Number of cores available: %d, MAX_THREADS set to: %d\n", cores, MAX_THREADS);
}