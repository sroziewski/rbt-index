#ifndef RBTSEARCH_H
#define RBTSEARCH_H

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

typedef struct SearchArgs {
    Node *root;
    Arguments arguments;
    NodeArray *results;
} SearchArgs;

void *search_tree_thread(void *args);

void search_tree_by_filename_and_type(Node *root, const Arguments arguments, NodeArray *results);

void initialize_threads();

Node *load_tree_from_shared_memory(const char *name);

int matches_pattern(const char *str, char **names, int names_count);

char *convert_glob_to_regex(const char *namePattern);

void node_array_init(NodeArray *array, size_t initial_capacity);

void node_array_free(NodeArray *array);

#endif //RBTSEARCH_H
