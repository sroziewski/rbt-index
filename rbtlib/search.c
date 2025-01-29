#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include "rbtree.h"
#include "search.h"

int MAX_THREADS = 1;

// Global thread counter
int active_threads = 0;
pthread_mutex_t thread_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void node_array_init(NodeArray *array, const size_t initial_capacity) {
    array->data = malloc(initial_capacity * sizeof(Node *));
    if (!array->data) {
        perror("Failed to allocate memory for NodeArray");
        exit(EXIT_FAILURE);
    }
    array->size = 0;
    array->capacity = initial_capacity;
}

// Free the memory used by the dynamic array
void node_array_free(NodeArray *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

// Add a node to the dynamic array
void node_array_add(NodeArray *array, Node *node) {
    if (array->size == array->capacity) {
        // Resize the array if it's full
        array->capacity *= 2;
        array->data = realloc(array->data, array->capacity * sizeof(Node *));
        if (!array->data) {
            perror("Failed to resize NodeArray");
            exit(EXIT_FAILURE);
        }
    }
    array->data[array->size++] = node;
}

Node *load_tree_from_shared_memory(const char *name) {
    // Open the shared memory object
    const int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        return NULL; // Return NULL to indicate failure
    }
    // Get the size of the shared memory object
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("Failed to get the shared memory size");
        close(shm_fd);
        return NULL;
    }
    // Map the shared memory object into the process's address space
    void *ptr = mmap(0, shm_stat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        return NULL;
    }
    // Deserialize the tree from the shared memory
    size_t offset = 0; // Tracks the current position in the byte stream
    Node *root = deserialize_node(ptr, &offset);
    // Cleanup: Unmap the shared memory and close the file descriptor
    munmap(ptr, shm_stat.st_size);
    close(shm_fd);

    return root; // Return the deserialized root node
}

// void search_tree(Node *root, const Arguments *args) {
//     if (root == NULL) {
//         return;
//     }
//     // printf("Active threads: %d\n", active_threads);
//     // Compile the regular expression for the name pattern
//     regex_t regex;
//     const int ret = regcomp(&regex, namePattern, REG_EXTENDED | REG_NOSUB);
//     if (ret != 0) {
//         char errbuf[128];
//         regerror(ret, &regex, errbuf, sizeof(errbuf));
//         fprintf(stderr, "Regex compilation error: %s\n", errbuf);
//         return;
//     }
//
//     // Check if the current node matches the name regex and file type
//     if (regexec(&regex, root->key.name, 0, NULL, 0) == 0 && strcmp(root->key.type, targetType) == 0) {
//         printf("Found file: %s (Type: %s, Size: %zu, Path: %s)\n",
//                root->key.name, root->key.type, root->key.size, root->key.path);
//     }
//
//     // Free the regex memory after usage
//     regfree(&regex);
//
//     // Initiate thread creation or run sequentially if max threads are reached
//     pthread_t leftThread, rightThread;
//     SearchArgs leftArgs = {root->left, namePattern, targetType};
//     SearchArgs rightArgs = {root->right, namePattern, targetType};
//
//     int create_left_thread = 0, create_right_thread = 0;
//
//     // Check and increment the global thread counter
//     pthread_mutex_lock(&thread_counter_mutex);
//     if (active_threads < MAX_THREADS) {
//         active_threads++;
//         create_left_thread = 1;
//     }
//     if (active_threads < MAX_THREADS) {
//         active_threads++;
//         create_right_thread = 1;
//     }
//     pthread_mutex_unlock(&thread_counter_mutex);
//
//     // Create or execute the left subtree search
//     if (create_left_thread) {
//         pthread_create(&leftThread, NULL, search_tree_thread, &leftArgs);
//     } else {
//         search_tree_by_filename_and_type(root->left, namePattern, targetType);
//     }
//
//     // Create or execute the right subtree search
//     if (create_right_thread) {
//         pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
//     } else {
//         search_tree_by_filename_and_type(root->right, namePattern, targetType);
//     }
//
//     // Join threads if they were created
//     if (create_left_thread) {
//         pthread_join(leftThread, NULL);
//         pthread_mutex_lock(&thread_counter_mutex);
//         active_threads--;
//         pthread_mutex_unlock(&thread_counter_mutex);
//     }
//     if (create_right_thread) {
//         pthread_join(rightThread, NULL);
//         pthread_mutex_lock(&thread_counter_mutex);
//         active_threads--;
//         pthread_mutex_unlock(&thread_counter_mutex);
//     }
// }

