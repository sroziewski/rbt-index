#ifndef RBTSEARCH_H
#define RBTSEARCH_H

typedef struct {
    Node **data;
    size_t size;
    size_t capacity;
} NodeArray;

typedef struct SearchArgs {
    Node *root;
    const char *namePattern;
    const char *targetType;
    NodeArray *results;
} SearchArgs;

typedef struct {
    char *mem_filename;
    char *name;
    int size;
    char *path;
    char *type;
    char *hash;
} Arguments;

void *search_tree_thread(void *args);

void search_tree_by_filename_and_type(Node *root, const char *namePattern, const char *targetType, NodeArray *results);

void initialize_threads();

Node *load_tree_from_shared_memory(const char *name);

int matches_pattern(const char *str, const char *namePattern);

char *convert_glob_to_regex(const char *namePattern);

void node_array_init(NodeArray *array, size_t initial_capacity);

void node_array_free(NodeArray *array);

#endif //RBTSEARCH_H
