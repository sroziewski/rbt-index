#ifndef RBTSEARCH_H
#define RBTSEARCH_H

typedef struct SearchArgs {
    Node *root;
    const char *namePattern;
    const char *targetType;
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

void search_tree_by_filename_and_type(Node *root, const char *namePattern, const char *targetType);

void initialize_threads();

Node *load_tree_from_shared_memory(const char *name);

int matches_pattern(const char *str, const char *namePattern);

char *convert_glob_to_regex(const char *namePattern);

#endif //RBTSEARCH_H
