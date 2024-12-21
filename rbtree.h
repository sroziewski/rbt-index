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

// Enum to represent node colors
typedef enum { RED, BLACK } NodeColor;

// File information structure
typedef struct FileInfo {
    char *filename;
    size_t filesize;
    char *filepath;
    char *filetype;
} FileInfo;

// Node structure for the Red-Black Tree
typedef struct Node {
    FileInfo key;
    NodeColor color;
    struct Node *left, *right, *parent;
} Node;

#define EXTENSION ".rbt"

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

// Function declarations

// Memory and Tree Management
Node* createNode(FileInfo key, NodeColor color, Node* parent);
void freeTree(Node *node);
void freeFileInfo(FileInfo *fileInfo);

static inline Node* grandparent(Node *n) {
    return (n && n->parent) ? n->parent->parent : NULL;
}

static inline Node* uncle(Node *n) {
    Node *g = grandparent(n);
    if (!g) return NULL;
    if (n->parent == g->left)
        return g->right;
    return g->left;
}

// Red-Black Tree Operations
void insert(Node** root, FileInfo key);
void inorder(Node* node);
void insert_rebalance(Node **root, Node *n);

// Rotation Operations
void rotate_left(Node** root, Node* n);
void rotate_right(Node** root, Node* n);

// Serialization and Deserialization
size_t serialize_file_info(FileInfo *fileInfo, char *buffer);
size_t deserialize_file_info(FileInfo *fileInfo, const char *buffer);
size_t serialize_node(Node *node, char *buffer);
size_t calc_file_info_size(FileInfo *fileInfo);
size_t calc_tree_size(Node *node);
Node *deserialize_node(char *buffer, size_t *currentOffset);
size_t serialize_file_info(FileInfo *fileInfo, char *buffer);

// File Operations
void store_rbt_to_file(Node *root, const char *filename);
void write_tree_to_file(Node *finalRoot, const char *filename);
Node* load_rbt_from_file(const char *filename);
void read_tree_from_file_to_shared_memory(const char *filename, const char *sharedMemoryName);

// Shared Memory Operations
void write_tree_to_shared_memory(Node *finalRoot);

// Utility Functions
void remove_trailing_newline(char *str);
FileInfo parseFileData(const char *inputLine);
char* add_rbt_extension(const char *filename);
char* getFileSizeAsString(double fileSizeBytes);
char* get_filename_from_path(const char* path);


#endif // RBTREE_H