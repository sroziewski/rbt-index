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

#include <ctype.h>
#include <errno.h>
#include <sys/time.h>

#include "../shared/shared.h"

int MAX_THREADS = 1;

// Global thread counter
int active_threads = 0;
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;

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
        entry->key = strdup(key); // Use strdup to copy the key
        entry->data = malloc(10 * sizeof(Node *)); // Start with an initial capacity of 10
        if (!entry->data) {
            perror("Failed to allocate initial memory for Node array in NodeHashmapEntry");
            exit(EXIT_FAILURE);
        }
        entry->data_count = 0; // No nodes added yet
        entry->capacity = 10; // Initial capacity
        HASH_ADD_STR(mapResults->entry, key, entry); // Add the entry to the hashmap
        mapResults->size++; // Increment the total number of keys in the hashmap
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
        free(entry->key); // Free key string
        free(entry->data); // Free data array
        free(entry); // Free hashmap entry
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

// Helper function to generate the regex string from a glob-style pattern
char *convert_glob_to_regex(const char *namePattern) {
    // Allocate a string for the regex (starting with double the size for safety)
    const int len = (int) strlen(namePattern);
    int allocated_size = len * 2 + 3; // Extra space for potential escapes, anchors
    char *regexPattern = malloc(allocated_size);
    if (!regexPattern) {
        perror("Failed to allocate memory for regex pattern");
        exit(EXIT_FAILURE);
    }

    // Build the regex pattern
    char *p = regexPattern;
    *p++ = '^'; // Add start anchor

    for (int i = 0; i < len; i++) {
        // Expand memory dynamically if needed (e.g., large input strings)
        if ((p - regexPattern) + 2 >= allocated_size) {
            // Ensure space for 2 extra characters
            allocated_size *= 2;
            regexPattern = realloc(regexPattern, allocated_size);
            if (!regexPattern) {
                perror("Failed to reallocate memory for regex pattern");
                exit(EXIT_FAILURE);
            }
            p = regexPattern + strlen(regexPattern); // Reset `p` after realloc
        }
        switch (namePattern[i]) {
            case '*': // Convert glob `*` to regex `.*`
                *p++ = '.';
                *p++ = '*';
                break;

            case '?': // Convert glob `?` to regex `.`
                *p++ = '.';
                break;

            case ' ': // Handle space explicitly (literal match)
                *p++ = '\\';
                *p++ = ' ';
                break;

            // Escape regex special characters
            case '.':
            case '^':
            case '$':
            case '+':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '|':
            case '\\':
            case '!':
                *p++ = '\\';
                *p++ = namePattern[i];
                break;

            default: // Default case: Add the character directly
                *p++ = namePattern[i];
                break;
        }
    }
    *p++ = '$'; // Add end anchor
    *p = '\0'; // Null-terminate the string

    return regexPattern; // Return the final regex
}