// Helper function to generate the regex string from a glob-style pattern
char *convert_glob_to_regex(const char *namePattern) {
    // Allocate a string for the regex (double the size for safety considering replacements)
    const size_t len = strlen(namePattern);
    char *regexPattern = malloc(len * 2 + 3); // Allocate for special characters + start/end anchors
    if (!regexPattern) {
        perror("Failed to allocate memory for regex pattern");
        exit(EXIT_FAILURE);
    }

    // Start building the regex pattern
    char *p = regexPattern;
    *p++ = '^'; // Add start anchor

    for (size_t i = 0; i < len; i++) {
        if (namePattern[i] == '*') {
            // Replace '*' with '.*' (regex for "zero or more characters")
            *p++ = '.';
            *p++ = '*';
        } else {
            // Escape special regex characters, if needed
            if (strchr(".^$+?()[]{}|\\", namePattern[i])) {
                *p++ = '\\'; // Add escape for special characters
            }
            *p++ = namePattern[i];
        }
    }

    *p++ = '$'; // Add end anchor
    *p = '\0';  // Null-terminate the string

    return regexPattern;
}

// Function to compile the regex and check if a string matches the given pattern
int matches_pattern(const char *str, const char *namePattern) {
    regex_t regex;

    // Convert the glob-style pattern to a regex pattern
    char *regexPattern = convert_glob_to_regex(namePattern);

    // Compile the constructed regex pattern
    const int ret = regcomp(&regex, regexPattern, REG_EXTENDED | REG_NOSUB);
    free(regexPattern); // Free the converted regex string after compiling

    if (ret != 0) {
        fprintf(stderr, "Failed to compile regex pattern: %s\n", namePattern);
        return 0; // Return false on compilation failure
    }

    // Execute regex match
    const int match = regexec(&regex, str, 0, NULL, 0) == 0;

    // Free resources used by the regex engine
    regfree(&regex);

    return match; // Return 1 if match, 0 otherwise
}

void search_tree_by_filename_and_type(Node *root, const char *namePattern, const char *targetType, NodeArray *results) {
    if (root == NULL) {
        return;
    }
    // Check if the current node matches the filename pattern and type
    if (matches_pattern(root->key.name, namePattern) && strcmp(root->key.type, targetType) == 0) {
        node_array_add(results, root);
    }
    // Prepare threading arguments
    pthread_t leftThread, rightThread;
    SearchArgs leftArgs = {root->left, namePattern, targetType, results};
    SearchArgs rightArgs = {root->right, namePattern, targetType, results};

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
        search_tree_by_filename_and_type(root->left, namePattern, targetType, results);
    }

    // Create or execute the right subtree search
    if (create_right_thread) {
        pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
    } else {
        search_tree_by_filename_and_type(root->right, namePattern, targetType, results);
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
    const SearchArgs *searchArgs = (SearchArgs *) (args);
    search_tree_by_filename_and_type(searchArgs->root, searchArgs->namePattern, searchArgs->targetType, searchArgs->results);
    return NULL;
}

void initialize_threads() {
    const long cores = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of cores
    if (cores <= 0) {
        perror("Failed to determine the number of processors");
        MAX_THREADS = 1; // Fallback to a single thread if detection fails
    } else if (cores >= 14) {
        MAX_THREADS = 14; // Limit to 16 threads if 16 or more cores are available
    } else {
        MAX_THREADS = 6; // Otherwise, use up to 8 threads
    }
    printf("Number of cores available: %ld, MAX_THREADS set to: %d\n", cores, MAX_THREADS);
}
