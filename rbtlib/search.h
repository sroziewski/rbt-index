#ifndef RBTSEARCH_H
#define RBTSEARCH_H

#include "../include/uthash.h"

typedef struct {
    Node **data;
    size_t size;
    size_t capacity;
} NodeArray;

typedef struct {
    char *mem_filename;
    char *filename;
    char **names;
    int names_count;
    char **hashes;
    int hashes_count;
    int size;
    char *size_str;
    size_t size_lower_bound;
    size_t size_upper_bound;
    char **paths;
    int paths_count;
    char *type;
    char *hash;
    bool duplicates;
} Arguments;

typedef struct NodeHashmapEntry {
    char *key;            // The key for the hashmap (e.g., filename, or any criteria)
    Node **data;          // The value (an array of Node pointers)
    size_t data_count;    // Number of nodes in the array
    size_t capacity;      // Capacity of the array for dynamic resizing
    UT_hash_handle hh;    // Handle for uthash
} NodeHashmapEntry;

typedef struct {
    NodeHashmapEntry *entry; // Hashmap of NodeHashmapEntry structs
    size_t size;               // Total number of keys in the hashmap
} MapResults;

typedef struct SearchArgs {
    Node *root;                // The root of the tree to search
    Arguments arguments;       // Arguments provided to the search
    MapResults *results;      // The hashmap to store search results
    bool (*match_function)(const char *, char **);
    long long *totalCount;
} SearchArgs;

typedef struct {
    size_t start;
    size_t end;
    char **lines;
    int *totalCount;
    void *root;
    pthread_mutex_t *result_lock;
    void *ctx;
    int thread_id;
} ThreadSearchData;

#define INITIAL_HASH_TABLE_SIZE 4096
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct HashTableEntry {
    char *key; // The hash string (e.g., from FileInfo)
    int count;
    FileInfo *fileInfo;
    struct HashTableEntry *next;
} HashTableEntry;

typedef struct HashTable {
    size_t size;
    HashTableEntry **table;
    pthread_mutex_t lock;     // Mutex for thread-safe insertion
} HashTable;

// Struct to pass arguments to threads
typedef struct ThreadDuplicatesArgs {
    Node *root;
    HashTable *hashTable;
} ThreadDuplicatesArgs;

void *search_tree_thread(void *args);

void search_tree_by_filename_and_type(Node *root, Arguments arguments, MapResults *results);

int initialize_threads();

Node *load_tree_from_shared_memory(const char *name);

int matches_pattern(const char *str, char **names, int names_count);

char *convert_glob_to_regex(const char *namePattern);

void map_results_add_node(MapResults *mapResults, Node *node, const char *key);

void node_array_free(NodeArray *array);

void cleanup_map_results(MapResults *mapResults);

void print_results(const MapResults *results);

void search_tree(Node *root, Arguments arguments, bool (*match_function)(const char *, char **), MapResults *results, long long *totalCount);

bool match_by_name(const char *name, char **names);

bool match_by_path(const char *path, char **paths);

long parse_size(const char *size_str);

bool match_by_hash(const char *hash, char **hashes);

void size_to_string(size_t size, char *buffer, size_t buffer_size);

bool match_by_size(const char *size, char **sizes);

bool is_valid_type(const char *type, const char *valid_types[]);

void *process_lines(void *arg);

void parallel_file_processing(const char *filename, void *root, int maxThreads);

HashTable *create_hash_table(size_t size);

void traverse_tree_in_parallel(Node *root, HashTable *hashTable);

void free_hash_table(HashTable *hashTable);

void print_help();

void detect_duplicates(Node *root);

void compute_duplicates_summary(HashTable *hashTable);

void free_arguments(const Arguments *args);

#endif //RBTSEARCH_H
