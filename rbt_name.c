#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum { RED, BLACK } NodeColor;

typedef struct FileInfo {
    char *filename;
    ssize_t filesize;
    char *filepath;
    char *filetype;
} FileInfo;

typedef struct Node {
    FileInfo key;
    NodeColor color;
    struct Node *left, *right, *parent;
} Node;

#define EXTENSION ".rbt"

// Function prototype to store Red-Black Tree in a file
void store_rbt_to_file(Node *root, const char *filename);
char* add_rbt_extension(const char *filename);

// Function prototypes
Node* createNode(FileInfo key, NodeColor color, Node* parent);
void rotate_left(Node** root, Node* n);
void rotate_right(Node** root, Node* n);
void insert_rebalance(Node **root, Node *n);
void insert_balance(Node** root, Node* n); // Combines balancing cases 1-5
void insert(Node** root, FileInfo key);   // Optimized insert
void inorder(Node* node);
void freeTree(Node *node);
void remove_trailing_newline(char *str);
FileInfo parseFileData(const char *inputLine);
size_t calc_file_info_size(FileInfo *fileInfo);
size_t calc_tree_size(Node *node);
size_t serialize_file_info(FileInfo *fileInfo, char *buffer);
size_t serialize_node(Node *node, char *buffer);
void write_tree_to_shared_memory(Node *finalRoot);
char* getFileSizeAsString(double fileSizeBytes);
Node* load_rbt_from_file(const char *filename);
Node *deserialize_node(char *buffer, size_t *currentOffset);
size_t deserialize_file_info(FileInfo *fileInfo, const char *buffer);
void serialize_node_to_file(Node *node, FILE *file);
// Use macros to simplify rotation operations
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

static inline Node* grandparent(Node *n) {
    return (n && n->parent) ? n->parent->parent : NULL;
}

static inline Node* uncle(Node *n) {
    Node *g = grandparent(n);
    if (!g) return NULL;
    if (n->parent == g->left)
        return g->right;
    else
        return g->left;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [--store] [--load filename]\n", argv[0]);
        return EXIT_FAILURE;
    }

    Node *finalRoot = NULL;

    if (argc == 3 && strcmp(argv[1], "--load") == 0) {
        // Handle the --load command
        // finalRoot = load_rbt_from_file(argv[2]);
        // if (!finalRoot) {
            // fprintf(stderr, "Failed to load Red-Black Tree from file: %s\n", argv[2]);
            // return EXIT_FAILURE;
        // }

        printf("Red-Black Tree successfully loaded from file: %s\n", argv[2]);
        printf("Files stored in Red-Black Tree in sorted order by filename:\n");
        write_tree_to_shared_memory(finalRoot);
    } else {
        // Handle the normal processing and storing workflow
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        char **lines = NULL;
        size_t numLines = 0;
        char buffer[8 * 1024];

        while (fgets(buffer, sizeof(buffer), file)) {
            remove_trailing_newline(buffer);
            if (*buffer) {
                lines = realloc(lines, sizeof(char *) * ++numLines);
                if (!lines) {
                    perror("Failed to allocate memory for lines");
                    fclose(file);
                    return EXIT_FAILURE;
                }
                lines[numLines - 1] = strdup(buffer);
                if (!lines[numLines - 1]) {
                    perror("Failed to duplicate buffer");
                    fclose(file);
                    return EXIT_FAILURE;
                }
            }
        }
        fclose(file);

        int totalProcessedCount = 0;

        for (size_t i = 0; i < numLines; i++) {
            FileInfo key = parseFileData(lines[i]);
            if (key.filename && key.filepath && key.filetype) {
                insert(&finalRoot, key);
                totalProcessedCount++;
            }
        }

        printf("\nFiles stored in Red-Black Tree in sorted order by filename:\n");
        inorder(finalRoot);

        printf("Total lines successfully processed: %d\n", totalProcessedCount);

        for (size_t i = 0; i < numLines; i++) {
            free(lines[i]);
        }
        free(lines);

        // Check if the program is run with the --store option
        if (argc == 3 && strcmp(argv[2], "--store") == 0) {
            char *storeFilename = add_rbt_extension(argv[1]);  // Use the input file's name as the base and append `.rbt`
            store_rbt_to_file(finalRoot, storeFilename);
            printf("Red-Black Tree has been stored in file: %s\n", storeFilename);
            free(storeFilename);
        }
        else {
            write_tree_to_shared_memory(finalRoot);
        }
    }

    // Free the tree if it was loaded or created
    freeTree(finalRoot);

    return EXIT_SUCCESS;
}

