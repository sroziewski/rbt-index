#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// Utility Functions
void remove_trailing_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Function to get file name from path
char* get_filename_from_path(const char* path) {
    const char *slash = strrchr(path, '/');  // Find the last '/' in the path
    return slash ? strdup(slash + 1) : strdup(path);  // Duplicate the file name
}


// File parsing into FileInfo
FileInfo parseFileData(const char *inputLine) {
    const char *separator = "<SEP>";
    char *lineCopy = strdup(inputLine);
    FileInfo result = {NULL, 0, NULL, NULL};

    if (!lineCopy) {
        perror("Failed to duplicate the input line");
        exit(EXIT_FAILURE);
    }

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

// Tree Node Allocation
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

// Free FileInfo
void freeFileInfo(FileInfo *fileInfo) {
    free(fileInfo->filename);
    free(fileInfo->filepath);
    free(fileInfo->filetype);
}

// Free Tree
void freeTree(Node *node) {
    if (!node) return;

    freeTree(node->left);
    freeTree(node->right);
    freeFileInfo(&node->key);
    free(node);
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

// Serializing and Deserializing implementations
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

size_t deserialize_file_info(FileInfo *fileInfo, const char *buffer) {
    if (!fileInfo || !buffer) {
        printf("Error: null FileInfo or buffer.\n");
        return 0;
    }

    size_t offset = 0;

    // Deserialize and debug filename
    printf("Deserializing filename...\n");
    size_t filenameLength = strlen(buffer + offset);
    fileInfo->filename = (char *)malloc(filenameLength + 1);
    if (!fileInfo->filename) {
        printf("Error: malloc failed for filename.\n");
        return 0;
    }
    strcpy(fileInfo->filename, buffer + offset);
    offset += filenameLength + 1;

    // Deserialize and debug filesize
    printf("Deserializing filesize...\n");
    memcpy(&fileInfo->filesize, buffer + offset, sizeof(fileInfo->filesize));
    offset += sizeof(fileInfo->filesize);

    // Deserialize and debug filepath
    printf("Deserializing filepath...\n");
    size_t filepathLength = strlen(buffer + offset);
    fileInfo->filepath = (char *)malloc(filepathLength + 1);
    if (!fileInfo->filepath) {
        printf("Error: malloc failed for filepath.\n");
        free(fileInfo->filename); // Free previously allocated memory
        return 0;
    }
    strcpy(fileInfo->filepath, buffer + offset);
    offset += filepathLength + 1;

    // Deserialize and debug filetype
    printf("Deserializing filetype...\n");
    size_t filetypeLength = strlen(buffer + offset);
    fileInfo->filetype = (char *)malloc(filetypeLength + 1);
    if (!fileInfo->filetype) {
        printf("Error: malloc failed for filetype.\n");
        free(fileInfo->filename); // Free previously allocated memory
        free(fileInfo->filepath);
        return 0;
    }
    strcpy(fileInfo->filetype, buffer + offset);
    offset += filetypeLength + 1;

    printf("Deserialization complete. Total bytes read: %zu\n", offset);
    return offset;
}

void write_tree_to_shared_memory(Node *finalRoot, const char *filePath, const char *prefix) {
    // Extract file name from the file path
    char *fileName = get_filename_from_path(filePath);
    // Allocate memory for the full shared memory name
    size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + strlen(EXTENSION) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);
    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return;
    }
    // Construct the shared memory name
    snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s", prefix, fileName, EXTENSION);
    // Calculate the size needed for serialization
    const size_t requiredSize = calc_tree_size(finalRoot);

    // Align size to system page size
    const long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pageSize <= 0) {
        perror("Failed to get page size");
        exit(EXIT_FAILURE);
    }

    const size_t alignedSize = ((requiredSize + pageSize - 1) / pageSize) * pageSize; // Align to page size

    // Create buffer for serialized tree (ensure it's still aligned)
    char *buffer = aligned_alloc(pageSize, alignedSize);
    if (buffer == NULL) {
        perror("Failed to allocate aligned buffer");
        exit(EXIT_FAILURE);
    }

    // Serialize the tree into the buffer
    const size_t usedSize = serialize_node(finalRoot, buffer);

    if (usedSize > alignedSize) {
        fprintf(stderr, "Serialized size exceeds aligned size!\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Create shared memory
    const int shm_fd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666); // Read-write permissions
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

    printf("Red-black tree written to shared memory, size: %s (%zu bytes) %s\n", getFileSizeAsString(usedSize), usedSize, sharedMemoryName);
    free(fileName);
    free(sharedMemoryName);
}

