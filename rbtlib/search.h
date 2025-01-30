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
    char **names;
    int names_count;
    int size;
    char *path;
    char *type;
    char *hash;
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
} SearchArgs;

void *search_tree_thread(void *args);

void search_tree_by_filename_and_type(Node *root, Arguments arguments, MapResults *results);

void initialize_threads();

Node *load_tree_from_shared_memory(const char *name);

int matches_pattern(const char *str, char **names, int names_count);

char *convert_glob_to_regex(const char *namePattern);

void map_results_add_node(MapResults *mapResults, Node *node, const char *key);

void node_array_free(NodeArray *array);

void cleanup_map_results(MapResults *mapResults);

void print_results(const MapResults *results);

#endif //RBTSEARCH_H
