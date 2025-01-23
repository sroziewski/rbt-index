#include "rbtree.h"

// Utility Functions
void remove_trailing_newline(char *str) {
    const size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Function to get file name from path
char *get_filename_from_path(const char *path) {
    const char *slash = strrchr(path, '/'); // Find the last '/' in the path
    return slash ? strdup(slash + 1) : strdup(path); // Duplicate the file name
}


// File parsing into FileInfo
FileInfo parseFileData(const char *inputLine) {
    const char *separator = "|";
    char *lineCopy = strdup(inputLine);
    FileInfo result = {NULL, 0, NULL, NULL};

    if (!lineCopy) {
        perror("Failed to duplicate the input line");
        exit(EXIT_FAILURE);
    }

    const char *start = lineCopy;

    // Extract filepath
    char *sepPos = strstr(start, separator);
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
Node *createNode(const FileInfo key, const NodeColor color, Node *parent) {
    Node *node = (Node *) malloc(sizeof(Node));
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
void freeFileInfo(const FileInfo *fileInfo) {
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

void insert(Node **root, const FileInfo key, int (*comparator)(const FileInfo *, const FileInfo *)) {
    // Allocate and initialize the new node
    Node *n = malloc(sizeof(Node));
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
        if (comparator(&n->key, &x->key) < 0) // Use comparator to determine order
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
size_t serialize_file_info(const FileInfo *fileInfo, char *buffer) {
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

// Deserialize a FileInfo from the buffer
size_t deserialize_file_info(FileInfo *fileInfo, const char *buffer) {
    size_t offset = 0;
    size_t length;

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filename = (char *) malloc(length);
    memcpy(fileInfo->filename, buffer + offset, length);
    offset += length;

    memcpy(&fileInfo->filesize, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filepath = (char *) malloc(length);
    memcpy(fileInfo->filepath, buffer + offset, length);
    offset += length;

    memcpy(&length, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);
    fileInfo->filetype = (char *) malloc(length);
    memcpy(fileInfo->filetype, buffer + offset, length);
    offset += length;

    return offset;
}

void write_tree_to_shared_memory(Node *finalRoot, const char *filePath, const char *prefix) {
    // Extract file name from the file path
    char *fileName = get_filename_from_path(filePath);
    // Allocate memory for the full shared memory name
    const size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + strlen(EXTENSION_RBT) +
                                          strlen(EXTENSION_MEM) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);
    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return;
    }
    // Construct the shared memory name
    snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s%s", prefix, fileName, EXTENSION_RBT, EXTENSION_MEM);
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
    if (ftruncate(shm_fd, (long long) alignedSize) == -1) {
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

    char *sizeStr = getFileSizeAsString(usedSize);

    const size_t fileSize = getSharedMemorySize(sharedMemoryName);
    char *memSizeStr = getFileSizeAsString(fileSize);

    printf("Red-black tree written to shared memory, rbt size: %s (%zu bytes), file %s size: %s (%zu bytes)\n", sizeStr, usedSize, sharedMemoryName, memSizeStr, fileSize);
    free(fileName);
    free(memSizeStr);
    free(sharedMemoryName);
    free(sizeStr);
}

Node *parent(const Node *n) {
    return n ? n->parent : NULL;
}

// Function to calculate the serialized size of a single FileInfo
size_t calc_file_info_size(const FileInfo *fileInfo) {
    return sizeof(size_t) + (strlen(fileInfo->filename) + 1) +
           sizeof(size_t) +
           sizeof(size_t) + (strlen(fileInfo->filepath) + 1) +
           sizeof(size_t) + (strlen(fileInfo->filetype) + 1);
}

// Function to calculate serialized size of the whole tree
size_t calc_tree_size(const Node *node) {
    if (node == NULL) {
        return 0;
    }
    const size_t size = calc_file_info_size(&node->key) + sizeof(NodeColor) + 2 * sizeof(int);
    return size + calc_tree_size(node->left) + calc_tree_size(node->right);
}

char *getFileSizeAsString(const size_t fileSizeBytesIn) {
    const double fileSizeBytes = (double) fileSizeBytesIn;
    const double kB = 1024.0;
    const double MB = 1024.0 * 1024.0;
    const double GB = 1024.0 * 1024.0 * 1024.0;
    char *result = malloc(20 * sizeof(char)); // Allocate memory for the result

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

void inorder(const Node *node) {
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
char *add_rbt_extension(const char *filename) {
    const size_t len = strlen(filename);
    const size_t extLen = strlen(EXTENSION_RBT);

    // Check if the filename already ends with '.rbt'
    if (len >= extLen && strcmp(filename + len - extLen, EXTENSION_RBT) == 0) {
        return strdup(filename); // Return a copy of the original filename
    }

    // Allocate memory for the new filename (original + ".rbt")
    char *newFilename = malloc(len + extLen + 1);
    if (!newFilename) {
        perror("Failed to allocate memory for filename");
        exit(EXIT_FAILURE);
    }

    // Append the ".rbt" extension
    strcpy(newFilename, filename);
    strcat(newFilename, EXTENSION_RBT);

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
        fprintf(stderr, "Error: Serialized size (%zu bytes) exceeds expected size (%zu bytes)!\n", usedSize,
                requiredSize);
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
    char *sizeStr = getFileSizeAsString(usedSize);
    printf("Red-black tree successfully written to file '%s', size: %s (%zu bytes)\n", filename, sizeStr, usedSize);
    free(sizeStr); // Free after use
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
    const int hasLeft = node->left != NULL;
    const int hasRight = node->right != NULL;
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
    const size_t extensionLen = strlen(EXTENSION_RBT);
    const int hasExtension = (fileNameLen >= extensionLen) && (
                                 strcmp(&fileName[fileNameLen - extensionLen], EXTENSION_RBT) == 0);
    // Allocate memory for the full shared memory name
    const size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + (hasExtension ? 0 : extensionLen) +
                                          strlen(EXTENSION_MEM) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);
    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return;
    }
    // Construct the shared memory name
    if (hasExtension) {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s", prefix, fileName, EXTENSION_MEM);
        // Don't append .rbt
    } else {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s%s", prefix, fileName, EXTENSION_RBT,
                 EXTENSION_MEM); // Append .rbt
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
    if (ftruncate(shm_fd, (long long) fileSize) == -1) {
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
    char *sizeStr = getFileSizeAsString(fileSize);
    printf("Serialized red-black tree successfully stored in shared memory %s, size: %s (%zu bytes)\n",
           sharedMemoryName, sizeStr, fileSize);
    munmap(sharedMemoryPtr, fileSize); // Unmap shared memory
    close(shm_fd); // Close shared memory file descriptor
    free(buffer); // Free the temporary buffer
    free(fileName);
    free(sharedMemoryName);
    free(sizeStr);
}

int remove_shared_memory_object(char **argv, const char *prefix) {
    char *fileName = get_filename_from_path(argv[3]);
    const size_t fileNameLen = strlen(fileName);
    const size_t extensionLen = strlen(EXTENSION_RBT);

    // Check if the file already has the ".rbt" extension
    const int hasExtension = (fileNameLen >= extensionLen) &&
                             (strcmp(&fileName[fileNameLen - extensionLen], EXTENSION_RBT) == 0);

    // Allocate shared memory name
    const size_t sharedMemoryNameLength = strlen(prefix) + strlen(fileName) + (hasExtension ? 0 : extensionLen) +
                                          strlen(EXTENSION_MEM) + 1;
    char *sharedMemoryName = malloc(sharedMemoryNameLength);

    if (!sharedMemoryName) {
        perror("Failed to allocate memory for shared memory name");
        free(fileName);
        return EXIT_FAILURE;
    }

    // Construct the shared memory name
    if (hasExtension) {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s", prefix, fileName, EXTENSION_MEM);
    } else {
        snprintf(sharedMemoryName, sharedMemoryNameLength, "%s%s%s%s", prefix, fileName, EXTENSION_RBT, EXTENSION_MEM);
    }
    const size_t fileSize = getSharedMemorySize(sharedMemoryName);
    char *sizeStr = getFileSizeAsString(fileSize);
    if (shm_unlink(sharedMemoryName) == 0) {
        printf("Shared memory object %s successfully removed, size: %s (%zu bytes)\n", sharedMemoryName, sizeStr,
               fileSize);
    } else {
        perror("Error removing shared memory object");
    }
    // Cleanup
    free(sharedMemoryName);
    free(fileName);
    free(sizeStr);
    return EXIT_SUCCESS;
}

int remove_shared_memory_object_by_name(const char *sharedMemoryName) {
    // Check if the shared memory name is valid
    if (!sharedMemoryName || strlen(sharedMemoryName) == 0) {
        fprintf(stderr, "Invalid shared memory name.\n");
        return EXIT_FAILURE;
    }
    const size_t fileSize = getSharedMemorySize(sharedMemoryName); // Shared memory size
    char *sizeStr = getFileSizeAsString(fileSize);
    if (shm_unlink(sharedMemoryName) == 0) {
        printf("Shared memory object %s successfully removed, size: %s (%zu bytes)\n", sharedMemoryName, sizeStr,
               fileSize);
    } else {
        perror("Error removing shared memory object");
    }
    free(sizeStr);

    return EXIT_SUCCESS;
}

int compareByFilename(const FileInfo *a, const FileInfo *b) {
    return strcmp(a->filename, b->filename);
}

int compareByFilesize(const FileInfo *a, const FileInfo *b) {
    return (a->filesize > b->filesize) - (a->filesize < b->filesize);
}

// Deserialize a node from the buffer
Node *deserialize_node(char *buffer, size_t *currentOffset) {
    Node *node = malloc(sizeof(Node));
    if (node == NULL) return NULL;

    const size_t offset = deserialize_file_info(&node->key, buffer + *currentOffset);
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
    const int ret = regcomp(&regex, namePattern, REG_EXTENDED | REG_NOSUB);
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

void listSharedMemoryEntities(const char *prefix) {
    const char *shmDir = "/dev/shm"; // POSIX shared memory location
    DIR *dir = opendir(shmDir);

    if (dir == NULL) {
        perror("Failed to open shared memory directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry starts with the given prefix
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            printf("Shared memory entity: %s\n", entry->d_name);
        }
    }

    closedir(dir);
}

void createRbt(const int argc, char *argv[], void (*insertFunc)(Node **, FileInfo), const char *prefix) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [--save] [--load filename.rbt] [--clean file.lst] [--list] [--remove sharedMemoryFilename]\n", argv[0]);
        return;
    }
    if (argc == 4 && strcmp(argv[2], "--load") == 0) {
        // Handle the --load command
        read_tree_from_file_to_shared_memory(argv[3], prefix);
        return;
    }
    if (argc == 4 && strcmp(argv[2], "--clean") == 0) {
        // Handle the --clean command
        remove_shared_memory_object(argv, prefix);
        return;
    }
    if (argc == 3 && strcmp(argv[1], "--list") == 0) {
        // Handle the --list command
        listSharedMemoryEntities(prefix);
        return;
    }
    if (argc == 4 && strcmp(argv[2], "--remove") == 0) {
        // Handle the --remove command
        remove_shared_memory_object_by_name(argv[3]);
        return;
    }
    // Handle the normal processing and storing workflow
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char **lines = NULL;
    size_t numLines = 0;
    char buffer[8 * MAX_LINE_LENGTH];
    Node *finalRoot = NULL; // Red-Black Tree node

    // Reading lines from the file
    while (fgets(buffer, sizeof(buffer), file)) {
        remove_trailing_newline(buffer);
        if (*buffer) {
            lines = realloc(lines, sizeof(char *) * ++numLines);
            if (!lines) {
                perror("Failed to allocate memory for lines");
                fclose(file);
                return;
            }
            lines[numLines - 1] = strdup(buffer);
            if (!lines[numLines - 1]) {
                perror("Failed to duplicate buffer");
                fclose(file);
                return;
            }
        }
    }
    fclose(file); // Close file after use

    int totalProcessedCount = 0;

    // Processing the lines to insert into the Red-Black Tree
    for (size_t i = 0; i < numLines; i++) {
        FileInfo key = { NULL, 0, NULL, NULL };
        if (lines != NULL && lines[i] != NULL) {
            key = parseFileData(lines[i]);
        } else {
            fprintf(stderr, "Error: lines[%ld] is NULL\n", i);
            continue;
        }

        // Ensure `FileInfo` contains valid data and insert into the Tree
        if (key.filename && key.filepath && key.filetype) {
            insertFunc(&finalRoot, key); // Use the provided insertion function
            totalProcessedCount++;
        }
    }

    // Display the processed files in sorted Red-Black Tree order
    printf("\nFiles stored in Red-Black Tree in sorted order by filename:\n");
    inorder(finalRoot);

    printf("Total lines successfully processed: %d\n", totalProcessedCount);

    // Free the allocated memory for lines
    for (size_t i = 0; i < numLines; i++) {
        if (lines != NULL && lines[i] != NULL) {
            free(lines[i]);
        }
    }
    free(lines);

    // Handle saving to file or shared memory
    if (argc == 3 && strcmp(argv[2], "--save") == 0) {
        char *storeFilename = add_rbt_extension(argv[1]);  // Append `.rbt` to the filename
        write_tree_to_file(finalRoot, storeFilename);
        free(storeFilename);
    } else {
        write_tree_to_shared_memory(finalRoot, argv[1], prefix);
    }

    // Free the tree if it was created or loaded
    freeTree(finalRoot);
}

size_t getSharedMemorySize(const char *sharedMemoryName) {
    // Open the shared memory object
    const int shm_fd = shm_open(sharedMemoryName, O_RDONLY, 0);
    if (shm_fd == -1) {
        perror("Failed to open shared memory object");
        exit( EXIT_FAILURE);
    }

    // Get the metadata about the shared memory object
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("Failed to get shared memory object size");
        close(shm_fd);
        return 0; // Return 0 to indicate failure
    }

    // Close the shared memory file descriptor
    close(shm_fd);

    // Return the size of the shared memory object
    return shm_stat.st_size;
}