Node* parent(Node* n) {
    return n ? n->parent : NULL;
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

void inorder(Node* node) {
    if (node != NULL) {
        inorder(node->left);
        printf("Filename: %s, Size: %zu bytes, Path: %s, Type: %s\n",
               node->key.filename, node->key.filesize, node->key.filepath,
               node->key.filetype);
        inorder(node->right);
    }
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

void write_tree_to_file(Node *finalRoot, const char *filename) {
    // Calculate the size needed for serialization
    const size_t requiredSize = calc_tree_size(finalRoot);

    // Open the file for writing in binary mode
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error: Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    // Create a buffer to hold the serialized tree data
    char *buffer = malloc(requiredSize);
    if (!buffer) {
        perror("Error: Failed to allocate memory for serialization buffer");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Serialize the tree into the buffer
    const size_t usedSize = serialize_node(finalRoot, buffer);

    // Ensure the serialized size does not exceed the calculated size
    if (usedSize > requiredSize) {
        fprintf(stderr, "Error: Serialized size (%zu bytes) exceeds expected size (%zu bytes)!\n", usedSize, requiredSize);
        free(buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Write the serialized data to the file
    if (fwrite(buffer, 1, usedSize, file) != usedSize) {
        perror("Error: Failed to write serialized data to file");
        free(buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Cleanup resources
    free(buffer);
    fclose(file);

    printf("Red-black tree successfully written to file '%s', size: %zu bytes\n", filename, usedSize);
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

void read_tree_from_file_to_shared_memory(char *filePath, const char *prefix) {
    // Open the file for reading in binary mode
    char *fileName = get_filename_from_path(filePath);
    // Allocate memory for the full shared memory name
    const size_t fileNameLen = strlen(fileName);
    const size_t extensionLen = strlen(EXTENSION);
    const int hasExtension = (fileNameLen >= extensionLen) && (strcmp(&fileName[fileNameLen - extensionLen], EXTENSION) == 0);
    // Allocate memory for the full shared memory name
    const size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + (hasExtension ? 0 : extensionLen) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);
    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return;
    }
    // Construct the shared memory name
    if (hasExtension) {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s", prefix, fileName); // Don't append .rbt
    } else {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s", prefix, fileName, EXTENSION); // Append .rbt
    }

    FILE *file = fopen(filePath, "rb");
    if (!file) {
        perror("Error: Failed to open file for reading");
        exit(EXIT_FAILURE);
    }

    // Determine the size of the serialized data
    fseek(file, 0, SEEK_END);
    const size_t fileSize = ftell(file); // Total size of the file
    if (fileSize == 0) {
        fprintf(stderr, "Error: File is empty.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    rewind(file);

    printf("Reading serialized data from file '%s', size: %zu bytes\n", filePath, fileSize);

    // Allocate a buffer to hold the serialized data
    char *buffer = malloc(fileSize);
    if (!buffer) {
        perror("Error: Failed to allocate memory for serialized data buffer");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Read the serialized data into the buffer
    if (fread(buffer, 1, fileSize, file) != fileSize) {
        perror("Error: Failed to read serialized data from file");
        free(buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file); // File read is complete

    // Set up shared memory
    const int shm_fd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666); // Open shared memory
    if (shm_fd == -1) {
        perror("Error: Failed to create shared memory object");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Resize the shared memory to hold the serialized data
    if (ftruncate(shm_fd, fileSize) == -1) {
        perror("Error: Failed to resize shared memory");
        free(buffer);
        close(shm_fd);
        shm_unlink(sharedMemoryName); // Cleanup the shared memory object
        exit(EXIT_FAILURE);
    }

    // Memory map the shared memory object
    void *sharedMemoryPtr = mmap(0, fileSize, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMemoryPtr == MAP_FAILED) {
        perror("Error: Failed to map shared memory");
        free(buffer);
        close(shm_fd);
        shm_unlink(sharedMemoryName); // Cleanup the shared memory object
        exit(EXIT_FAILURE);
    }

    // Copy serialized data from the buffer into shared memory
    memcpy(sharedMemoryPtr, buffer, fileSize);

    // Cleanup
    printf("Serialized data successfully stored in shared memory '%s' (size: %zu bytes)\n", sharedMemoryName, fileSize);
    munmap(sharedMemoryPtr, fileSize); // Unmap shared memory
    close(shm_fd); // Close shared memory file descriptor
    free(buffer); // Free the temporary buffer
    free(fileName);
    free(sharedMemoryName);
}

int remove_shared_memory_object(char **argv, const char *prefix) {
    char *fileName = get_filename_from_path(argv[2]);
    const char *extension = ".rbt";
    const size_t fileNameLen = strlen(fileName);
    const size_t extensionLen = strlen(extension);

    // Check if the file already has the ".rbt" extension
    int hasExtension = (fileNameLen >= extensionLen) &&
                       (strcmp(&fileName[fileNameLen - extensionLen], extension) == 0);

    // Allocate shared memory name
    const size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + (hasExtension ? 0 : extensionLen) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);

    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return EXIT_FAILURE;
    }

    // Construct the shared memory name
    if (hasExtension) {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s", prefix, fileName);
    } else {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s", prefix, fileName, extension);
    }

    // Remove the shared memory object
    if (shm_unlink(sharedMemoryName) == 0) {
        printf("Shared memory object '%s' successfully removed.\n", sharedMemoryName);
    } else {
        perror("Error removing shared memory object");
    }

    // Cleanup
    free(sharedMemoryName);
    free(fileName);
    return EXIT_SUCCESS;
}