char* getFileSizeAsString(double fileSizeBytes) {
    const double kB = 1024.0;
    const double MB = 1024.0 * 1024.0;
    const double GB = 1024.0 * 1024.0 * 1024.0;
    char* result = (char*)malloc(20 * sizeof(char));  // Allocate memory for the result

    if (fileSizeBytes >= GB) {
        snprintf(result, 20, "%.2f GB", fileSizeBytes / GB);
    } else if (fileSizeBytes >= MB) {
        snprintf(result, 20, "%.2f MB", fileSizeBytes / MB);
    } else if (fileSizeBytes >= kB) {
        snprintf(result, 20, "%.2f kB", fileSizeBytes / kB);
    } else {
        snprintf(result, 20, "%.2f bytes", fileSizeBytes);
    }

    return result;
}

void write_tree_to_shared_memory(Node *finalRoot) {
    const char *name = "shared_memory_fname_rb_tree";

    // Calculate the size needed for serialization
    size_t requiredSize = calc_tree_size(finalRoot);

    // Align size to system page size
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pageSize <= 0) {
        perror("Failed to get page size");
        exit(EXIT_FAILURE);
    }

    size_t alignedSize = ((requiredSize + pageSize - 1) / pageSize) * pageSize; // Align to page size

    // Create buffer for serialized tree (ensure it's still aligned)
    char *buffer = (char *)aligned_alloc(pageSize, alignedSize);
    if (buffer == NULL) {
        perror("Failed to allocate aligned buffer");
        exit(EXIT_FAILURE);
    }

    // Serialize the tree into the buffer
    size_t usedSize = serialize_node(finalRoot, buffer);

    if (usedSize > alignedSize) {
        fprintf(stderr, "Serialized size exceeds aligned size!\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Create shared memory
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); // Read-write permissions
    if (shm_fd == -1) {
        perror("Failed to create shared memory object");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Set the size of shared memory to the aligned size
    if (ftruncate(shm_fd, alignedSize) == -1) {
        perror("Failed to set shared memory size");
        free(buffer);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Map shared memory to process space
    void *ptr = mmap(0, alignedSize, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        free(buffer);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Copy serialized tree to shared memory
    memcpy(ptr, buffer, usedSize);

    // Cleanup
    if (munmap(ptr, alignedSize) == -1) {
        perror("Failed to unmap shared memory");
    }
    close(shm_fd);
    free(buffer);

    printf("Red-black tree written to shared memory, size: %s (%zu bytes) %s\n", getFileSizeAsString(usedSize), usedSize, name);
}

// Function to calculate the serialized size of a single FileInfo
size_t calc_file_info_size(FileInfo *fileInfo) {
    return sizeof(size_t) + (strlen(fileInfo->filename) + 1) +
           sizeof(size_t) +
           sizeof(size_t) + (strlen(fileInfo->filepath) + 1) +
           sizeof(size_t) + (strlen(fileInfo->filetype) + 1);
}

// Function to calculate serialized size of the whole tree
size_t calc_tree_size(Node *node) {
    if (node == NULL) {
        return 0;
    }
    size_t size = calc_file_info_size(&node->key) + sizeof(NodeColor) + 2 * sizeof(int);
    return size + calc_tree_size(node->left) + calc_tree_size(node->right);
}

// Function to serialize FileInfo
size_t serialize_file_info(FileInfo *fileInfo, char *buffer) {
    size_t offset = 0;
    size_t length;

    length = strlen(fileInfo->filename) + 1;
    memcpy(buffer + offset, &length, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, fileInfo->filename, length);
    offset += length;

    memcpy(buffer + offset, &fileInfo->filesize, sizeof(size_t));
    offset += sizeof(size_t);

    length = strlen(fileInfo->filepath) + 1;
    memcpy(buffer + offset, &length, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, fileInfo->filepath, length);
    offset += length;

    length = strlen(fileInfo->filetype) + 1;
    memcpy(buffer + offset, &length, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, fileInfo->filetype, length);
    offset += length;

    return offset;
}

// Function to serialize a single node
size_t serialize_node(Node *node, char *buffer) {
    if (node == NULL) {
        return 0;
    }

    size_t offset = 0;
    offset += serialize_file_info(&node->key, buffer);

    memcpy(buffer + offset, &node->color, sizeof(NodeColor));
    offset += sizeof(NodeColor);

    // Mark non-null nodes
    int hasLeft = node->left != NULL;
    int hasRight = node->right != NULL;
    memcpy(buffer + offset, &hasLeft, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &hasRight, sizeof(int));
    offset += sizeof(int);

    // Serialize child nodes (left and right)
    if (node->left) {
        offset += serialize_node(node->left, buffer + offset);
    }
    if (node->right) {
        offset += serialize_node(node->right, buffer + offset);
    }

    return offset;
}

void remove_trailing_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

char* get_filename_from_path(const char* path) {
    const char *slash = strrchr(path, '/');
    return slash ? strdup(slash + 1) : strdup(path);
}

FileInfo parseFileData(const char *inputLine) {
    const char *separator = "<SEP>";
    char *lineCopy = strdup(inputLine);
    FileInfo result = {NULL, 0, NULL, NULL};

    if (!lineCopy) {
        perror("Failed to duplicate the input line");
        exit(EXIT_FAILURE);
    }

    char *token;
    char *start = lineCopy;
    char *sepPos;

    // Extract filepath
    sepPos = strstr(start, separator);
    if (sepPos) {
        *sepPos = '\0';
        result.filepath = strdup(start);
        result.filename = get_filename_from_path(start);
        start = sepPos + strlen(separator);
    }

    // Extract filesize
    sepPos = strstr(start, separator);
    if (sepPos) {
        *sepPos = '\0';
        result.filesize = strtoul(start, NULL, 10);
        start = sepPos + strlen(separator);
    }

    // Extract filetype
    if (*start) {
        result.filetype = strdup(start);
    }

    free(lineCopy);
    return result;
}

Node* createNode(FileInfo key, NodeColor color, Node* parent) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    node->key = key;
    node->left = node->right = NULL;
    node->parent = parent;
    node->color = color;
    return node;
}

Node* parent(Node* n) {
    return n ? n->parent : NULL;
}

void rotate_left(Node** root, Node* n) {
    Node* r = n->right;
    n->right = r->left;
    if (r->left) r->left->parent = n;
    r->parent = n->parent;
    if (!n->parent)
        *root = r;
    else if (n == n->parent->left)
        n->parent->left = r;
    else
        n->parent->right = r;
    r->left = n;
    n->parent = r;
}

void rotate_right(Node** root, Node* n) {
    Node* l = n->left;
    n->left = l->right;
    if (l->right) l->right->parent = n;
    l->parent = n->parent;
    if (!n->parent)
        *root = l;
    else if (n == n->parent->left)
        n->parent->left = l;
    else
        n->parent->right = l;
    l->right = n;
    n->parent = l;
}

void insert(Node **root, FileInfo key) {
    // Allocate and initialize the new node
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    n->key = key;
    n->color = RED;
    n->left = n->right = n->parent = NULL;

    // Standard BST insertion (now comparing filenames)
    Node *y = NULL;
    Node *x = *root;

    while (x != NULL) {
        y = x;
        if (strcmp(n->key.filename, x->key.filename) < 0)
            x = x->left;
        else
            x = x->right;
    }

    n->parent = y;
    if (y == NULL) {
        *root = n; // Tree was empty
    } else if (strcmp(n->key.filename, y->key.filename) < 0) {
        y->left = n;
    } else {
        y->right = n;
    }

    // Rebalance the tree
    insert_rebalance(root, n);
}

void insert_rebalance(Node **root, Node *n) {
    while (n->parent && n->parent->color == RED) {
        Node *g = grandparent(n);
        if (n->parent == g->left) {
            Node *u = g->right; // Uncle
            if (u && u->color == RED) {
                // Case 1: Recoloring
                n->parent->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                n = g;
            } else {
                if (n == n->parent->right) {
                    // Case 2: Left rotation
                    n = n->parent;
                    ROTATE_LEFT(root, n);
                }
                // Case 3: Right rotation
                n->parent->color = BLACK;
                g->color = RED;
                ROTATE_RIGHT(root, g);
            }
        } else {
            Node *u = g->left; // Uncle
            if (u && u->color == RED) {
                n->parent->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                n = g;
            } else {
                if (n == n->parent->left) {
                    n = n->parent;
                    ROTATE_RIGHT(root, n);
                }
                n->parent->color = BLACK;
                g->color = RED;
                ROTATE_LEFT(root, g);
            }
        }
    }
    (*root)->color = BLACK; // Ensure root remains black
}

void inorder(Node* node) {
    if (node != NULL) {
        inorder(node->left);
        printf("Filename: %s, Size: %zu bytes, Path: %s, Type: %s\n",
               node->key.filename, node->key.filesize, node->key.filepath,
               node->key.filetype);
        inorder(node->right);
    }
}

void freeFileInfo(FileInfo *fileInfo) {
    free(fileInfo->filename);
    free(fileInfo->filepath);
    free(fileInfo->filetype);
}

void freeTree(Node *node) {
    if (!node) return;

    freeTree(node->left);
    freeTree(node->right);

    freeFileInfo(&node->key);
    free(node);
}

/**
 * Add the .rbt extension to the filename if it does not already have it.
 */
char* add_rbt_extension(const char *filename) {
    const size_t len = strlen(filename);
    const size_t extLen = strlen(EXTENSION);

    // Check if the filename already ends with '.rbt'
    if (len >= extLen && strcmp(filename + len - extLen, EXTENSION) == 0) {
        return strdup(filename);  // Return a copy of the original filename
    }

    // Allocate memory for the new filename (original + ".rbt")
    char *newFilename = malloc(len + extLen + 1);
    if (!newFilename) {
        perror("Failed to allocate memory for filename");
        exit(EXIT_FAILURE);
    }

    // Append the ".rbt" extension
    strcpy(newFilename, filename);
    strcat(newFilename, EXTENSION);

    return newFilename;
}

void store_rbt_to_file(Node *root, const char *filename) {
    if (!root) {
        fprintf(stderr, "Error: Tree is empty. Nothing to store.\n");
        return;
    }

    // Open the file for writing
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    // Serialize the entire tree starting from the root
    serialize_node_to_file(root, file);

    // Close the file
    if (fclose(file) != 0) {
        perror("Error closing file");
    }
}

// Helper function moved outside of main function
void serialize_node_to_file(Node *node, FILE *file) {
    if (!node) return;

    char buffer[8 * 1024];
    const size_t size = serialize_file_info(&node->key, buffer);
    fwrite(buffer, 1, size, file);

    fwrite(&node->color, sizeof(NodeColor), 1, file);

    const int hasLeft = node->left != NULL;
    const int hasRight = node->right != NULL;

    fwrite(&hasLeft, sizeof(int), 1, file);
    fwrite(&hasRight, sizeof(int), 1, file);

    if (node->left) serialize_node_to_file(node->left, file);
    if (node->right) serialize_node_to_file(node->right, file);
}
