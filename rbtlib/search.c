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

void map_results_add_node(MapResults *mapResults, Node *node, const char *key) {
    NodeHashmapEntry *entry;
    // Check if the key exists in the hashmap
    HASH_FIND_STR(mapResults->entry, key, entry);
    // If the entry does not exist, create a new one
    if (!entry) {
        entry = malloc(sizeof(NodeHashmapEntry));
        if (!entry) {
            perror("Failed to allocate memory for NodeHashmapEntry");
            exit(EXIT_FAILURE);
        }
        // Initialize the new NodeHashmapEntry
        entry->key = strdup(key);           // Use strdup to copy the key
        entry->data = malloc(10 * sizeof(Node *));  // Start with an initial capacity of 10
        if (!entry->data) {
            perror("Failed to allocate initial memory for Node array in NodeHashmapEntry");
            exit(EXIT_FAILURE);
        }
        entry->data_count = 0;             // No nodes added yet
        entry->capacity = 10;              // Initial capacity
        HASH_ADD_STR(mapResults->entry, key, entry); // Add the entry to the hashmap
        mapResults->size++;                // Increment the total number of keys in the hashmap
    }
    // If the entry exists or is newly created, add the Node to its data array
    if (entry->data_count == entry->capacity) {
        // Resize the data array if full
        entry->capacity *= 2;
        entry->data = realloc(entry->data, entry->capacity * sizeof(Node *));
        if (!entry->data) {
            perror("Failed to resize Node array in NodeHashmapEntry");
            exit(EXIT_FAILURE);
        }
    }
    // Add the Node to the data array
    entry->data[entry->data_count++] = node;
}

void cleanup_map_results(MapResults *mapResults) {
    NodeHashmapEntry *entry, *tmp;
    // Free all hashmap entries
    HASH_ITER(hh, mapResults->entry, entry, tmp) {
        HASH_DEL(mapResults->entry, entry); // Remove from hashmap
        free(entry->key);                  // Free key string
        free(entry->data);                 // Free data array
        free(entry);                       // Free hashmap entry
    }
    mapResults->size = 0;
}

// Free the memory used by the dynamic array
void node_array_free(NodeArray *array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
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

int matches_pattern(const char *str, char **names, const int names_count) {
    // Create a buffer to hold the full regex pattern for all names
    size_t total_length = 0;
    for (int i = 0; i < names_count; i++) {
        char *glob = convert_glob_to_regex(names[i]);
        total_length += strlen(glob) + 3; // Include space for '|', parentheses, or '\0'.
        free(glob); // Free the temporary converted regex string
    }
    // Allocate memory for the combined regex pattern
    char *regexPattern = malloc(total_length + 3); // Additional space for start/end parenthesis
    if (!regexPattern) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    // Construct the full regex: Combine all names into a single OR regex, e.g., "(name1|name2|name3)"
    strcpy(regexPattern, "(");
    for (int i = 0; i < names_count; i++) {
        char *glob = convert_glob_to_regex(names[i]);
        strcat(regexPattern, glob); // Append each regex converted glob
        free(glob); // Free memory from each converted glob
        if (i < names_count - 1) {
            strcat(regexPattern, "|"); // Add OR operator between patterns
        }
    }
    strcat(regexPattern, ")"); // Close the parentheses
    // Compile the constructed regex pattern
    regex_t regex;
    const int ret = regcomp(&regex, regexPattern, REG_EXTENDED | REG_NOSUB);
    free(regexPattern); // Free the combined regex pattern after compiling
    if (ret != 0) {
        fprintf(stderr, "Failed to compile regex pattern.\n");
        return 0; // Return false on compilation failure
    }
    // Execute regex match
    const int match = regexec(&regex, str, 0, NULL, 0) == 0;
    // Free resources used by the regex engine
    regfree(&regex);

    return match;
}

void search_tree_by_filename_and_type(Node *root, const Arguments arguments, MapResults *results) {
    if (root == NULL) {
        return;
    }
    if (arguments.type == NULL || strcmp(root->key.type, arguments.type) == 0) { // Check type condition
        for (int i = 0; i < arguments.names_count; ++i) {
            if (matches_pattern(root->key.name, &arguments.names[i], 1)) { // Check for a match
                map_results_add_node(results, root, arguments.names[i]);
                break;
            }
        }
    }
    // Prepare threading arguments
    pthread_t leftThread, rightThread;
    SearchArgs leftArgs = {root->left, arguments, results};
    SearchArgs rightArgs = {root->right, arguments, results};

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
        search_tree_by_filename_and_type(root->left, arguments, results);
    }
    // Create or execute the right subtree search
    if (create_right_thread) {
        pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
    } else {
        search_tree_by_filename_and_type(root->right, arguments, results);
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
    search_tree_by_filename_and_type(searchArgs->root, searchArgs->arguments, searchArgs->results);
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

void print_results(const MapResults *results) {
    NodeHashmapEntry *entry, *tmp;
    printf("\nResults (%zu keys):\n", results->size);
    // Traverse each entry in the hashmap
    HASH_ITER(hh, results->entry, entry, tmp) {
        printf("\nKey: %s\n\n", entry->key); // Print the key of the current entry
        // Loop through the data array of Nodes in the current entry
        for (size_t i = 0; i < entry->data_count; i++) {
            Node *node = entry->data[i]; // Access each Node pointer
            printf("  File: %s | Type: %s | Size: %zu | Path: %s\n",
                node->key.name, node->key.type, node->key.size, node->key.path);
        }
    }
}
