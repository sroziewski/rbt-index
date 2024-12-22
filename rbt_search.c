#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>

// Define color constants
typedef enum { RED, BLACK } NodeColor;

// Define the FileInfo structure
typedef struct FileInfo {
    char *filename;
    size_t filesize;
    char *filepath;
    char *filetype;
} FileInfo;

// Define the Node structure
typedef struct Node {
    FileInfo key;
    NodeColor color;
    struct Node *left, *right, *parent;
} Node;

// Deserialize a FileInfo from the buffer
size_t deserialize_file_info(FileInfo *fileInfo, char *buffer) {
    size_t offset = 0;
    size_t length;

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filename = (char *)malloc(length);
    memcpy(fileInfo->filename, buffer + offset, length);
    offset += length;

    memcpy(&fileInfo->filesize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filepath = (char *)malloc(length);
    memcpy(fileInfo->filepath, buffer + offset, length);
    offset += length;

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filetype = (char *)malloc(length);
    memcpy(fileInfo->filetype, buffer + offset, length);
    offset += length;

    return offset;
}

// Deserialize a node from the buffer
Node *deserialize_node(char *buffer, size_t *currentOffset) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) return NULL;

    size_t offset = deserialize_file_info(&node->key, buffer + *currentOffset);
    *currentOffset += offset;

    memcpy(&node->color, buffer + *currentOffset, sizeof(NodeColor));
    *currentOffset += sizeof(NodeColor);

    int hasLeft, hasRight;
    memcpy(&hasLeft, buffer + *currentOffset, sizeof(int));
    *currentOffset += sizeof(int);
    memcpy(&hasRight, buffer + *currentOffset, sizeof(int));
    *currentOffset += sizeof(int);

    if (hasLeft) {
        node->left = deserialize_node(buffer, currentOffset);
        node->left->parent = node;
    } else {
        node->left = NULL;
    }

    if (hasRight) {
        node->right = deserialize_node(buffer, currentOffset);
        node->right->parent = node;
    } else {
        node->right = NULL;
    }

    return node;
}

// Recursive function to free allocated nodes
void free_node(Node *node) {
    if (node == NULL) return;
    free_node(node->left);
    free_node(node->right);
    free(node->key.filename);
    free(node->key.filepath);
    free(node->key.filetype);
    free(node);
}

// Function to search and print files with a given size and type
void search_tree_for_size_and_type(Node *root, size_t targetSize, const char *targetType) {
    if (root == NULL) {
        return;
    }

    if (root->key.filesize == targetSize && strcmp(root->key.filetype, targetType) == 0) {
        printf("Found file: %s (Size: %zu, Type: %s)\n", root->key.filename, root->key.filesize, root->key.filetype);
    }

    search_tree_for_size_and_type(root->left, targetSize, targetType);
    search_tree_for_size_and_type(root->right, targetSize, targetType);
}

void search_tree_for_name_and_type(Node *root, const char *namePattern, const char *targetType) {
    if (root == NULL) {
        return;
    }

    // Compile the regular expression for the name pattern
    regex_t regex;
    int ret = regcomp(&regex, namePattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        char errbuf[128];
        regerror(ret, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex compilation error: %s\n", errbuf);
        return;
    }

    // Check if the current node matches the name regex and file type
    if (regexec(&regex, root->key.filename, 0, NULL, 0) == 0 && strcmp(root->key.filetype, targetType) == 0) {
        printf("Found file: %s (Type: %s, Size: %zu, Path: %s)\n",
               root->key.filename, root->key.filetype, root->key.filesize, root->key.filepath);
    }

    // Free the regex memory after usage
    regfree(&regex);

    // Traverse the left and right subtrees
    search_tree_for_name_and_type(root->left, namePattern, targetType);
    search_tree_for_name_and_type(root->right, namePattern, targetType);
}

int main(int argc, char *argv[]) {
    const char *name = "shared_memory_fname_rb_tree2";
    size_t targetSize = 75988048;  // Example size to search for
    const char *targetType = "T_BINARY";  // Example file type to search for

    // Parse command line arguments
    int shouldClose = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--close") == 0) {
            shouldClose = 1;
        }
    }

    // Open shared memory
    int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        exit(EXIT_FAILURE);
    }

    // Get the size of the shared memory
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("Failed to get the shared memory size");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    void *ptr = mmap(0, shm_stat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Deserialize the tree from shared memory
    size_t offset = 0;
    Node *root = deserialize_node((char *)ptr, &offset);

    // Search for files with the specified size and type
    search_tree_for_size_and_type(root, targetSize, targetType);
    const char *targetName = "lib.*.so";
    search_tree_for_name_and_type(root, targetName, targetType);

    // Clean up
    munmap(ptr, shm_stat.st_size);
    close(shm_fd);

    // Free deserialized nodes
    free_node(root);

    // Optionally remove the shared memory
    if (shouldClose) {
        if (shm_unlink(name) == -1) {
            perror("Failed to remove shared memory");
            exit(EXIT_FAILURE);
        } else {
            printf("Shared memory successfully removed.\n");
        }
    }

    return 0;
}