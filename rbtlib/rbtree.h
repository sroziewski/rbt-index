#ifndef RBTREE_H
#define RBTREE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>

#include "../shared/lconsts.h"

// Enum to represent node colors
typedef enum { RED, BLACK } NodeColor;

// File information structure
typedef struct FileInfo {
    char name[LINK_LENGTH];
    size_t size;
    char path[MAX_LINE_LENGTH];
    char type[MAX_TYPE_LENGTH];
    char linkTarget[LINK_LENGTH];
    char hash[33];
    size_t childrenCount;
    bool isHidden;
    bool isDir;
    int isLink; // 1 is link to file, 2 link to directory
} FileInfo;

// Node structure for the Red-Black Tree
typedef struct Node {
    FileInfo key;
    NodeColor color;
    struct Node *left, *right, *parent;
} Node;

typedef struct {
    const char *prefix;
    void (*insert_fn)(Node **root, FileInfo key);
    bool all;
    bool skipCheck;
    bool save;
    const char *filename;
} Config;

#define EXTENSION_RBT ".rbt"
#define EXTENSION_MEM ".mem"

#define ROTATE_LEFT(root, n)              \
    do {                                  \
        Node *r = (n)->right;             \
        (n)->right = r->left;             \
        if (r->left)                      \
            r->left->parent = (n);        \
        r->parent = (n)->parent;          \
        if (!(n)->parent)                 \
            *(root) = r;                  \
        else if ((n) == (n)->parent->left)\
            (n)->parent->left = r;        \
        else                              \
            (n)->parent->right = r;       \
        r->left = (n);                    \
        (n)->parent = r;                  \
    } while (0)

#define ROTATE_RIGHT(root, n)             \
    do {                                  \
        Node *l = (n)->left;              \
        (n)->left = l->right;             \
        if (l->right)                     \
            l->right->parent = (n);       \
        l->parent = (n)->parent;          \
        if (!(n)->parent)                 \
            *(root) = l;                  \
        else if ((n) == (n)->parent->left)\
            (n)->parent->left = l;        \
        else                              \
            (n)->parent->right = l;       \
        l->right = (n);                   \
        (n)->parent = l;                  \
    } while (0)

#define DEFINE_COMPARATOR_BY_FIELD(FIELD, CMP_FUNC)                       \
int compareBy##FIELD(const FileInfo *a, const FileInfo *b) {              \
return CMP_FUNC(a->FIELD, b->FIELD);                                  \
}                                                                         \
void insert_##FIELD(Node **root, const FileInfo key) {                    \
insert(root, key, compareBy##FIELD);                                  \
}

#define DEFINE_NUMERIC_COMPARATOR(FIELD)                                \
int compareBy##FIELD(const FileInfo *a, const FileInfo *b) {            \
return (a->FIELD > b->FIELD) - (a->FIELD < b->FIELD);               \
}                                                                       \
void insert_##FIELD(Node **root, const FileInfo key) {                  \
insert(root, key, compareBy##FIELD);                                \
}
// Function declarations

// Memory and Tree Management
Node *createNode(FileInfo key, NodeColor color, Node *parent);

void freeTree(Node *node);

void freeFileInfo(FileInfo *fileInfo);

Node *deserialize_node(char *buffer, size_t *currentOffset);

void freeNode(Node *node);

void search_tree_for_size_and_type(Node *root, size_t targetSize, const char *targetType);

static inline Node *grandparent(Node *n) {
    return (n && n->parent) ? n->parent->parent : NULL;
}

static inline Node *uncle(Node *n) {
    Node *g = grandparent(n);
    if (!g) return NULL;
    if (n->parent == g->left)
        return g->right;
    return g->left;
}

// Red-Black Tree Operations
void insert(Node **root, const FileInfo key, int (*comparator)(const FileInfo *, const FileInfo *));

void inorder(const Node *node);

void insert_rebalance(Node **root, Node *n);

int compareByFilename(const FileInfo *a, const FileInfo *b);

int compareByFilesize(const FileInfo *a, const FileInfo *b);

void search_tree_for_name_and_type(Node *root, const char *namePattern, const char *targetType);

// Rotation Operations
void rotate_left(Node **root, Node *n);

void rotate_right(Node **root, Node *n);

// Serialization and Deserialization
size_t serialize_file_info(const FileInfo *fileInfo, char *buffer);

size_t deserialize_file_info(FileInfo *fileInfo, const char *buffer);

long long serialize_node(Node *node, char *buffer);

size_t calc_file_info_size(const FileInfo *fileInfo);

long long calc_tree_size(const Node *node);

size_t serialize_file_info(const FileInfo *fileInfo, char *buffer);

// File Operations
void store_rbt_to_file(Node *root, const char *filename);

void write_tree_to_file(Node *finalRoot, const char *filename);

Node *load_rbt_from_file(const char *filename);

void read_tree_from_file_to_shared_memory(char *filename, const char *prefix);

// Shared Memory Operations
void write_tree_to_shared_memory(Node *finalRoot, const char *filePath, const char *prefix);

int remove_shared_memory_object(char **argv, const char *prefix);

int remove_shared_memory_object_by_name(const char *sharedMemoryName);

// Utility Functions

void remove_trailing_newline(char *str);

void parseFileData(const char *inputLine, FileInfo *result, EVP_MD_CTX *ctx);

char *add_rbt_extension(const char *filename);

char *get_filename_from_path(const char *path);

void listSharedMemoryEntities(const char *prefix);

void createRbt(const int argc, char *argv[], void (*insertFunc)(Node **, FileInfo), const char *prefix, Config config);

long long getSharedMemorySize(const char *sharedMemoryName);

void compute_md5(const char *input, char *output);

void to_lowercase(const wchar_t *input, wchar_t *output);

void concatenate_name_and_size(const FileInfo *result, char *out);

void convert_char_to_wchar(const char *input, wchar_t *output, size_t output_size);

void convert_wchar_to_char(const wchar_t *input, char *output, size_t output_size);

void sha256_first_64bits_to_hex(const char *input, char *output_hex, EVP_MD_CTX *ctx);

void concatenate_strings(const char *string1, const char *string2, char *output);

#endif // RBTREE_H