void print_node_info(const Node *node) {
    if (!node) {
        printf("Invalid node!\n");
        return;
    }
    // Print details about the node
    printf("%s: %s | Type: %s | Size: %s (%zu) | Path: %s\n",
                   (strcmp(node->key.type, "T_DIR") == 0)
                       ? "Dir"
                       : (strcmp(node->key.type, "T_LINK_DIR") == 0 || strcmp(node->key.type, "T_LINK_FILE") == 0)
                             ? "Link"
                             : "File",
                   node->key.name, node->key.type, getFileSizeAsString((long long) node->key.size), node->key.size,
                   node->key.path);
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

bool match_by_name(const char *name, char **names) {
    return matches_pattern(name, names, 1);
}

bool match_by_path(const char *path, char **paths) {
    return matches_pattern(path, paths, 1);
}

bool match_by_hash(const char *hash, char **hashes) {
    if (hash == NULL || hashes == NULL || hashes[0] == NULL) {
        return false;
    }
    if (strcmp(hash, hashes[0]) == 0) {
        return true;
    }
    return false;
}

bool match_by_size(const char *size, char **sizes) {
    if (size == NULL || sizes == NULL || sizes[0] == NULL) {
        return false;
    }
    char *endptr1;
    errno = 0;
    const size_t size_value = strtoul(size, &endptr1, 10);
    if (errno != 0 || *endptr1 != '\0') {
        return false;
    }
    char *endptr2;
    errno = 0;
    const size_t sizes_value = strtoul(sizes[0], &endptr2, 10);
    if (errno != 0 || *endptr2 != '\0') {
        return false;
    }
    return size_value == sizes_value;
}

void size_to_string(const size_t size, char *buffer, const size_t buffer_size) {
    snprintf(buffer, buffer_size, "%zu", size);
}

void search_tree(Node *root, const Arguments arguments, bool (*match_function)(const char *, char **),
                 MapResults *results, long long *totalCount) {
    if (root == NULL) {
        return;
    }

    if ((arguments.type == NULL || strcmp(root->key.type, arguments.type) == 0 || strcmp(arguments.type, "T_FILE") == 0
         && strcmp(root->key.type, "T_DIR") != 0) &&
        ((arguments.size_lower_bound == 0 || arguments.size_lower_bound > 0 && root->key.size >= arguments.
          size_lower_bound) &&
         (arguments.size_upper_bound == 0 || arguments.size_upper_bound > 0 && root->key.size <= arguments.
          size_upper_bound) ||
         (arguments.size_lower_bound <= root->key.size && root->key.size <= arguments.size_upper_bound))) {
        if (arguments.names != NULL) {
            for (int i = 0; i < arguments.names_count; ++i) {
                if (match_function(root->key.name, &arguments.names[i])) {
                    if (arguments.names_count > 1) {
                        map_results_add_node(results, root, arguments.names[i]);
                    } else {
                        print_node_info(root);
                        (*totalCount)++;
                    }
                    break;
                }
            }
        } else if (arguments.paths != NULL) {
            for (int i = 0; i < arguments.paths_count; ++i) {
                if (match_function(root->key.path, &arguments.paths[i])) {
                    if (arguments.paths_count > 1) {
                        map_results_add_node(results, root, arguments.paths[i]);
                    } else {
                        print_node_info(root);
                        (*totalCount)++;
                    }
                    break;
                }
            }
        }
        else if (arguments.hashes != NULL) {
            for (int i = 0; i < arguments.hashes_count; ++i) {
                if (match_function(root->key.hash, &arguments.hashes[i])) {
                    if (arguments.hashes_count > 1) {
                        map_results_add_node(results, root, arguments.hashes[i]);
                    } else {
                        print_node_info(root);
                        (*totalCount)++;
                    }
                    break;
                }
            }
        }
        else if (arguments.hash != NULL) {
            char *temp_array[] = {arguments.hash, NULL};
            if (match_function(root->key.hash, temp_array)) {
                print_node_info(root);
                (*totalCount)++;
            }
        } else if (arguments.size >= 0) {
            char current_node_size_str[20];
            size_to_string(root->key.size, current_node_size_str, sizeof(current_node_size_str));
            char *temp_array[] = {arguments.size_str, NULL};
            if (match_function(current_node_size_str, temp_array)) {
                print_node_info(root);
                (*totalCount)++;
            }
        } else if (arguments.size == -2) {
            print_node_info(root);
            (*totalCount)++;
        }
    }
    // Prepare threading arguments
    pthread_t leftThread, rightThread;
    SearchArgs leftArgs = {root->left, arguments, results, match_function, totalCount};
    SearchArgs rightArgs = {root->right, arguments, results, match_function, totalCount};

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
        search_tree(root->left, arguments, match_function, results, totalCount);
    }
    // Create or execute the right subtree search
    if (create_right_thread) {
        pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
    } else {
        search_tree(root->right, arguments, match_function, results, totalCount);
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
    search_tree(searchArgs->root, searchArgs->arguments, searchArgs->match_function, searchArgs->results,
                searchArgs->totalCount);
    return NULL;
}

int initialize_threads() {
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
    return 2;
}

void print_results(const MapResults *results) {
    NodeHashmapEntry *entry, *tmp;
    size_t total_nodes = 0; // To count the total number of nodes across all entries

    printf("\nResults (%zu keys):\n", results->size);
    // Traverse each entry in the hashmap
    HASH_ITER(hh, results->entry, entry, tmp) {
        printf("\nKey: %s\n", entry->key); // Print the key of the current entry
        printf("----------------------------------\n");

        // Loop through the data array of Nodes in the current entry
        size_t key_node_count = 0; // Counter for the current key's nodes
        for (size_t i = 0; i < entry->data_count; i++) {
            Node *node = entry->data[i]; // Access each Node pointer
            printf("%s: %s | Type: %s | Size: %s (%zu) | Path: %s\n",
                   (strcmp(node->key.type, "T_DIR") == 0)
                       ? "Dir"
                       : (strcmp(node->key.type, "T_LINK_DIR") == 0 || strcmp(node->key.type, "T_LINK_FILE") == 0)
                             ? "Link"
                             : "File",
                   node->key.name, node->key.type, getFileSizeAsString((long long) node->key.size), node->key.size,
                   node->key.path);

            key_node_count++; // Increment the count for this key
        }
        printf("----------------------------------\n");
        printf("Summary for key '%s': %zu nodes\n", entry->key, key_node_count);
        total_nodes += key_node_count; // Add the count for this key to the total
    }
    printf("----------------------------------\n");
    printf("Total nodes found: %zu\n", total_nodes);
}

long parse_size(const char *size_str) {
    char unit = '\0';
    long multiplier = 1;
    const size_t len = strlen(size_str);

    // Check for size suffix, ignore trailing '+' or non-numeric characters
    size_t last_digit_index = len - 1;
    while (last_digit_index > 0 && !isdigit(size_str[last_digit_index])) {
        last_digit_index--; // Find the last digit in the string
    }

    // Determine the unit if it exists (e.g., 'K', 'M', 'G')
    if (last_digit_index < len - 1) {
        unit = size_str[last_digit_index + 1];
    }

    // Determine the multiplier based on the unit
    switch (unit) {
        case 'k':
        case 'K': multiplier = 1024;
            break;
        case 'm':
        case 'M': multiplier = 1024 * 1024;
            break;
        case 'g':
        case 'G': multiplier = 1024 * 1024 * 1024;
            break;
        default: multiplier = 1;
            break;
    }

    // Create a temporary string that contains only the numeric part
    char numeric_part[20] = {0};
    strncpy(numeric_part, size_str, last_digit_index + 1);

    // Parse the numeric part of the size
    char *endptr = NULL;
    const long value = strtol(numeric_part, &endptr, 10);

    // Validate the numeric part
    if (endptr == numeric_part || *endptr != '\0') {
        fprintf(stderr, "Invalid size value: %s\n", size_str);
        exit(EXIT_FAILURE);
    }

    // Return the value multiplied by the appropriate unit
    return value * multiplier;
}

bool is_valid_type(const char *type, const char *valid_types[]) {
    for (int i = 0; valid_types[i] != NULL; i++) {
        if (strcmp(type, valid_types[i]) == 0) {
            return true;
        }
    }
    return false;
}

void *process_lines(void *arg) {
    const ThreadSearchData *data = (ThreadSearchData *)arg;
    Arguments arguments = {0};
    long long localCount = 0;

    for (size_t i = data->start; i < data->end; i++) {
        FileInfo key = {0};
        if (data->lines != NULL && data->lines[i] != NULL) {
            parseFileData(data->lines[i], &key, data->ctx);
        } else {
            fprintf(stderr, "Error: lines[%ld] is NULL\n", i);
            continue;
        }

        // Ensure `FileInfo` contains valid data
        if (key.name && key.path && key.type && key.hash) {
            const size_t length = strlen(key.hash);

            arguments.hash = (char *)malloc(length + 1); // +1 for null terminator
            if (arguments.hash == NULL) {
                perror("Failed to allocate memory for arguments.hash");
                exit(EXIT_FAILURE);
            }
            memcpy(arguments.hash, key.hash, length);
            arguments.hash[length] = '\0';

            // Call the search tree (assuming root and results are thread-safe)
            search_tree(data->root, arguments, match_by_hash, NULL, &localCount);
            free(arguments.hash);
        }
    }

    // Update the total count (atomic operation)
    pthread_mutex_lock(data->result_lock);
    *(data->totalCount) += (int)localCount;
    pthread_mutex_unlock(data->result_lock);
    printf("Thread %d: Processed chunk [%zu - %zu)\n", data->thread_id, data->start, data->end);


    return NULL;
}

void parallel_file_processing(const char *filename, void *root, const int maxThreads) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int totalCount;
    // Determine number of cores and calculate threads
    const size_t numCores = sysconf(_SC_NPROCESSORS_ONLN);
    const int numThreads = (int)(numCores / maxThreads) + 1;
    printf("Number of threads: %d\n", numThreads);

    char **lines = NULL;
    size_t numLines = 0;

    // Read lines from the file
    if (read_file_lines(filename, &lines, &numLines) != 0) {
        fprintf(stderr, "Failed to read lines from '%s'.\n", filename);
        exit(EXIT_FAILURE);
    }

    // Allocate space for threads and thread data
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * numThreads);
    if (!threads) {
        perror("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

    ThreadSearchData *threadData = (ThreadSearchData *)malloc(sizeof(ThreadSearchData) * numThreads);
    if (!threadData) {
        perror("Failed to allocate memory for thread data");
        free(threads);
        exit(EXIT_FAILURE);
    }

    // Create a mutex for synchronizing totalCount updates
    pthread_mutex_t result_lock;
    pthread_mutex_init(&result_lock, NULL);

    // Calculate workload distribution
    const size_t chunkSize = (numLines + numThreads - 1) / numThreads;

    // Create an OpenSSL hashing context
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error: Unable to create hashing context\n");
        exit(EXIT_FAILURE);
    }

    // Create threads and assign data to each thread
    for (int i = 0; i < numThreads; i++) {
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i + 1) * chunkSize < numLines ? (i + 1) * chunkSize : numLines;
        threadData[i].lines = lines;
        threadData[i].totalCount = &totalCount;
        threadData[i].root = root;
        threadData[i].result_lock = &result_lock;
        threadData[i].ctx = ctx;
        threadData[i].thread_id = i;

        if (pthread_create(&threads[i], NULL, process_lines, &threadData[i]) != 0) {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
            free(threads);
            free(threadData);
            pthread_mutex_destroy(&result_lock);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    const double elapsed = get_time_difference(start, end);

    // Output total count and execution time
    printf("Total count of processed items: %d\n", totalCount);
    printf("Execution time: %.0f seconds\n", elapsed);
    // Clean up
    pthread_mutex_destroy(&result_lock);
    free(threads);
    free(threadData);
    EVP_MD_CTX_free(ctx);
}

// Function to create a hash table
HashTable *create_hash_table(size_t size) {
    HashTable *hashTable = malloc(sizeof(HashTable));
    if (!hashTable) {
        perror("Failed to allocate memory for hash table");
        exit(EXIT_FAILURE);
    }

    hashTable->size = size;
    hashTable->table = calloc(size, sizeof(HashTableEntry *));
    if (!hashTable->table) {
        perror("Failed to allocate memory for hash table entries");
        free(hashTable);
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&hashTable->lock, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(hashTable->table);
        free(hashTable);
        exit(EXIT_FAILURE);
    }

    return hashTable;
}

// djb2 hash function
unsigned long hash_function(const char *key, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % size;
}

FileInfo *copyFileInfo(const FileInfo *source) {
    if (source == NULL) return NULL;

    FileInfo *copy = malloc(sizeof(FileInfo));
    if (!copy) {
        perror("Failed to allocate memory for FileInfo");
        return NULL;
    }
    strncpy(copy->name, source->name, sizeof(copy->name) - 1);
    copy->name[sizeof(copy->name) - 1] = '\0';
    strncpy(copy->path, source->path, sizeof(copy->path) - 1);
    copy->path[sizeof(copy->path) - 1] = '\0';
    strncpy(copy->type, source->type, sizeof(copy->type) - 1);
    copy->type[sizeof(copy->type) - 1] = '\0';
    copy->size = source->size;

    return copy;
}

// Insert into the hash table (thread-safe)
void insert_into_hash_table(HashTable *hashTable, const FileInfo *entry) {
    const unsigned long index = hash_function(entry->hash, hashTable->size);

    // Lock the table for thread safety
    pthread_mutex_lock(&hashTable->lock);

    HashTableEntry *current = hashTable->table[index];
    while (current) {
        if (strcmp(current->key, entry->hash) == 0) {
            current->count++;
            current->fileInfo = copyFileInfo(entry);
            pthread_mutex_unlock(&hashTable->lock); // Unlock before returning
            return;
        }
        current = current->next;
    }

    // If the key doesn't exist, create a new entry
    HashTableEntry *newEntry = malloc(sizeof(HashTableEntry));
    if (!newEntry) {
        perror("Failed to allocate memory for hash table entry");
        pthread_mutex_unlock(&hashTable->lock);
        exit(EXIT_FAILURE);
    }
    newEntry->key = strdup(entry->hash);
    newEntry->count = 1;
    newEntry->fileInfo = copyFileInfo(entry);
    newEntry->next = hashTable->table[index];
    hashTable->table[index] = newEntry;

    pthread_mutex_unlock(&hashTable->lock); // Unlock after inserting
}

// Free hash table resources
void free_hash_table(HashTable *hashTable) {
    for (size_t i = 0; i < hashTable->size; i++) {
        HashTableEntry *current = hashTable->table[i];
        while (current) {
            HashTableEntry *toFree = current;
            current = current->next;
            free(toFree->key);
            free(toFree->fileInfo);
            free(toFree);
        }
    }
    free(hashTable->table);
    pthread_mutex_destroy(&hashTable->lock);
    free(hashTable);
}

// Struct to pass arguments to threads
typedef struct ThreadArgs {
    Node *root;
    HashTable *hashTable;
} ThreadArgs;

// Recursive function to traverse the tree and insert hashes (with thread limiting)
void *parallel_traverse_and_insert(void *args) {
    const ThreadDuplicatesArgs *threadArgs = (ThreadDuplicatesArgs *)args;
    const Node *root = threadArgs->root;
    HashTable *hashTable = threadArgs->hashTable;
    Arguments *arguments = threadArgs->arguments;

    if (root == NULL) return NULL; // Base case: empty tree/subtree

    // Insert current node's hash into the hash table
    if (should_insert(arguments, root->key.type)) {
        insert_into_hash_table(hashTable, &root->key);
    }
    // Prepare arguments for left and right subtree threads
    pthread_t leftThread, rightThread;
    ThreadDuplicatesArgs leftArgs = {root->left, hashTable, arguments};
    ThreadDuplicatesArgs rightArgs = {root->right, hashTable, arguments};

    // Track whether threads are spawned
    bool leftThreadSpawned = false, rightThreadSpawned = false;

    // Attempt to spawn threads for left and right subtrees
    pthread_mutex_lock(&thread_lock);
    if (active_threads < MAX_THREADS) {
        active_threads++;
        pthread_mutex_unlock(&thread_lock);
        pthread_create(&leftThread, NULL, parallel_traverse_and_insert, &leftArgs);
        leftThreadSpawned = true;
    } else {
        pthread_mutex_unlock(&thread_lock);
        parallel_traverse_and_insert(&leftArgs); // Run in the current thread
    }

    pthread_mutex_lock(&thread_lock);
    if (active_threads < MAX_THREADS) {
        active_threads++;
        pthread_mutex_unlock(&thread_lock);
        pthread_create(&rightThread, NULL, parallel_traverse_and_insert, &rightArgs);
        rightThreadSpawned = true;
    } else {
        pthread_mutex_unlock(&thread_lock);
        parallel_traverse_and_insert(&rightArgs); // Run in the current thread
    }

    // Wait for the spawned threads to finish
    if (leftThreadSpawned) {
        pthread_join(leftThread, NULL);
        pthread_mutex_lock(&thread_lock);
        active_threads--;
        pthread_cond_signal(&thread_cond);
        pthread_mutex_unlock(&thread_lock);
    }
    if (rightThreadSpawned) {
        pthread_join(rightThread, NULL);
        pthread_mutex_lock(&thread_lock);
        active_threads--;
        pthread_cond_signal(&thread_cond);
        pthread_mutex_unlock(&thread_lock);
    }

    return NULL;
}

// Traverse tree in parallel and insert hash values (entry point)
void traverse_tree_in_parallel(Node *root, HashTable *hashTable, Arguments *arguments) {
    if (root == NULL) return; // If the tree is empty, there's nothing to process

    ThreadDuplicatesArgs args = {root, hashTable, arguments};
    parallel_traverse_and_insert(&args); // Start parallel traversal
}

void print_help() {
    printf("Usage: [options]\n\n");
    printf("Options:\n");
    printf("  -f <file>          Specify the memory filename (required argument).\n");
    printf("  -n <names>         Multiple names to be provided (space-separated, stop with the next argument starting with '-').\n");
    printf("  -s <size>          Specify size in bytes or size with suffix (e.g., 100, 10k, 5M). Skips processing if the next argument is '--size'.\n");
    printf("  --size <range>     Specify size range or boundary. Examples:\n");
    printf("                     - Lower bound: '10M-'\n");
    printf("                     - Upper bound: '10M'\n");
    printf("                     - Range: '10M-100M'.\n");
    printf("  -p <paths>         Multiple paths to be provided (space-separated, stop with the next argument starting with '-').\n");
    printf("  -t <type>          Specify the type. Allowed values are:\n");
    printf("                     T_DIR, T_TEXT, T_BINARY, T_IMAGE, T_JSON, T_AUDIO, T_FILM, T_COMPRESSED, T_YAML, T_EXE,\n");
    printf("                     T_C, T_PYTHON, T_JS, T_JAVA, T_LOG, T_PACKAGE, T_CLASS, T_TEMPLATE, T_PHP, T_MATHEMATICA,\n");
    printf("                     T_PDF, T_JAR, T_HTML, T_XML, T_XHTML, T_MATLAB, T_FORTRAN, T_SCIENCE, T_CPP, T_TS, T_DOC,\n");
    printf("                     T_CALC, T_LATEX, T_SQL, T_PRESENTATION, T_DATA, T_LIBRARY, T_OBJECT, T_CSV, T_CSS, T_LINK_DIR,\n");
    printf("                     T_LINK_FILE, T_FILE\n");
    printf("  -h <hash> <file> <filesize>\n");
    printf("                     Compute the hash of the specified file. Requires filename and filesize.\n");
    printf("  --help             Display this help message and exit.\n");
    exit(EXIT_SUCCESS); // Terminate the program after displaying the help message
}

void detect_duplicates(Node *root, Arguments *arguments) {
    // Create the hash table
    HashTable *hashTable = create_hash_table(INITIAL_HASH_TABLE_SIZE);
    // Traverse the tree and populate the hash table
    traverse_tree_in_parallel(root, hashTable, arguments);
    // Print the results
    printf("----------------------------------\n");
    printf("Duplicates:\n");
    for (size_t i = 0; i < hashTable->size; i++) {
        const HashTableEntry *current = hashTable->table[i];
        while (current) {
            if (current->count > 1) {
                printf("Hash: %s, Count: %d |%s|%s|%s %ld\n", current->key, current->count, current->fileInfo->type, current->fileInfo->name, current->fileInfo->path, current->fileInfo->size);
            }
            current = current->next;
        }
    }
    compute_duplicates_summary(hashTable);
    // Free the hash table
    free_hash_table(hashTable);
}

// Function to compute the number of duplicated hashes and their sum
void compute_duplicates_summary(HashTable *hashTable) {
    if (!hashTable) {
        return;
    }
    int numDuplicates = 0; // Initialize number of duplicates
    int sumCounts = 0;     // Initialize sum of duplicate counts
    // Lock the hash table for thread-safe traversal (if multi-threading is used)
    pthread_mutex_lock(&hashTable->lock);
    // Iterate through each bucket in the hash table
    for (size_t i = 0; i < hashTable->size; i++) {
        const HashTableEntry *entry = hashTable->table[i];
        // Traverse the linked list (for chains caused by collisions)
        while (entry) {
            if (entry->count > 1) {
                numDuplicates++;        // Increment the number of duplicated hashes
                sumCounts += entry->count; // Add the count to the total sum
            }
            entry = entry->next;
        }
    }
    printf("----------------------------------\n");
    printf("Found duplicated elements: %d\nSum of all duplicated element counts: %d\n", numDuplicates, sumCounts);
    // Unlock the hash table after traversal
    pthread_mutex_unlock(&hashTable->lock);
}

void free_arguments(Arguments *args) {
    if (args->names) {
        free(args->names);
        *args->names = NULL;
    }
    if (args->paths) {
        free(args->paths);
        *args->paths = NULL;
    }
    if (args->hashes) {
        free(args->hashes);
        *args->hashes = NULL;
    }
    if (args->types) {
        free(args->types);
        *args->types = NULL;
    }
    if (args->filename) {
        free(args->filename);
        args->filename = NULL;
    }
    if (args->mem_filename) {
        free(args->mem_filename);
        args->mem_filename = NULL;
    }
    if (args->hash) {
        free(args->hash);
        args->hash = NULL;
    }
    if (args->type) {
        free(args->type);
        args->type = NULL;
    }
}

bool should_insert(const Arguments *args, const char *type) {
    // 1. Insert if root.type is in args.types
    if (args->types != NULL) {
        for (int i = 0; i < args->types_count; i++) {
            if (strcmp(args->types[i], type) == 0) {
                return true; // Type is allowed
            }
        }
    }
    // 2. Insert if args.types is NULL (no restrictions)
    if (args->types == NULL) {
        return true;
    }
    // 3. Insert if there is only one element in args.types, and it equals T_FILE
    if (args->types_count == 1 && strcmp(args->types[0], "T_FILE") == 0 && strcmp(type, "T_DIR") != 0) {
        return true;
    }

    return false; // All conditions failed
}