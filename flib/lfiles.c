#include "lfiles.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <magic.h>
#include <pthread.h>
#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <omp.h>
#include <stdbool.h>
#include <unistd.h>

#include "../shared/lconsts.h"

const char *FILE_TYPES[] = {
    "T_DIR", "T_TEXT", "T_BINARY", "T_IMAGE", "T_JSON", "T_AUDIO", "T_FILM",
    "T_COMPRESSED", "T_YAML", "T_EXE", "T_C", "T_PYTHON", "T_JS",
    "T_JAVA", "T_LOG", "T_PACKAGE", "T_CLASS", "T_TEMPLATE",
    "T_PDF", "T_JAR", "T_HTML", "T_XML", "T_XHTML",
    "T_TS", "T_DOC", "T_CALC", "T_LATEX", "T_SQL",
    "T_CSV", "T_CSS"
};

// Check if the file has the .json extension (case insensitive)
int isJsonFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".json") == 0;
}

// Check if the file has the .yaml or .yml extension (case insensitive)
int isYamlFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && (strcasecmp(dot, ".yaml") == 0 || strcasecmp(dot, ".yml") == 0);
}

int isExeFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".exe") == 0;
}

int isAudioFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".mp3") == 0 || dot && strcasecmp(dot, ".wav") == 0 || dot && strcasecmp(dot, ".ogg")
           == 0;
}

int isImageFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".png") == 0 || dot && strcasecmp(dot, ".jpg") == 0 || dot && strcasecmp(dot, ".gif")
           == 0 || dot && strcasecmp(dot, ".jpeg") == 0 || dot && strcasecmp(dot, ".bmp") == 0 || dot &&
           strcasecmp(dot, ".tiff") == 0 || dot && strcasecmp(dot, ".webp") == 0
           || dot && strcasecmp(dot, ".svg") == 0 || dot && strcasecmp(dot, ".raw") == 0 || dot &&
           strcasecmp(dot, ".psd") == 0 || dot && strcasecmp(dot, ".eps") == 0 || dot && strcasecmp(dot, ".ico") == 0;
}

int isFilmFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".mp4") == 0 || dot && strcasecmp(dot, ".avi") == 0 || dot && strcasecmp(dot, ".mkv")
           == 0 || dot && strcasecmp(dot, ".mov") == 0 || dot && strcasecmp(dot, ".wmv") == 0 || dot &&
           strcasecmp(dot, ".flv") == 0 || dot && strcasecmp(dot, ".mpg") == 0
           || dot && strcasecmp(dot, ".mpeg") == 0 || dot && strcasecmp(dot, ".3gp") == 0 || dot &&
           strcasecmp(dot, ".webm") == 0 || dot && strcasecmp(dot, ".vob") == 0 || dot && strcasecmp(dot, ".mov") == 0
           || dot && strcasecmp(dot, ".mxf") == 0 || dot && strcasecmp(dot, ".divx") == 0 || dot &&
           strcasecmp(dot, ".asf") == 0;
}

// Check if the file has a .c extension (case insensitive)
int isCFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".c") == 0;
}

// Check if the file has a .py extension (case insensitive)
int isPythonFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".py") == 0;
}

// Check if the file has a .java extension (case insensitive)
int isJavaFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".java") == 0;
}

// Check if the file has a .java extension (case insensitive)
int isCompressedFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".7z") == 0 || dot && strcasecmp(dot, ".tbz") == 0 || dot && strcasecmp(dot, ".bz2")
           == 0 || dot && strcasecmp(dot, ".tar") == 0 || dot && strcasecmp(dot, ".arj") == 0;
}

int isPackageFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".deb") == 0 || dot && strcasecmp(dot, ".rpm") == 0;
}

int isLogFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".log") == 0;
}

int isClassFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".class") == 0;
}

int isTemplateFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".tpl") == 0;
}

int isPdfFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".pdf") == 0;
}

int isJarFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".jar") == 0;
}

int isHtmlFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".html") == 0;
}

int isXmlFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".xml") == 0;
}

int isXhtmlFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".xhtml") == 0;
}

int isTsFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".ts") == 0;
}

int isJsFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".js") == 0;
}

int isTexFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".tex") == 0;
}

int isDocFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".doc") == 0 || dot && strcasecmp(dot, ".rtf") == 0 || dot && strcasecmp(dot, ".docx")
           == 0;
}

int isSqlFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".sql") == 0 || dot && strcasecmp(dot, ".psql") == 0 || dot && strcasecmp(dot, ".hql")
           == 0;
}

int isCalcFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".ods") == 0 || dot && strcasecmp(dot, ".xls") == 0 || dot && strcasecmp(dot, ".xlsx")
           == 0;
}

int isCsvFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".csv") == 0;
}

int isCssFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".css") == 0 || dot && strcasecmp(dot, ".scss") == 0;
}

const char *getFileTypeCategory(const char *mimeType, const char *filePath) {
    struct stat pathStat;
    if (stat(filePath, &pathStat) == 0 && S_ISDIR(pathStat.st_mode)) {
        return "T_DIR"; // Add classification for directories
    }
    if (strstr(mimeType, "text/") == mimeType) {
        if (isJsonFile(filePath)) {
            return "T_JSON";
        }
        if (isYamlFile(filePath)) {
            return "T_YAML";
        }
        if (isCFile(filePath)) {
            return "T_C";
        }
        if (isPythonFile(filePath)) {
            return "T_PYTHON";
        }
        if (isJavaFile(filePath)) {
            return "T_JAVA";
        }
        if (isCompressedFile(filePath)) {
            return "T_COMPRESSED";
        }
        if (isLogFile(filePath)) {
            return "T_LOG";
        }
        if (isTemplateFile(filePath)) {
            return "T_TEMPLATE";
        }
        if (isHtmlFile(filePath)) {
            return "T_HTML";
        }
        if (isXmlFile(filePath)) {
            return "T_XML";
        }
        if (isXhtmlFile(filePath)) {
            return "T_XHTML";
        }
        if (isTsFile(filePath)) {
            return "T_TS";
        }
        if (isJsFile(filePath)) {
            return "T_JS";
        }
        if (isDocFile(filePath)) {
            return "T_DOC";
        }
        if (isTexFile(filePath)) {
            return "T_LATEX";
        }
        if (isSqlFile(filePath)) {
            return "T_SQL";
        }
        if (isCssFile(filePath)) {
            return "T_CSS";
        }
        if (isCsvFile(filePath)) {
            return "T_CSV";
        }
        return "T_TEXT";
    }
    if (strstr(mimeType, "image/") == mimeType || isImageFile(filePath)) {
        return "T_IMAGE";
    }
    if (strstr(mimeType, "video/") == mimeType || isFilmFile(filePath)) {
        return "T_FILM";
    }
    if (strstr(mimeType, "audio/") == mimeType || isAudioFile(filePath)) {
        return "T_AUDIO";
    }
    if (strstr(mimeType, "application/zip") == mimeType ||
        strstr(mimeType, "application/x-tar") == mimeType ||
        strstr(mimeType, "application/x-bzip2") == mimeType ||
        strstr(mimeType, "application/gzip") == mimeType ||
        strstr(mimeType, "application/x-7z-compressed") == mimeType ||
        strstr(mimeType, "application/x-rar-compressed") == mimeType) {
        if (isJarFile(filePath)) {
            return "T_JAR";
        }
        return "T_COMPRESSED";
    }
    if (isExeFile(filePath)) {
        return "T_EXE";
    }
    if (isPackageFile(filePath)) {
        return "T_PACKAGE";
    }
    if (isClassFile(filePath)) {
        return "T_CLASS";
    }
    if (isPdfFile(filePath)) {
        return "T_PDF";
    }
    if (isJarFile(filePath)) {
        return "T_JAR";
    }
    if (isCalcFile(filePath)) {
        return "T_CALC";
    }
    if (isJsonFile(filePath)) {
        return "T_JSON";
    }
    return "T_BINARY";
}

char *getFileName(const char *path) {
    char *pathCopy = strdup(path); // Strdup to avoid modifying the input path
    const char *baseName = basename(pathCopy);
    char *result = strdup(baseName);
    free(pathCopy);
    return result;
}

int compareFileEntries(const void *a, const void *b) {
    const FileEntry *entryA = (FileEntry *) a;
    const FileEntry *entryB = (FileEntry *) b;
    return strcmp(entryA->path, entryB->path); // Case-sensitive comparison
}

void initQueue(TaskQueue *queue, const int capacity) {
    queue->tasks = (char **) malloc(capacity * sizeof(char *));
    if (!queue->tasks) {
        fprintf(stderr, "Failed to allocate memory for task queue\n");
        exit(EXIT_FAILURE);
    }
    queue->head = 0;
    queue->tail = 0;
    queue->capacity = capacity;
    pthread_mutex_init(&queue->mutex, NULL);
}

void resizeQueue(TaskQueue *queue) {
    const int new_capacity = queue->capacity * RESIZE_FACTOR;
    char **new_tasks = realloc(queue->tasks, new_capacity * sizeof(char *));
    if (!new_tasks) {
        fprintf(stderr, "Queue resize failed\n");
        exit(EXIT_FAILURE);
    }

    if (queue->tail < queue->head) {
        memmove(&new_tasks[queue->capacity], &new_tasks[0], queue->tail * sizeof(char *));
        queue->tail += queue->capacity;
    }

    queue->tasks = new_tasks;
    queue->capacity = new_capacity;
}

void enqueue(TaskQueue *queue, const char *path) {
    pthread_mutex_lock(&queue->mutex);
    if ((queue->tail + 1) % queue->capacity == queue->head) {
        resizeQueue(queue);
    }
    queue->tasks[queue->tail] = strdup(path);
    queue->tail = (queue->tail + 1) % queue->capacity;
    pthread_mutex_unlock(&queue->mutex);
}

char *dequeue(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->head == queue->tail) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    char *task = queue->tasks[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

void freeQueue(TaskQueue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    for (int i = queue->head; i != queue->tail; i = (i + 1) % queue->capacity) {
        free(queue->tasks[i]);
    }
    free(queue->tasks);
}

int findEntryIndexAdded(const FileEntry *entries, const int count, const char *path) {
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].path, path) == 0) {
            return i; // Return true if the path already exists
        }
    }
    return 0; // Not added yet
}

void replaceChar(char *str, const char oldChar, const char newChar) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == oldChar) {
            str[i] = newChar; // Replace oldChar with newChar
        }
    }
}

int moveFile(const char *fullPath, const char *newFullPath) {
    if (rename(fullPath, newFullPath) == 0) {
        return 0; // Success
    }
    perror("Error moving file");
    return -1; // Failure
}

/**
 * Extracts unique root tokens from a delimiter-separated text file and stores them in a dynamically
 * allocated array while updating the root count.
 *
 * This function reads a text file line by line, extracts the first token from each line using the
 * '|' delimiter, and adds unique tokens to the provided root array. It skips consecutive lines
 * starting with the same token and resizes the root array as needed to accommodate more tokens.
 * Memory for added tokens is dynamically allocated, and the caller is responsible for freeing it.
 *
 * @param fileName  The name of the input file to process. Each line of the file should conform to
 *                  the expected format with tokens separated by '|'.
 * @param root      A double pointer to an array of strings where the unique root tokens will be
 *                  stored. This array will be dynamically resized during execution.
 * @param count     A pointer to an integer representing the number of elements currently in the
 *                  root array. It will be updated as new tokens are added.
 */
void get_dir_root(const char *fileName, char ***root, int *count) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE_LENGTH]; // Buffer to hold a line from the file
    int capacity = MAX_LINE_LENGTH; // Initial size for the root array
    char *previousToken = NULL; // To keep track of the previous token

    // Read the first line
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Extract the first token from the first line
        const char *firstToken = strtok(buffer, SEP);
        if (firstToken == NULL) {
            fprintf(stderr, "Invalid file format: no valid token in the first line.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Store the first token into the root array
        previousToken = strdup(firstToken);
        if (previousToken == NULL) {
            perror("Error duplicating token");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        (*root)[*count] = malloc(sizeof(char) * strlen(previousToken) + 1); // Example: allocate memory for a string
        snprintf((*root)[*count], strlen(previousToken) + 1, "%s", previousToken);

        // (*root)[*count] = previousToken;
        (*count)++;
    }

    // Process the rest of the lines
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Extract the first token from the current line
        const char *lineToken = strtok(buffer, SEP);
        if (lineToken == NULL) {
            continue; // Skip lines that do not contain a valid token
        }

        // If the current line starts with the previous token, skip it
        if (strstr(lineToken, previousToken) != NULL) {
            continue;
        }

        // Add the token to the root array
        char *newToken = strdup(lineToken);
        if (newToken == NULL) {
            perror("Error duplicating token");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Expand the root array if necessary
        if (*count >= capacity) {
            capacity *= 2;
            char **newRoot = realloc(*root, capacity * sizeof(char *));
            if (newRoot == NULL) {
                perror("Error reallocating memory for root array");
                fclose(file);
                exit(EXIT_FAILURE);
            }
            *root = newRoot;
        }

        (*root)[*count] = malloc(sizeof(char) * strlen(newToken) + 1); // Example: allocate memory for a string
        snprintf((*root)[*count], strlen(newToken) + 1, "%s", newToken);

        // (*root)[*count] = newToken;
        (*count)++;

        // Update the `previousToken`
        previousToken = newToken;
    }
    (*root)[*count] = NULL; // Mark the end of the array
    fclose(file);
}

/**
 * Helper function to check if a string is a valid size_t (non-negative integer).
 * @param str: A string to check.
 * @return 1 if valid, 0 otherwise.
 */
int is_size_t(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0; // Non-digit character found
        }
        str++;
    }
    return 1; // Entire string is numeric
}

// Function to check if a value belongs to FILE_TYPES
bool is_valid_file_type(const char *type) {
    for (int i = 0; i < FILE_TYPES_COUNT; i++) {
        if (strcmp(FILE_TYPES[i], type) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Process a file line by line, ensuring each row contains between 3 and 6 columns
 * and that the second column is of type size_t.
 * @param filename: The path to the input file.
 * @return 0 on success, non-zero on error.
 */
int process_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;

    // Process each line of the file
    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        // Remove trailing newline character, if present
        line[strcspn(line, "\n")] = '\0';

        // Make a copy of the line since `strtok` modifies the input
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, line, sizeof(lineCopy));
        lineCopy[sizeof(lineCopy) - 1] = '\0';

        // Split the line into columns using '|'
        int columnCount = 0;
        char *columns[7] = {NULL}; // To store up to 6 columns
        char *token = strtok(line, SEP);

        while (token != NULL && columnCount < 7) {
            columns[columnCount++] = token;
            token = strtok(NULL, SEP);
        }

        // Check the number of columns (between 3 and 6)
        if (columnCount < 3 || columnCount > 6) {
            fprintf(stderr, "Error: Invalid row at line %d. Expected 3-6 columns, got %d. Row content: \"%s\"\n",
                    lineNumber, columnCount, lineCopy);
            exit(EXIT_FAILURE);
        }

        // Validate the second column as size_t
        if (!is_size_t(columns[1])) {
            fprintf(stderr, "Error: Invalid size_t value in the second column at line %d. Row content: \"%s\"\n",
                    lineNumber, lineCopy);
            continue; // Go to the next line
        }

        // Validate the 3rd column against FILE_TYPES
        if (!is_valid_file_type(columns[2])) {
            fprintf(stderr, "Error: Invalid file type in the third column at line %d. Row content: \"%s\"\n",
                    lineNumber, lineCopy);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    return EXIT_SUCCESS;
}

/**
 * Verifies, processes, and validates a set of merge files while managing a temporary filename list and root count.
 *
 * This function iterates through an array of provided merge file names, validates their existence and format,
 * and processes their contents to extract directory roots. It ensures that `tmpFileNames` is dynamically allocated
 * or resized as necessary and tracks the number of root elements identified. Each merge file is checked to confirm
 * it is a regular file before processing. Errors during memory allocation, file access, or invalid merge file formats
 * result in error messages being displayed and appropriate action taken (e.g., program termination).
 *
 * - Dynamically manages memory for the `tmpFileNames` array to store extracted roots.
 * - For each file, calls `get_dir_root` to extract and update directory roots.
 * - Ensures each file in `mergeFileNames` exists and is a regular file.
 * - Uses `process_file` to handle file-specific processing for valid files.
 *
 * @param mergeFileNames An array of strings representing paths to merge files to be processed.
 *                       The array must be terminated with a NULL pointer.
 * @param tmpFileNames   A pointer to a dynamic array of strings that will store temporary filenames used during processing.
 *                       If `*tmpFileNames` is NULL, the function allocates memory for the array.
 * @param rootCount      A pointer to an integer used to count the number of root directory entries extracted
 *                       from the processed merge files.
 */
void check_input_files(char **mergeFileNames, char ***tmpFileNames, int *rootCount) {
    if (mergeFileNames == NULL) {
        fprintf(stderr, "Error: mergeFileNames is NULL.\n");
        exit(EXIT_FAILURE);
    }
    if (*tmpFileNames == NULL) {
        *tmpFileNames = realloc(*tmpFileNames, sizeof(char *) * (MAX_LINE_LENGTH + 2));
        if (*tmpFileNames == NULL) {
            perror("Error allocating memory for tmpFileNames");
            exit(EXIT_FAILURE);
        }
    } else {
        *tmpFileNames = realloc(*tmpFileNames, sizeof(char *) * (MAX_LINE_LENGTH + 2)); // Reallocate memory
        if (*tmpFileNames == NULL) {
            perror("Error reallocating memory for tmpFileNames");
            exit(EXIT_FAILURE);
        }
    }
    int tmp = 0;

    // *tmpFileNames = malloc(MAX_LINE_LENGTH * sizeof(char *));
    if (*tmpFileNames == NULL) {
        perror("Error allocating memory for tmpFileNames in check_merge_files");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; mergeFileNames[i] != NULL; i++) {
        get_dir_root(mergeFileNames[i], tmpFileNames, &tmp);
        const char *fileName = mergeFileNames[i];
        fprintf(stdout, "Checking input file format: %s\n", fileName);
        // Check if the file is a regular file
        struct stat fileStat;
        if (stat(fileName, &fileStat) != 0) {
            perror("Error accessing file");
            fprintf(stderr, "Error: Cannot access file '%s'.\n", fileName);
            continue;
        }
        if (!S_ISREG(fileStat.st_mode)) {
            fprintf(stderr, "Error: Merge file '%s' is not a regular file.\n", fileName);
            exit(EXIT_FAILURE);
        }
        // Check and process the file using process_file
        if (process_file(fileName) != EXIT_SUCCESS) {
            fprintf(stderr, "Error: Failed to process file '%s'.\n", fileName);
        }
        if (*rootCount == -1) {
            break;
        }
    }
    *rootCount = tmp;
}

/**
 * Processes directories and their contents, updating file statistics and managing a dynamic,
 * thread-safe queue of tasks.
 *
 * This function dequeues directory paths from a task queue, scans their contents, and processes
 * files and directories to update an array of FileEntry structures and file statistics. It uses
 * OpenMP to parallelize file processing, and handles dynamically allocated memory for directory
 * entries and FileEntry structures. Directories are added to the FileEntry array with their
 * children count, while various file types are categorized and their statistics are updated.
 *
 * The function stops processing if it encounters memory allocation errors or a critical issue
 * during directory or file handling.
 *
 * @param taskQueue      A pointer to the TaskQueue containing directories to be processed.
 * @param entries        A pointer to an array of FileEntry structures representing files and
 *                       directories that have been processed. The array is dynamically resized
 *                       as needed.
 * @param count          A pointer to an integer tracking the number of FileEntry structures
 *                       added to the `entries` array.
 * @param capacity       A pointer to an integer representing the current capacity of the
 *                       `entries` array. The array is resized by multiplying its size by
 *                       RESIZE_FACTOR whenever more capacity is needed.
 * @param sizeThreshold  The minimum file size (in bytes) for files to be included in the
 *                       `entries` array and statistics.
 * @param skipDirs       An integer flag (0 or 1) to determine whether directories should be
 *                       included in the `entries` array and counted in statistics.
 */
void processDirectory(TaskQueue *taskQueue, FileEntry **entries, int *count, int *capacity,
                      const long long sizeThreshold, const int skipDirs) {
    char **dirEntries = malloc(INITIAL_CAPACITY * sizeof(char *));
    if (!dirEntries) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int numEntries = 0;
    int entryCapacity = INITIAL_CAPACITY;

    while (1) {
        char *currentPath;
#pragma omp critical
        {
            currentPath = dequeue(taskQueue);
        }
        if (!currentPath) break;

        struct dirent *entry;
        DIR *dp = opendir(currentPath);
        if (dp == NULL) {
            if (errno == EACCES) {
                fprintf(stderr, "Permission denied: %s\n", currentPath);
            } else {
                fprintf(stderr, "Error opening directory %s: %s\n", currentPath, strerror(errno));
            }
            free(currentPath);
            continue;
        }

        int childrenCount = 0; // Count files and directories for the current directory
        while ((entry = readdir(dp)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                continue;
            }

            if (numEntries >= entryCapacity) {
                entryCapacity *= RESIZE_FACTOR;
                char **newDirEntries = realloc(dirEntries, entryCapacity * sizeof(char *));
                if (!newDirEntries) {
                    perror("realloc");
                    closedir(dp);
                    free(currentPath);
                    exit(EXIT_FAILURE);
                }
                dirEntries = newDirEntries;
            }
            dirEntries[numEntries] = strdup(entry->d_name);
            if (!dirEntries[numEntries]) {
                perror("strdup");
                closedir(dp);
                free(currentPath);
                exit(EXIT_FAILURE);
            }
            numEntries++;
            childrenCount++; // Increment children count
        }
        closedir(dp);

        // Update the current directory entry with its children count
#pragma omp critical
        {
            if (*count >= *capacity) {
                *capacity *= RESIZE_FACTOR;
                *entries = (FileEntry *) realloc(*entries, (*capacity) * sizeof(FileEntry));
                if (!*entries) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }
            const int current = *count;
            (*count)++;
            snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s", currentPath);
            (*entries)[current].size = 4096; // Size is 0 for directories
            (*entries)[current].isDir = 1; // Mark as a directory
            (*entries)[current].childrenCount = childrenCount; // Store the children count
            snprintf((*entries)[current].type, sizeof((*entries)[current].type), "T_DIR");
        }

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < numEntries; ++i) {
            char fullPath[MAX_LINE_LENGTH];
            struct stat fileStat;
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, dirEntries[i]);

            if (stat(fullPath, &fileStat) == 0) {
                if (S_ISREG(fileStat.st_mode)) {
                    if (strstr(fullPath, "|") != NULL) {
                        char fullPathCopy[sizeof(fullPath)];
                        strcpy(fullPathCopy, fullPath);
                        replaceChar(fullPath, '|', '-');
                        moveFile(fullPathCopy, fullPath);
                    }

                    magic_t magic = magic_open(MAGIC_MIME_TYPE);
                    if (magic != NULL && magic_load(magic, NULL) == 0) {
                        const char *mimeType = magic_file(magic, fullPath);
                        if (mimeType) {
                            if (fileStat.st_size >= sizeThreshold) {
#pragma omp critical
                                {
                                    if (*count >= *capacity) {
                                        *capacity *= RESIZE_FACTOR;
                                        *entries = (FileEntry *) realloc(*entries, (*capacity) * sizeof(FileEntry));
                                        if (!*entries) {
                                            perror("realloc");
                                            exit(EXIT_FAILURE);
                                        }
                                    }
                                    const int current = *count;
                                    (*count)++;
                                    snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s",
                                             fullPath);
                                    (*entries)[current].size = fileStat.st_size;
                                    (*entries)[current].isDir = 0; // Mark as a file
                                    (*entries)[current].childrenCount = 0; // Files don't have children
                                    snprintf((*entries)[current].type, sizeof((*entries)[current].type), "%s",
                                             getFileTypeCategory(mimeType, fullPath));
                                }
                            }
                        }
                        magic_close(magic);
                    } else {
                        fprintf(stderr, "Failed to initialize magic: %s\n", magic_error(magic));
                        if (magic) {
                            magic_close(magic);
                        }
                    }
                } else if (S_ISDIR(fileStat.st_mode)) {
#pragma omp critical
                    {
                        if (*count >= *capacity) {
                            *capacity *= RESIZE_FACTOR;
                            *entries = (FileEntry *) realloc(*entries, (*capacity) * sizeof(FileEntry));
                            if (!*entries) {
                                perror("realloc");
                                exit(EXIT_FAILURE);
                            }
                        }
                        if (!findEntryIndexAdded(*entries, *count, fullPath)) {
                            const int current = *count;
                            (*count)++;
                            snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s", fullPath);
                            (*entries)[current].size = 4096; // Size is 0 for directories
                            (*entries)[current].isDir = 1; // Mark as a directory
                            (*entries)[current].childrenCount = 0; // Initialize children count (updated when processed)
                            snprintf((*entries)[current].type, sizeof((*entries)[current].type), "T_DIR");
                        }
                        if (!skipDirs) {
                            // If directories need to be enqueued for further exploration
                            enqueue(taskQueue, fullPath);
                        }
                    }
                }
            } else {
                fprintf(stderr, "Error stating %s: %s\n", fullPath, strerror(errno)); // Only log errors
            }
            free(dirEntries[i]);
        }
        free(currentPath);
        numEntries = 0;
    }
    free(dirEntries);
}

void initializeFileEntries(FileEntry *entries, const size_t count) {
    if (!entries) {
        return; // Avoid null pointer dereference
    }

    for (size_t i = 0; i < count; i++) {
        entries[i].path[0] = '\0'; // Initialize path to empty
        entries[i].size = 0; // Initialize size to 0
        entries[i].isDir = 0; // Initialize isDir to false (0)
        entries[i].isHidden = 0; // Initialize isHidden to false (0)
        entries[i].childrenCount = 0; // Initialize childrenCount to 0
        entries[i].type[0] = '\0'; // Initialize type to empty
    }
}

char *getFileSizeAsString(const long long fileSizeBytesIn) {
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

void printSizeDetails(const char *type, const int count, const long long size) {
    if (count > 0) {
        printf("Total Number of %s Files: %d\n", type, count);
        char *sizeStr = getFileSizeAsString(size);
        printf("Total Size of %s Files: %lld bytes (%s)\n", type, size, sizeStr);
        free(sizeStr);
    }
}

/**
 * Releases dynamically allocated resources for a variable number of character pointers.
 *
 * This function frees the memory allocated for each dynamically-allocated string passed to
 * it as arguments. The first parameter is a pointer to a character pointer, and additional
 * character pointers can be specified through variadic arguments. After freeing a resource,
 * the corresponding pointer is set to NULL to prevent accidental reuse or double freeing.
 *
 * The function uses variable argument handling (variadic arguments) to process an arbitrary
 * number of resources. The list of arguments must be terminated with a NULL pointer to
 * indicate the end of the resources.
 *
 * @param first   A pointer to the first dynamically-allocated resource. It is freed and
 *                its pointer is set to NULL after the operation.
 * @param ...     A variadic list of additional dynamically-allocated resources. Each resource
 *                is processed similarly to the first. The list must end with a NULL pointer.
 */
void release_temporary_resources(char **first, ...) {
    if (first && *first) {
        free(*first);
        *first = NULL; // Avoid double freeing
    }

    va_list args;
    char **resource;

    va_start(args, first);
    while ((resource = va_arg(args, char **)) != NULL) {
        if (resource && *resource) {
            free(*resource);
            *resource = NULL; // Clear pointer after freeing
        }
    }
    va_end(args);
}

/**
 * Resizes a dynamically allocated array of `FileEntry` structures by removing empty entries.
 *
 * This function iterates through an array of `FileEntry` structures and counts the number of
 * non-empty entries, identified by a non-empty `path` field. A new array of appropriate size
 * is then allocated, and all non-empty entries from the original array are copied into the
 * new array. The old array's memory is freed, and the pointer to the array is updated to point
 * to the newly resized array. The entry count is also updated to reflect the new size.
 *
 * This function ensures efficient memory management by freeing unused entries and reallocating
 * memory only where necessary. It performs error checking for memory allocation failures.
 *
 * @param entries  A double pointer to the array of `FileEntry` structures. The function resizes
 *                 the array, replacing the old array pointer with the new one. The caller is
 *                 responsible for ensuring the input array is properly initialized.
 * @param count    A pointer to an integer containing the count of entries in the original array.
 *                 The function updates this value to the new count after resizing.
 */
void resizeEntries(FileEntry **entries, int *count) {
    // Step 1: Count non-empty elements
    int nonEmptyCount = 0;
    for (int i = 0; i < *count; i++) {
        if ((*entries)[i].path[0] != '\0') {
            // Check if "empty" — path[0] not null
            nonEmptyCount++;
        }
    }

    // Step 2: Create a new resized array
    FileEntry *newEntries = malloc(nonEmptyCount * sizeof(FileEntry));
    if (!newEntries) {
        perror("Error resizing entries");
        return;
    }
    initializeFileEntries(newEntries, nonEmptyCount);

    // Step 3: Copy non-empty elements
    size_t index = 0;
    for (int i = 0; i < *count; i++) {
        if ((*entries)[i].path[0] != '\0') {
            newEntries[index++] = (*entries)[i];
        }
    }

    // Step 4: Free old array and update the pointer
    free(*entries);
    *entries = newEntries;
    *count = nonEmptyCount; // Update count to reflect new size
}

/**
 * Reads and parses file entries from a specified file into a dynamically allocated array.
 *
 * This function opens a file containing information about file entries, reads each line,
 * and parses the data into an array of `FileEntry` structures. Each line in the file
 * should follow the format: `path|size|type[|C_COUNT|childrenCount][|F_HIDDEN]`.
 * The function processes up to a fixed number of entries and dynamically allocates memory
 * for the array based on `fixed_count`.
 *
 * The parsed information includes the file's path, size, type, and additional metadata such
 * as whether it is a directory, its children count (if applicable), and whether it is hidden.
 * Duplicate directories are handled by replacing earlier entries with the new one.
 *
 * The function performs robust error checking for parsing issues and ensures proper memory
 * management. If any error occurs (e.g., file access, memory allocation failure), the program
 * terminates with an error message. Malformed lines in the file are skipped with a warning.
 *
 * @param filename     The name of the file to read from. The file must be accessible and
 *                     formatted correctly.
 * @param entries      A double pointer to a `FileEntry` array. The function allocates memory
 *                     to this pointer, and the caller is responsible for freeing it afterward.
 *                     If this pointer already contains allocated memory, it will be freed
 *                     before reallocating.
 * @param fixed_count  The maximum number of entries to read. This defines the size of the
 *                     allocated array.
 * @param count        Pointer to an integer where the number of successfully read entries will
 *                     be stored.
 */
void read_entries(const char *filename, FileEntry **entries, const size_t fixed_count, int *count) {
    *count = 0;
    // Open the file for reading
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Reset the current entries
    size_t i = 0;
    if (*entries) {
        // free(*entries);
    }

    // Allocate memory for the fixed amount of entries
    *entries = malloc(fixed_count * sizeof(FileEntry));
    initializeFileEntries(*entries, fixed_count);
    if (!*entries) {
        perror("Memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        // Remove the newline character, if present
        int isAdded = false;
        const size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Stop reading if we've reached the fixed count
        if (i >= fixed_count) {
            break;
        }

        // Parse the line using `strtok` for the `|` delimiter
        FileEntry entry = {0}; // Ensure to initialize all fields
        // Look for hidden flag f
        if (strstr(buffer, "F_HIDDEN") != NULL) {
            entry.isHidden = true;
        }
        char *token = strtok(buffer, SEP);
        if (!token) {
            fprintf(stderr, "Error parsing path in line: %s\n", buffer);
            continue;
        }
        strncpy(entry.path, token, sizeof(entry.path) - 1);
        entry.path[sizeof(entry.path) - 1] = '\0';

        token = strtok(NULL, SEP);
        if (!token) {
            fprintf(stderr, "Error parsing size in line: %s\n", buffer);
            continue;
        }
        char *endptr;
        entry.size = strtol(token, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid numeric format for size in line: %s\n", buffer);
            continue;
        }

        token = strtok(NULL, SEP);
        if (!token) {
            fprintf(stderr, "Error parsing type in line: %s\n", buffer);
            continue;
        }
        strncpy(entry.type, token, sizeof(entry.type) - 1);
        entry.type[sizeof(entry.type) - 1] = '\0';
        // Check if the entry is a directory and process extra flags
        if (strcmp(entry.type, "T_DIR") == 0) {
            entry.isDir = true;

            // Look for additional flags (e.g., C_COUNT and F_HIDDEN)
            token = strtok(NULL, SEP);
            while (token) {
                if (strncmp(token, "C_COUNT", 8) == 0) {
                    token = strtok(NULL, SEP);
                    entry.childrenCount = strtol(token, &endptr, 10);
                    if (*endptr != '\0') {
                        fprintf(stderr, "Invalid numeric format in C_COUNT: %s\n", token);
                    }
                } else if (strstr(token, "F_HIDDEN") != NULL) {
                    // const char *hiddenFlag = ", F_HIDDEN";
                    entry.isHidden = true;
                    // if (strlen(entry.type) + strlen(hiddenFlag) + 1 < MAX_TYPE_LENGTH) {
                    //     strncat(entry.type, hiddenFlag, MAX_TYPE_LENGTH - strlen(entry.type) - 1);
                    // } else {
                    //     fprintf(stderr, "Error: Not enough space to append to entry.type\n");
                    // }
                }
                token = strtok(NULL, SEP);
            }
            if (i >= 1 && strcmp((*entries)[i - 1].path, entry.path) == 0 && (*entries)[i - 1].isDir == 1) {
                (*entries)[i - 1] = entry;
                isAdded = true;
            } else if (i >= 2 && strcmp((*entries)[i - 2].path, entry.path) == 0 && (*entries)[i - 2].isDir == 1) {
                (*entries)[i - 2] = entry;
                isAdded = true;
            }
        }
        // Store the entry in the array
        if (!isAdded && *entry.path != '\0') {
            (*entries)[i++] = entry;
            (*count)++;
        }
    }

    fclose(file);
}

/**
 * Writes detailed information about a collection of file entries to a specified file.
 *
 * This function iterates over an array of `FileEntry` structures and writes information
 * for each entry to the provided file. The output includes the file's path, size, type,
 * and additional metadata.
 *
 * If the entry represents a directory, the number of its children is appended to the output.
 * Entries that are hidden (i.e., filenames starting with a '.') are explicitly flagged in the output.
 *
 * The output format follows this structure: `path|size|type[|C_COUNT|childrenCount][|F_HIDDEN]\n`.
 * Each entry is written as a single line in the output file. If the file cannot be opened,
 * an error message is printed, and the function terminates early.
 *
 * The user must ensure that the provided filename and file mode are valid for the intended operation.
 *
 * @param entries  Pointer to an array of `FileEntry` structures containing details of each file.
 *                 Each entry must include valid data for `path`, `size`, `type`, and directory status.
 * @param count    The number of file entries in the array. This specifies how many entries to process.
 * @param filename The name of the file to which the information will be written. This file will
 *                 be created or modified according to the specified mode.
 * @param mode     The file access mode (e.g., "w" for write, "a" for append). The mode must be a
 *                 valid format recognized by `fopen`.
 */
void printToFile(FileEntry *entries, const int count, const char *filename, const char *mode) {
    FILE *outputFile = fopen(filename, mode);
    if (!outputFile) {
        perror("Failed to open file");
        return;
    }
    for (int i = 0; i < count; i++) {
        char *fileName = getFileName(entries[i].path);
        const int isHidden = (fileName[0] == '.'); // Check if the file is hidden
        free(fileName);
        fprintf(outputFile, "%s%s%ld%s%s", entries[i].path, SEP, entries[i].size, SEP, entries[i].type);
        // If the entry is a directory, add the count of children
        if (entries[i].isDir) {
            fprintf(outputFile, "%sC_COUNT%s%zu", SEP, SEP, entries[i].childrenCount);
        }
        if (isHidden) {
            fprintf(outputFile, "%sF_HIDDEN", SEP);
        }
        fprintf(outputFile, "\n"); // End the line
    }
    fclose(outputFile);
}

/**
 * Check if a directory path is already present in rootDirectories.
 * @param rootDirectories Array of root directories.
 * @param dirPath Directory path to check.
 * @return 1 if the directory is present, 0 otherwise.
 */
static int is_in_root_directories(char **rootDirectories, const char *dirPath) {
    for (int i = 0; rootDirectories[i] != NULL; i++) {
        if (strcmp(rootDirectories[i], dirPath) == 0) {
            return 1; // Found in rootDirectories
        }
    }
    return 0; // Not found
}

/**
 * Reads directories in a parentDir, adds them to directories array if not present in rootDirectories,
 * and limits them to the step count.
 * @param parentDir The parent directory to scan.
 * @param directories Pointer to dynamic array of directories.
 * @param rootDirectories Array of already included directories to skip duplicates.
 * @param step Maximum number of directories to retrieve.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int read_directories(const char *parentDir, char ***directories, char **rootDirectories, const int step) {
    DIR *dir;
    struct dirent *entry;
    struct stat entryStat;
    int count = 0; // Number of directories stored

    // Open the parent directory
    dir = opendir(parentDir);
    if (dir == NULL) {
        perror("Failed to open directory");
        return EXIT_FAILURE;
    }

    // Initialize directories array
    *directories = malloc(sizeof(char *) * (PATH_MAX + 1)); // Allocate memory for directories
    if (*directories == NULL) {
        perror("Memory allocation failed for directories");
        closedir(dir);
        return EXIT_FAILURE;
    }

    // Read entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        char fullPath[PATH_MAX]; // Combine parentDir and entry name
        if (snprintf(fullPath, PATH_MAX, "%s/%s", parentDir, entry->d_name) >= PATH_MAX) {
            fprintf(stderr, "Full path exceeds the maximum allowable length\n");
            continue; // Skip this entry
        }

        // Get statistics for the entry
        if (stat(fullPath, &entryStat) == -1) {
            perror("Failed to stat directory entry");
            continue; // Skip this entry
        }

        // Check if the entry is a directory (and not "." or "..")
        if (S_ISDIR(entryStat.st_mode) &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            // Check if the directory is not already in rootDirectories
            if (!is_in_root_directories(rootDirectories, fullPath)) {
                // Allocate memory and store the full path
                (*directories)[count] = strdup(fullPath);
                if ((*directories)[count] == NULL) {
                    perror("Memory allocation failed for directory path");
                    // Free allocated memory and close directory
                    for (int j = 0; j < count; j++) free((*directories)[j]);
                    free(*directories);
                    closedir(dir);
                    return EXIT_FAILURE;
                }

                count++;
                // Stop if we've stored `step` directories
                if (count == step) {
                    break;
                }
            }
        }
    }

    closedir(dir);

    // NULL-terminate the list of directory paths
    (*directories)[count] = NULL;

    // Check if we found fewer directories than expected
    if (count < step) {
        fprintf(stderr, "Warning: Found only %d directories, expected %d.\n", count, step);
    }

    return EXIT_SUCCESS;
}


/**
 * Parses command-line arguments to process the "--step" option, ensuring valid step values,
 * a corresponding directory, and an optional output file specification.
 *
 * This function parses command-line arguments to extract the value for the "--step" option,
 * validates its range (2 to 10000), verifies the existence of a directory provided afterward,
 * and retrieves the name of an output file if the "-o" option is specified.
 * It performs error checking for argument validity, memory allocation, and input format,
 * exiting with an error message in case of invalid input.
 *
 * @param argc            The number of command-line arguments.
 * @param argv            An array of strings representing the command-line arguments.
 * @param stepValue       A pointer to an integer where the parsed step value will be stored.
 * @param parentDirectory A pointer to a string where the directory path will be stored
 *                        if valid. Memory for the string is allocated dynamically.
 * @param outputFileName  A pointer to a string where the output file name will be stored
 *                        if the "-o" option is provided. Memory for the string is dynamically allocated.
 * @return                Returns 0 on successful parsing and validation of the arguments.
 *                        Exits the program with an error message if any issue is encountered.
 */
int handle_step_option(const int argc, char *argv[], int *stepValue, char **parentDirectory, char **outputFileName) {
    char *endPtr; // Used to check for strtol errors
    for (int i = 1; i < argc; i++) {
        // Check for the "--step" option
        if (strcmp(argv[i], "--step") == 0) {
            // Retrieve the number after "--step"
            if (i + 1 < argc) {
                errno = 0; // Reset errno before strtol
                const long value = strtol(argv[i + 1], &endPtr, 10);
                // Validate that the value is numeric and within 2-10000
                if (*endPtr != '\0' || errno != 0 || value < 2 || value > 10000) {
                    fprintf(stderr, "Error: '%s' is not a valid step value (must be an integer between 2 and 10000)\n", argv[i + 1]);
                    exit(EXIT_FAILURE);
                }
                *stepValue = (int)value; // Store the step value
                // Retrieve the directory argument
                if (i + 2 < argc) {
                    struct stat statBuffer;
                    if (stat(argv[i + 2], &statBuffer) == 0 && S_ISDIR(statBuffer.st_mode)) {
                        *parentDirectory = removeTrailingSlash(argv[i + 2]); // Store the directory path
                        if (*parentDirectory == NULL) {
                            perror("Failed to allocate memory for directory path");
                            exit(EXIT_FAILURE);
                        }
                        // Retrieve the output file name after "-o"
                        if (i + 3 < argc && strcmp(argv[i + 3], "-o") == 0) {
                            if (i + 4 < argc) {
                                *outputFileName = strdup(argv[i + 4]); // Store the file name
                                if (*outputFileName == NULL) {
                                    perror("Failed to allocate memory for output file name");
                                    exit(EXIT_FAILURE);
                                }
                                return 0; // Success
                            }
                            fprintf(stderr, "Error: Missing output file name after '-o'\n");
                            exit(EXIT_FAILURE);
                        }
                        fprintf(stderr, "Error: '-o' option not found\n");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr, "Error: '%s' is not a valid directory\n", argv[i + 2]);
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "Error: Missing directory argument after '--step <number>'\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "Error: Missing number after '--step'\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stderr, "Error: '--step' option not provided\n");
    exit(EXIT_FAILURE);
}

/**
 * Prints detailed information about a collection of file entries to the standard output.
 *
 * This function iterates over a list of file entries and prints details for each file,
 * including its path, size (in both human-readable and raw byte formats), type, and any
 * other applicable metadata (e.g., whether it is a directory or a hidden file).
 *
 * If the entry is a directory, the number of children it contains is also displayed.
 * Hidden files (those starting with a '.') are explicitly marked in the output.
 *
 * The size of each file is converted to a human-readable format using `getFileSizeAsString`.
 * The string returned by this function is dynamically allocated and must be freed to
 * prevent memory leaks. This function ensures that `free` is called for this data
 * after it is printed.
 *
 * @param entries A pointer to an array of `FileEntry` structures containing information
 *                about the files to be printed. Each entry must contain valid data,
 *                including its path, size, type, and whether it is a directory.
 * @param count   The number of file entries in the array. This value specifies how many
 *                entries to iterate over.
 */
void printToStdOut(FileEntry *entries, const int count) {
    for (int i = 0; i < count; i++) {
        const char *fileName = getFileName(entries[i].path);
        const int isHidden = (fileName[0] == '.'); // Check if the file is hidden
        char *sizeStr = getFileSizeAsString(entries[i].size);
        printf("%s %s, Size: %s (%ld bytes), Type: %s",
               strcmp(entries[i].type, "T_DIR") == 0 ? "Dir: " : "File:",
               entries[i].path, sizeStr, entries[i].size, entries[i].type);
        // If the entry is a directory, add the count of children
        if (entries[i].isDir) {
            printf(", C_COUNT: %zu", entries[i].childrenCount);
        }
        if (isHidden) {
            printf(", F_HIDDEN");
        }
        printf("\n"); // End the line
        free(sizeStr); // Free dynamically allocated string from getFileSizeAsString
    }
}

/**
 * Deletes a specified file from the file system.
 *
 * This function deletes a file with the provided filename if it exists. It first checks for the
 * existence of the file using the `access` function with the `F_OK` flag, which tests for the
 * file's presence. If the file exists, the `remove` function is called to delete it. Any
 * failure during the deletion process results in an error message being displayed using `perror`.
 *
 * @param filename  A pointer to a null-terminated string specifying the path to the file
 *                  that is to be deleted.
 */
void deleteFile(char *filename) {
    // Check if the file exists
    if (access(filename, F_OK) == 0) {
        // F_OK tests for existence
        const int result = remove(filename);
        if (result != 0) {
            perror("Failed to delete the file");
        }
    }
}

void deleteFiles(char **tmpFileNames) {
    if (tmpFileNames == NULL) {
        return; // Handle empty array case
    }

    for (int i = 0; tmpFileNames[i] != NULL; i++) {
        deleteFile(tmpFileNames[i]); // Call deleteFile for each filename
    }
}

/**
 * Removes the trailing slash from a given string, if present, and returns a new string.
 *
 * This function processes an input string to check for a trailing slash ('/').
 * If the trailing slash exists, it allocates memory for a new string without
 * the trailing slash, copies the content excluding the slash, and null-terminates
 * the result. If there is no trailing slash, a duplicate of the original string is returned.
 *
 * Memory for the returned string is dynamically allocated and should be freed
 * by the caller to avoid memory leaks.
 *
 * In case of memory allocation errors, it logs an error message and returns NULL.
 *
 * @param token The input string to process. It represents the original string that might
 *              have a trailing slash. If NULL is passed, the function returns NULL.
 *
 * @return A new dynamically allocated string without a trailing slash, or NULL in case of
 *         errors. If the input string does not contain a trailing slash, a duplicate of
 *         the original string is returned.
 */
char *removeTrailingSlash(const char *token) {
    if (token == NULL) {
        return NULL;
    }
    const size_t len = strlen(token);
    if (len > 0 && token[len - 1] == '/') {
        // Allocate memory for the new string (excluding the trailing '/')
        char *newFileName = (char *) malloc(len * sizeof(char));
        if (newFileName == NULL) {
            perror("malloc failed");
            return NULL;
        }
        strncpy(newFileName, token, len - 1);
        newFileName[len - 1] = '\0'; // Null-terminate the string
        return newFileName;
    }
    // If no trailing slash, return a duplicate of the original string
    char *newFileName = strdup(token);
    if (newFileName == NULL) {
        perror("strdup failed");
    }
    return newFileName;
}

/**
 * Accumulates the size and number of children for directory entries within a file entry array.
 *
 * This function processes an array of file entries, identifying directory entries
 * and calculating their total size and number of children by analyzing subsequent
 * entries in the array that reside within the directory's path hierarchy.
 *
 * For each directory entry, this function resets the `childrenCount` and `size`
 * fields before accumulating the relevant data. Only entries whose paths match
 * the directory's path as a prefix are considered as children. The process stops
 * early if entries no longer belong to the current directory hierarchy to optimize performance.
 *
 * @param entries A pointer to an array of `FileEntry` structures representing files
 *                and directories. Each entry holds information about size, path, and
 *                whether it is a directory.
 * @param count   The total number of entries in the `entries` array. This is used to
 *                determine the bounds for iteration to avoid accessing invalid memory.
 */
void accumulateChildrenAndSize(FileEntry *entries, const size_t count) {
    if (entries == NULL || count == 0) {
        return;
    }
    // Iterate through entries
    for (size_t i = 0; i < count; i++) {
        // Only accumulate for directories
        if (entries[i].isDir) {
            entries[i].childrenCount = 0; // Reset the children count
            entries[i].size = 4096; // Reset the size (to accumulate later)
            // Check subsequent entries to see if they belong to this directory
            if (strcmp(entries[i].path, "/home/simon/playground/trading/share/terminfo/X") == 0) {
                int a = 1;
            }
            if (strcmp(entries[i].path, "/home/simon/playground/trading/share/terminfo/e") == 0) {
                int a = 1;
            }
            if (strcmp(entries[i].path, "/home/simon/playground/trading/share/terminfo/n") == 0) {
                int a = 1;
            }
            if (strcmp(entries[i].path, "/home/simon/playground/trading/share/terminfo/m") == 0) {
                int a = 1;
            }
            if (strcmp(entries[i].path, "/home/simon/playground/trading/share/terminfo/x") == 0) {
                int a = 1;
            }
            for (size_t j = i + 1; j < count; j++) {
                if (strstr(entries[j].path, entries[i].path) == entries[j].path &&
                    entries[j].path[strlen(entries[i].path)] == '/') {
                    // Current directory is a parent of entries[j]
                    entries[i].childrenCount++;
                    entries[i].size += entries[j].size;
                } else if (j + 1 < count) {
                    bool stop = true;
                    for (int k = 0; k < 3 && j + k < count; k++) {
                        const char *res = strstr(entries[j + k].path, entries[i].path);
                        if (res && strcmp(res, entries[j + k].path) == 0) {
                            // At least one path matches within the next 10 entries
                            stop = false;
                            break;
                        }
                    }
                    if (stop) {
                        // Break out of the outer loop
                        break;
                    }
                }
            }
        }
    }
}

/**
 * Frees memory allocated for an array of directory paths.
 *
 * Iterates through the array of directory paths, deallocating each string and
 * then the array itself. All pointers are safely reset to NULL to avoid
 * dangling references.
 *
 * @param directories A pointer to a dynamically allocated array of strings
 *                    representing directory paths. Each string in the array,
 *                    as well as the array itself, will be deallocated.
 *                    The provided pointer and all its elements will be set to NULL
 *                    after memory is freed.
 */
void free_array(char ***directories) {
    if (directories && *directories) {
        // Free each string in the array
        for (int i = 0; (*directories)[i] != NULL; i++) {
            free((*directories)[i]); // Free each string
            (*directories)[i] = NULL; // Avoid dangling pointer
        }
        // Free the array itself
        free(*directories);
        *directories = NULL; // Avoid dangling pointer
    }
}

void free_multiple_arrays(char ***first_directory, ...) {
    va_list args;
    va_start(args, first_directory); // Initialize the argument list

    char ***current_directory = first_directory;

    while (current_directory != NULL) {
        if (*current_directory && *current_directory != NULL) {
            // Free each string in the array
            for (int i = 0; (*current_directory)[i] != NULL; i++) {
                free((*current_directory)[i]); // Free each string
                (*current_directory)[i] = NULL; // Avoid dangling pointer
            }
            // Free the array itself
            free(*current_directory);
            *current_directory = NULL; // Avoid dangling pointer
        }
        // Get the next argument (char ***)
        current_directory = va_arg(args, char ***);
    }

    va_end(args); // Clean up the argument list
}

int belongs_to_array(const char *arg, char **mergeFileNames, const int size) {
    for (int i = 0; i < size; ++i) {
        if (strcmp(arg, mergeFileNames[i]) == 0) {
            return 1; // Found
        }
    }
    return 0; // Not found
}

char **remove_duplicate_directories(char **directories, const int count, int *new_count) {
    char **unique_directories = malloc(count * sizeof(char *)); // Allocate memory for new array
    if (!unique_directories) {
        perror("Failed to allocate memory");
        free_array(&unique_directories);
        exit(EXIT_FAILURE);
    }

    int unique_index = 0;

    for (int i = 0; i < count; i++) {
        int is_duplicate = 0;

        // Check if the current directory is already in the unique array
        for (int j = 0; j < unique_index; j++) {
            if (strcmp(removeTrailingSlash(directories[i]), removeTrailingSlash(unique_directories[j])) == 0) {
                is_duplicate = 1;
                break;
            }
        }

        // If it's not a duplicate, add it to the unique array
        if (!is_duplicate) {
            unique_directories[unique_index] = malloc(strlen(directories[i]) + 1); // Allocate memory for the string
            if (!unique_directories[unique_index]) {
                perror("Failed to allocate memory for unique directory");
                free_array(&unique_directories);
                exit(EXIT_FAILURE);
            }
            strcpy(unique_directories[unique_index], removeTrailingSlash(directories[i]));
            unique_index++;
        }
    }

    *new_count = unique_index; // Set the new count of unique directories

    // Reallocate memory to adjust to the actual size
    unique_directories = realloc(unique_directories, unique_index * sizeof(char *));
    if (!unique_directories) {
        perror("Failed to reallocate memory");
        free_array(&unique_directories);
        exit(EXIT_FAILURE);
    }
    unique_directories[unique_index] = NULL; // Ensure NULL-termination
    return unique_directories;
}

int is_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == 0) {
        return S_ISDIR(path_stat.st_mode);
    }
    return 0;
}

int is_file(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == 0) {
        return S_ISREG(path_stat.st_mode);
    }
    return 0;
}


/**
 * Processes command-line arguments for various output options, input directories, file thresholds,
 * and flags, updating corresponding pointers and structures.
 *
 * This function parses a variety of input options, including output file locations, merge files,
 * statistics file inputs, and other command-line arguments. It validates the usage of exclusive
 * flags, allocates memory for dynamically stored file names, and ensures proper argument formats
 * are respected. If invalid or conflicting arguments are provided, the function cleans up allocated
 * resources and returns an error.
 *
 * The key functionalities include:
 * - Handling `-o` for specifying output file name.
 * - Processing `--merge` and `-m` options for handling merge file inputs.
 * - Managing `--stats` to process statistics file names.
 * - Supporting additional flags such as `--print` for standard output behavior.
 * - Checking for mutual exclusions and ensuring proper usage of mandatory options.
 *
 * Memory allocated dynamically within the function needs to be freed by the caller using the
 * appropriate cleanup function (e.g., `free_multiple_arrays`). The provided pointers are updated
 * in-place, and their values vary based on the types of arguments provided.
 *
 * Errors and conditions leading to early termination include:
 * - Invalid combinations of `-o`, `--merge`, `--stats`, and `-m` options.
 * - Missing required file arguments for flags such as `-o`, `--merge`, or `--stats`.
 * - Inappropriate use of exclusive options, e.g., `--stats` with `--merge`.
 * - Memory allocation failures or malformed arguments.
 *
 * @param argc                The count of arguments provided to the program.
 * @param argv                Array of argument strings passed from the command line.
 * @param skipDirs            Pointer to an integer flag indicating whether directories should be skipped.
 * @param sizeThreshold       Pointer to a long long variable for size threshold values.
 * @param outputFileName      Pointer to a string for the output file name.
 * @param outputTmpFileName   Pointer to a string for the temporary output file name.
 * @param tmpFileNames        Pointer to a dynamically allocated array of temporary file names.
 * @param directories         Pointer to a dynamically allocated array of directory names.
 * @param mergeFileNames      Pointer to a dynamically allocated array of merge file names.
 * @param statFileNames       Pointer to a dynamically allocated array of statistics file names.
 * @param directoryCount      Pointer to an integer holding the number of input directories.
 * @param mergeFileName       Pointer to a string for a single merge file name.
 * @param mergeFileCount      Pointer to an integer count of merge files provided.
 * @param statFileCount       Pointer to an integer count of statistics files provided.
 * @param printStd            Pointer to a boolean flag for enabling or disabling standard output.
 * @param parentDirectory     Pointer to a string holding the parent directory for processed steps.
 * @param stepCount           Pointer to an integer for the number of steps.
 *
 * @return                    Returns `EXIT_SUCCESS` (0) on successful processing, or
 *                            `EXIT_FAILURE` (1) if an error occurs (e.g., invalid arguments, allocation failure).
 */
int process_arguments(const int argc, char **argv, int *skipDirs, long long *sizeThreshold, char **outputFileName,
                      char **outputTmpFileName,
                      char ***tmpFileNames, char ***directories, char ***mergeFileNames, char ***statFileNames,
                      int *directoryCount,
                      char **mergeFileName, int *mergeFileCount, int *statFileCount, bool *printStd, char **parentDirectory,
                      int *stepCount) {
    *skipDirs = 0; // Default: don't skip directories
    *sizeThreshold = 0; // Default: no size threshold
    *outputFileName = NULL;
    *outputTmpFileName = NULL;
    *directories = NULL;
    *tmpFileNames = NULL;
    *mergeFileNames = NULL;
    *statFileNames = NULL;
    *directoryCount = 0;
    *mergeFileName = NULL;

    int mergeFileCountTmp = 0;
    int statFileCountTmp = 0;

    // First pass: process output-related options and other mutual exclusions
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                *outputFileName = argv[++i];
            } else {
                fprintf(stderr, "Output file name expected after -o\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--merge") == 0) {
            if (*statFileNames != NULL) {
                fprintf(stderr, "Error: --stats cannot be used with --merge.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }

            if (i + 1 < argc) {
                *mergeFileName = argv[++i];
            } else {
                fprintf(stderr, "File name for --merge is missing\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            if (*statFileNames != NULL) {
                fprintf(stderr, "Error: --stats cannot be used with -m.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }

            // Signal the use of the -m flag
            if (*outputFileName != NULL || *mergeFileName != NULL) {
                fprintf(stderr, "Error: -m cannot be used with --merge.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }

            // Parse file names that follow -m
            for (int j = i + 1; j < argc; j++) {
                if (argv[j][0] == '-') {
                    // Stop processing on the next flag
                    break;
                }

                // Allocate and store merge file names
                *mergeFileNames = realloc(*mergeFileNames, sizeof(char *) * (mergeFileCountTmp + 2));
                if (*mergeFileNames == NULL) {
                    perror("Memory allocation failed for mergeFileNames");
                    free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                    return EXIT_FAILURE;
                }

                // Store the file name
                (*mergeFileNames)[mergeFileCountTmp] = strdup(argv[j]);
                if ((*mergeFileNames)[mergeFileCountTmp] == NULL) {
                    perror("Memory allocation failed for mergeFileNames entry");
                    free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                    return EXIT_FAILURE;
                }

                mergeFileCountTmp++;
                // Ensure NULL-termination
                (*mergeFileNames)[mergeFileCountTmp] = NULL;
                i = j; // Move index to the last processed argument
            }

            // Ensure at least one file was provided after -m
            if (mergeFileCountTmp == 0) {
                fprintf(stderr, "Error: -m requires at least one file name.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--stats") == 0) {
            if (*mergeFileName != NULL || *mergeFileNames != NULL) {
                fprintf(stderr, "Error: --stats cannot be used with --merge or -m.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }

            // Parse file names that follow --stats
            for (int j = i + 1; j < argc; j++) {
                if (argv[j][0] == '-') {
                    // Stop processing on the next flag
                    break;
                }

                // Allocate and store stat file names
                *statFileNames = realloc(*statFileNames, sizeof(char *) * (statFileCountTmp + 2));
                if (*statFileNames == NULL) {
                    perror("Memory allocation failed for statFileNames");
                    free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                    return EXIT_FAILURE;
                }

                // Store the file name
                (*statFileNames)[statFileCountTmp] = strdup(argv[j]);
                if ((*statFileNames)[statFileCountTmp] == NULL) {
                    perror("Memory allocation failed for statFileNames entry");
                    free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                    return EXIT_FAILURE;
                }
                statFileCountTmp++;
                // Ensure NULL-termination
                (*statFileNames)[statFileCountTmp] = NULL;
                i = j; // Move index to the last processed argument
            }
            // Ensure at least one file was provided after --stats
            if (statFileCountTmp == 0) {
                fprintf(stderr, "Error: --stats requires at least one file name.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--print") == 0) {
            *printStd = true;
        }
    }
    if (handle_step_option(argc, argv, stepCount, parentDirectory, outputFileName) == 0) {
        return EXIT_SUCCESS;
    }

    // Ensure --add and -o are not used simultaneously
    if (*outputFileName != NULL && *mergeFileName != NULL) {
        fprintf(stderr, "Error: --merge and -o cannot be used together.\n");
        free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
        return EXIT_FAILURE;
    }

    if (*mergeFileNames != NULL && *mergeFileName != NULL) {
        fprintf(stderr, "Error: -m cannot be used with --merge.\n");
        free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
        return EXIT_FAILURE;
    }

    // Ensure that `-o` (output file name) is provided before proceeding
    if (*outputFileName == NULL && *mergeFileName == NULL && *statFileNames == NULL) {
        fprintf(
            stderr,
            "Error: The -o <outputfile> option is required otherwise use --merge <filename> or --stats <filename(s)>.\n");
        fprintf(
            stderr,
            "Usage: %s [1. 4. <directory_path(s)>] [2. -m <filename(s)>] [-M maxSizeInMB] [--skip-dirs] [1. 2. -o <outputfile>] [4. --merge <filename>] [5. --stats <filename(s)>]\n",
            argv[0]);
        if (argc == 1) {
            exit(EXIT_FAILURE);
        }
        free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
        return EXIT_FAILURE;
    }

    // Check if outputFileName is equal to one of the merge files
    if (*outputFileName != NULL && *mergeFileNames != NULL) {
        for (int i = 0; (*mergeFileNames)[i] != NULL; i++) {
            if (strcmp(*outputFileName, (*mergeFileNames)[i]) == 0) {
                fprintf(stderr, "Error: outputFileName '%s' cannot be the same as a merge file '%s'\n", *outputFileName,
                        (*mergeFileNames)[i]);
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                return EXIT_FAILURE;
            }
        }
    }

    // Check if all merge files exist
    if (*mergeFileNames != NULL) {
        for (int i = 0; (*mergeFileNames)[i] != NULL; i++) {
            if (access((*mergeFileNames)[i], F_OK) != 0) {
                fprintf(stderr, "Error: Merge file '%s' does not exist.\n", (*mergeFileNames)[i]);
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                return EXIT_FAILURE;
            }
        }
    }

    // Prepare a temporary output file name if -o is provided
    if (*mergeFileName != NULL) {
        const size_t outputFileNameLen = strlen(*mergeFileName); // Get the length of the original output file name
        const size_t tmpSuffixLen = strlen(".tmp"); // Length of the ".tmp" suffix

        *outputTmpFileName = malloc(outputFileNameLen + tmpSuffixLen + 1); // +1 for the null terminator
        *outputFileName = malloc(outputFileNameLen + tmpSuffixLen + 1); // +1 for the null terminator
        if (*outputTmpFileName == NULL) {
            free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
            return EXIT_FAILURE; // Exit on memory allocation failure
        }
        strcpy(*outputFileName, *mergeFileName);
        strcat(*outputFileName, ".tmp");
        strcpy(*outputTmpFileName, *mergeFileName);
        strcat(*outputTmpFileName, ".tmp.tmp");
    }

    if (*outputFileName != NULL) {
        const size_t outputFileNameLen = strlen(*outputFileName); // Get the length of the original output file name
        const size_t tmpSuffixLen = strlen(".tmp"); // Length of the ".tmp" suffix

        *outputTmpFileName = malloc(outputFileNameLen + tmpSuffixLen + 1); // +1 for the null terminator
        if (*outputTmpFileName == NULL) {
            free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
            return EXIT_FAILURE; // Exit on memory allocation failure
        }
        strcpy(*outputTmpFileName, *outputFileName);
        strcat(*outputTmpFileName, ".tmp");
    }

    // Process remaining arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--skip-dirs") == 0) {
            *skipDirs = 1; // Enable directory skipping
        } else if (strcmp(argv[i], "-M") == 0) {
            if (i + 1 < argc && isdigit(argv[i + 1][0])) {
                char *endptr;
                const double sizeInMB = strtod(argv[++i], &endptr);
                *sizeThreshold = (long long) (sizeInMB * 1024 * 1024);
            } else {
                fprintf(stderr, "Invalid or missing size argument after -M\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            // Handle the output file
            if (i + 1 < argc) {
                *outputFileName = strdup(argv[++i]); // Copy next argument as output file name
                if (*outputFileName == NULL) {
                    perror("Memory allocation failed (output file name)");
                    free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                    release_temporary_resources(outputTmpFileName, NULL);
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Missing argument after -o\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }
        } else if (argv[i][0] != '-' && *mergeFileNames == NULL && strcmp(argv[i - 1], "--merge") != 0 &&
                   is_directory(argv[i])) {
            // Treat as a directory path (non-option argument)
            *directories = realloc(*directories, sizeof(char *) * (*directoryCount + 2));
            *tmpFileNames = realloc(*tmpFileNames, sizeof(char *) * (*directoryCount + 2));
            if (*directories == NULL || *tmpFileNames == NULL) {
                perror("Memory allocation failed");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }

            // Store the directory argument
            (*directories)[*directoryCount] = strdup(argv[i]);
            if ((*directories)[*directoryCount] == NULL) {
                perror("Memory allocation failed (directory entry)");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }

            // Generate a temporary file name
            char tmpFileNameBuffer[MAX_LINE_LENGTH]; // Assumes a max temporary filename size
            snprintf(tmpFileNameBuffer, sizeof(tmpFileNameBuffer), "%s_tmp%d",
                     *mergeFileName ? *mergeFileName : (*outputFileName ? *outputFileName : "default"),
                     *directoryCount);
            (*tmpFileNames)[*directoryCount] = strdup(tmpFileNameBuffer);
            if ((*tmpFileNames)[*directoryCount] == NULL) {
                perror("Memory allocation failed (temporary file name)");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }

            // Increment directory count and terminate both arrays
            (*directories)[++(*directoryCount)] = NULL;
            (*tmpFileNames)[*directoryCount] = NULL;
        } else if (strcmp(argv[i], "--merge") == 0) {
            // do nothing here
        } else if (strcmp(argv[i], "-m") == 0) {
            // do nothing here
        } else if (strcmp(argv[i], "--stats") == 0) {
            // do nothing here
        } else if (strcmp(argv[i], "--print") == 0) {
            // do nothing here
        }
        else if (strcmp(argv[i], "--step") == 0) {
            // do nothing here
        }
        else {
            if (!belongs_to_array(argv[i], *mergeFileNames, mergeFileCountTmp) && !belongs_to_array(
                    argv[i], *statFileNames,
                    statFileCountTmp) && (*mergeFileName == NULL || *mergeFileName != NULL && strcmp(
                                              *mergeFileName, argv[i]) != 0)) {
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
                release_temporary_resources(outputTmpFileName, NULL);
                return EXIT_FAILURE;
            }
        }
    }
    if (mergeFileNames != NULL && *mergeFileNames != NULL) {
        if (mergeFileCountTmp < 2) {
            fprintf(stderr, "Error: -m option requires at least two file names.\n");
            free_multiple_arrays(directories, tmpFileNames, mergeFileNames, NULL);
            release_temporary_resources(outputTmpFileName, NULL);
            exit(EXIT_FAILURE);
        }
        *mergeFileCount = mergeFileCountTmp;
    }
    if (statFileNames != NULL) {
        *statFileCount = statFileCountTmp;
    }
    if (directories != NULL && *directories != NULL) {
        *directories = remove_duplicate_directories(*directories, *directoryCount, directoryCount);
    }
    return EXIT_SUCCESS;
}

void initializeFileStatistics(FileStatistics *fileStats) {
    if (fileStats == NULL) {
        return; // Handle NULL pointer to avoid segmentation faults
    }
    // Use memset to set all fields to zero
    memset(fileStats, 0, sizeof(FileStatistics));
}

/**
 * Prints detailed statistics of files and directories from the given file statistics data.
 *
 * This function outputs a summary report that includes the count and total
 * size for various file types, as well as overall statistics for files,
 * hidden files, and directories. Each category of file types (e.g., text,
 * music, images) is reported separately.
 *
 * For hidden files, the total number and size are displayed only if there
 * are hidden files present. The total size of all files is displayed in
 * both raw bytes and a human-readable format. The detailed statistics are
 * printed using helper functions.
 *
 * @param fileStats A `FileStatistics` structure containing the statistics
 *                  for different file categories, hidden files, total files,
 *                  total size, and directories.
 */
void printFileStatistics(const FileStatistics fileStats) {
    printf("\nSummary:\n");
    printSizeDetails("Text", fileStats.textFiles, fileStats.textSize);
    printSizeDetails("Music", fileStats.musicFiles, fileStats.musicSize);
    printSizeDetails("Film", fileStats.filmFiles, fileStats.filmSize);
    printSizeDetails("Image", fileStats.imageFiles, fileStats.imageSize);
    printSizeDetails("Compressed", fileStats.compressedFiles, fileStats.compressedSize);
    printSizeDetails("Binary", fileStats.binaryFiles, fileStats.binarySize);
    printSizeDetails("JSON", fileStats.jsonFiles, fileStats.jsonSize);
    printSizeDetails("Csv", fileStats.csvFiles, fileStats.csvSize);
    printSizeDetails("YAML", fileStats.yamlFiles, fileStats.yamlSize);
    printSizeDetails("Python", fileStats.pythonFiles, fileStats.pythonSize);
    printSizeDetails("Java", fileStats.javaFiles, fileStats.javaSize);
    printSizeDetails("Ts", fileStats.tsFiles, fileStats.tsSize);
    printSizeDetails("Js", fileStats.jsFiles, fileStats.jsSize);
    printSizeDetails("Sql", fileStats.sqlFiles, fileStats.sqlSize);
    printSizeDetails("Html", fileStats.htmlFiles, fileStats.htmlSize);
    printSizeDetails("Css", fileStats.cssFiles, fileStats.cssSize);
    printSizeDetails("Xhtml", fileStats.xhtmlFiles, fileStats.xhtmlSize);
    printSizeDetails("Xml", fileStats.xmlFiles, fileStats.xmlSize);
    printSizeDetails("Packages", fileStats.packageFiles, fileStats.packageSize);
    printSizeDetails("Log", fileStats.logFiles, fileStats.logSize);
    printSizeDetails("Class", fileStats.classFiles, fileStats.classSize);
    printSizeDetails("Template", fileStats.templateFiles, fileStats.templateSize);
    printSizeDetails("Pdf", fileStats.pdfFiles, fileStats.pdfSize);
    printSizeDetails("Doc", fileStats.docFiles, fileStats.docSize);
    printSizeDetails("LaTex", fileStats.texFiles, fileStats.texSize);
    printSizeDetails("Calc", fileStats.calcFiles, fileStats.calcSize);
    printSizeDetails("Jar", fileStats.jarFiles, fileStats.jarSize);
    printSizeDetails("C Source", fileStats.cFiles, fileStats.cSize);
    printSizeDetails("EXE", fileStats.exeFiles, fileStats.exeSize);
    printf("------------------------------------\n");
    if (fileStats.hiddenFiles > 0) {
        printf("Total Number of Hidden Files: %d\n", fileStats.hiddenFiles);
        printf("Total Size of Hidden Files: %s (%lld bytes) \n", getFileSizeAsString(fileStats.hiddenFilesSize),
               fileStats.hiddenFilesSize);
    }
    if (fileStats.hiddenDirs > 0) {
        printf("Total Number of Hidden Directories: %d\n", fileStats.hiddenDirs);
        printf("Total Size of Hidden Directories: %s (%lld bytes) \n", getFileSizeAsString(fileStats.hiddenDirsSize),
               fileStats.hiddenDirsSize);
    }
    char *file_size_as_string = getFileSizeAsString(fileStats.totalSize);

    printf("------------------------------------\n");
    printf("Total Number of Directories: %d\n", fileStats.totalDirs);
    printf("Total Number of Files: %d\n", fileStats.totalFiles);
    printf("Total Number of Files and Directories: %d\n", fileStats.totalDirs + fileStats.totalFiles);
    printf("Total Size of Files: %lld bytes (%s)\n", fileStats.totalSize, file_size_as_string);
    free(file_size_as_string);
}

int sort_and_write_results_to_file(char *tmpFileName, char *outputFileName, int *totalCount, int count,
                                   FileEntry *entries, const int acc) {
    // Sorting and writing results to file
    if (count < INITIAL_ENTRIES_CAPACITY) {
        qsort(entries, count, sizeof(FileEntry), compareFileEntries);
        printToFile(entries, count, tmpFileName, NEW); // we treat outputFileName as temp for a while
    } else {
        char command[MAX_LINE_LENGTH];
        printToFile(entries, count, outputFileName, NEW);
        snprintf(command, sizeof(command), "sort --parallel=12 -t %s -k1,1 %s -o %s", SEP, outputFileName, tmpFileName);
        // we treat outputFileName as temp for a while
        const int ret = system(command);
        if (ret == -1 || WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Sort command failed with status: %d\n", WEXITSTATUS(ret));
            free(entries);
            return EXIT_FAILURE;
        }
    }
    int outputCount = 0;
    // Post-processing and final statistics
    read_entries(tmpFileName, &entries, count, &outputCount);
    resizeEntries(&entries, &count); // Resize entries array to actual size
    if (acc) {
        accumulateChildrenAndSize(entries, outputCount);
    }
    *totalCount = outputCount;
    printToFile(entries, outputCount, outputFileName, NEW);

    return EXIT_SUCCESS;
}

void compute_file_statistics(const FileEntry *entries, const int count, FileStatistics *stats, char **directories) {
    int dirCount = 0;
    for (size_t j = 0; directories[j] != NULL; j++) {
        dirCount++;
    }
    int newCount = 0;
    char **unique_directories = remove_duplicate_directories(directories, dirCount, &newCount);

    // Initialize all statistics to 0
    memset(stats, 0, sizeof(FileStatistics));

    for (int i = 0; i < count; i++) {
        const FileEntry *entry = &entries[i];

        // Update global statistics
        for (size_t j = 0; unique_directories[j] != NULL; j++) {
            if (strcmp(entry->path, unique_directories[j]) == 0) {
                stats->totalSize += entry->size;
            }
        }
        if (entry->isDir) {
            stats->totalDirs++;
        } else {
            stats->totalFiles++;
        }

        // File type counters using string comparison
        if (entry->isHidden && !entry->isDir) {
            stats->hiddenFiles++;
            stats->hiddenFilesSize += entry->size;
        } else if (entry->isHidden && entry->isDir) {
            stats->hiddenDirs++;
            stats->hiddenDirsSize += entry->size;
        }
        if (strcmp(entry->type, "T_TEXT") == 0) {
            stats->textFiles++;
            stats->textSize += entry->size;
        } else if (strcmp(entry->type, "T_JSON") == 0) {
            stats->jsonFiles++;
            stats->jsonSize += entry->size;
        } else if (strcmp(entry->type, "T_AUDIO") == 0) {
            stats->musicFiles++;
            stats->musicSize += entry->size;
        } else if (strcmp(entry->type, "T_FILM") == 0) {
            stats->filmFiles++;
            stats->filmSize += entry->size;
        } else if (strcmp(entry->type, "T_IMAGE") == 0) {
            stats->imageFiles++;
            stats->imageSize += entry->size;
        } else if (strcmp(entry->type, "T_COMPRESSED") == 0) {
            stats->compressedFiles++;
            stats->compressedSize += entry->size;
        } else if (strcmp(entry->type, "T_YAML") == 0) {
            stats->yamlFiles++;
            stats->yamlSize += entry->size;
        } else if (strcmp(entry->type, "T_EXE") == 0) {
            stats->exeFiles++;
            stats->exeSize += entry->size;
        } else if (strcmp(entry->type, "T_C") == 0) {
            stats->cFiles++;
            stats->cSize += entry->size;
        } else if (strcmp(entry->type, "T_PYTHON") == 0) {
            stats->pythonFiles++;
            stats->pythonSize += entry->size;
        } else if (strcmp(entry->type, "T_JAVA") == 0) {
            stats->javaFiles++;
            stats->javaSize += entry->size;
        } else if (strcmp(entry->type, "T_LOG") == 0) {
            stats->logFiles++;
            stats->logSize += entry->size;
        } else if (strcmp(entry->type, "T_PACKAGE") == 0) {
            stats->packageFiles++;
            stats->packageSize += entry->size;
        } else if (strcmp(entry->type, "T_CLASS") == 0) {
            stats->classFiles++;
            stats->classSize += entry->size;
        } else if (strcmp(entry->type, "T_TEMPLATE") == 0) {
            stats->templateFiles++;
            stats->templateSize += entry->size;
        } else if (strcmp(entry->type, "T_PDF") == 0) {
            stats->pdfFiles++;
            stats->pdfSize += entry->size;
        } else if (strcmp(entry->type, "T_HTML") == 0) {
            stats->htmlFiles++;
            stats->htmlSize += entry->size;
        } else if (strcmp(entry->type, "T_XML") == 0) {
            stats->xmlFiles++;
            stats->xmlSize += entry->size;
        } else if (strcmp(entry->type, "T_XHTML") == 0) {
            stats->xhtmlFiles++;
            stats->xhtmlSize += entry->size;
        } else if (strcmp(entry->type, "T_TS") == 0) {
            stats->tsFiles++;
            stats->tsSize += entry->size;
        } else if (strcmp(entry->type, "T_JAR") == 0) {
            stats->jarFiles++;
            stats->jarSize += entry->size;
        } else if (strcmp(entry->type, "T_DOC") == 0) {
            stats->docFiles++;
            stats->docSize += entry->size;
        } else if (strcmp(entry->type, "T_CALC") == 0) {
            stats->calcFiles++;
            stats->calcSize += entry->size;
        } else if (strcmp(entry->type, "T_LATEX") == 0) {
            stats->textFiles++;
            stats->textSize += entry->size;
        } else if (strcmp(entry->type, "T_SQL") == 0) {
            stats->sqlFiles++;
            stats->sqlSize += entry->size;
        } else if (strcmp(entry->type, "T_CSV") == 0) {
            stats->csvFiles++;
            stats->csvSize += entry->size;
        } else if (strcmp(entry->type, "T_CSS") == 0) {
            stats->cssFiles++;
            stats->cssSize += entry->size;
        } else if (!entry->isDir) {
            stats->binaryFiles++;
            stats->binarySize += entry->size;
        }
    }
    free_array(&unique_directories);
}

/**
 * Processes a directory and its contents while generating file statistics and managing output files.
 *
 * This function initializes and manages a dynamic task queue to handle directories and their contents.
 * It dynamically allocates memory for file entry structures, processes directories to collect files and subdirectories,
 * sorts the file entries, writes the results to output files, and computes statistical data for files.
 *
 * Tasks include:
 * - Creating and initializing a task queue for directory processing.
 * - Dynamically allocating an array of FileEntry structures to store processed directory and file information.
 * - Adding the given directory to the task queue and removing trailing slashes for consistency.
 * - Processing all directories from the queue while dynamically resizing the FileEntry array as needed.
 * - Sorting and writing the collected file entries to output files using the specified temporary and final file names.
 * - Computing file statistics based on processed entries and updating provided statistics structures.
 * - Freeing allocated resources, including task queues and memory for file entries, to avoid memory leaks.
 *
 * The function ensures thread safety and handles critical errors, such as memory allocation failures or directory processing issues,
 * gracefully by freeing resources and returning an error code.
 *
 * @param directory     A string representing the directory path to be processed.
 * @param outputFileName A string representing the name of the output file to store sorted results.
 * @param tmpFileName   A string representing the temporary file name used during sorting and writing.
 * @param sizeThreshold A long long integer defining the minimum file size (in bytes) to be considered during processing.
 * @param skipDirs      An integer flag (0 or 1) to specify whether to include directories in results and statistics.
 * @param totalCount    A pointer to an integer where the total count of processed entries will be updated.
 *
 * @return Returns EXIT_SUCCESS on successful completion, or EXIT_FAILURE in case of an error.
 */
int processDirectoryTask(const char *directory, char *outputFileName, char *tmpFileName,
                         const long long sizeThreshold, const int skipDirs, int *totalCount) {
    // Initialize task queue
    TaskQueue taskQueue;
    initQueue(&taskQueue, INITIAL_CAPACITY);
    enqueue(&taskQueue, removeTrailingSlash(directory));
    // Allocate memory for file entries
    int capacity = INITIAL_CAPACITY;
    int count = 0;
    FileEntry *entries = malloc(capacity * sizeof(FileEntry));
    if (!entries) {
        perror("malloc");
        freeQueue(&taskQueue);
        return EXIT_FAILURE;
    }
    // Process all directories in the queue
    processDirectory(&taskQueue, &entries, &count, &capacity, sizeThreshold, skipDirs);
    sort_and_write_results_to_file(outputFileName, tmpFileName, totalCount, count, entries, true);
    freeQueue(&taskQueue);
    // free(entries);

    return EXIT_SUCCESS;
}

/**
 * Appends the contents of a temporary file to an output file, and counts the total number of
 * lines in the output file after the operation is completed.
 *
 * This function opens an output file in append mode and a temporary file in read mode. If the
 * temporary file is empty, no changes are made to the output file and the function concludes
 * successfully. Otherwise, it reads the contents of the temporary file in chunks, writing them
 * to the output file. After appending, the output file is reopened in read mode to count the
 * number of newline characters, updating the total line count.
 *
 * Any errors during file operations, such as opening, reading, or writing, result in appropriate
 * error messages being displayed, and the function exits with a failure status.
 *
 * @param tmpFileName     The path to the temporary file whose content will be appended to the
 *                        output file.
 * @param outputFileName  The path to the output file, which will be created if it does not exist
 *                        and appended to if it does.
 * @param totalCount      A pointer to an integer that will be updated with the total number of
 *                        lines in the output file after appending.
 *
 * @return                Returns EXIT_SUCCESS (0) on successful operation, or EXIT_FAILURE (non-zero)
 *                        if any file operation fails.
 */
int append_file(const char *tmpFileName, const char *outputFileName, int *totalCount) {
    // Open the output file in append mode (create it if it doesn't exist)
    FILE *outputFile = fopen(outputFileName, "a");
    if (!outputFile) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }
    FILE *tmpFile = fopen(tmpFileName, "r");
    if (!tmpFile) {
        perror("Error opening temporary file");
        fclose(outputFile); // Close output file before returning
        return EXIT_FAILURE;
    }
    if (fseek(tmpFile, 0, SEEK_END) == 0 && ftell(tmpFile) == 0) {
        printf("Temporary file is empty: %s\n", tmpFileName);
        fclose(tmpFile);
        fclose(outputFile);
        return EXIT_SUCCESS; // Nothing to append for empty temporary file
    }
    rewind(tmpFile); // Reset file pointer to beginning for reading
    char buffer[BUFSIZ]; // A buffer to temporarily hold file data
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), tmpFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, outputFile) < bytesRead) {
            perror("Error writing to output file");
            fclose(tmpFile);
            fclose(outputFile);
            return EXIT_FAILURE;
        }
    }
    // Explicitly flush buffered output
    fflush(outputFile);
    // Clean up: Close both files
    fclose(tmpFile);
    fclose(outputFile);

    // Now, count the number of lines in the output file
    FILE *outputFileForCounting = fopen(outputFileName, "r");
    if (!outputFileForCounting) {
        perror("Error opening output file for counting");
        return EXIT_FAILURE;
    }

    int lineCount = 0;
    char c;
    while ((c = (char) fgetc(outputFileForCounting)) != EOF) {
        if (c == '\n') {
            lineCount++;
        }
    }
    fclose(outputFileForCounting);
    *totalCount = lineCount;

    return EXIT_SUCCESS;
}

/**
 * Combines the statistical data of two `FileStatistics` objects into a new
 * `FileStatistics` object. The resulting object contains the aggregated
 * counts and sizes of files and directories, as well as detailed statistics
 * for various file types.
 *
 * This function is useful for merging file statistics from different sources
 * or directories to generate a cumulative report.
 *
 * @param a Pointer to the first `FileStatistics` object.
 * @param b Pointer to the second `FileStatistics` object.
 *
 * @return A new `FileStatistics` object containing the aggregated
 *         statistical data from the inputs.
 */
FileStatistics addFileStatistics(const FileStatistics *a, const FileStatistics *b) {
    FileStatistics result;

    // Add global statistics
    result.totalSize = a->totalSize + b->totalSize;
    result.totalFiles = a->totalFiles + b->totalFiles;
    result.totalDirs = a->totalDirs + b->totalDirs;

    // Add file type counters
    result.textFiles = a->textFiles + b->textFiles;
    result.musicFiles = a->musicFiles + b->musicFiles;
    result.filmFiles = a->filmFiles + b->filmFiles;
    result.imageFiles = a->imageFiles + b->imageFiles;
    result.binaryFiles = a->binaryFiles + b->binaryFiles;
    result.compressedFiles = a->compressedFiles + b->compressedFiles;
    result.texFiles = a->texFiles + b->texFiles;
    result.jsonFiles = a->jsonFiles + b->jsonFiles;
    result.yamlFiles = a->yamlFiles + b->yamlFiles;
    result.exeFiles = a->exeFiles + b->exeFiles;
    result.templateFiles = a->templateFiles + b->templateFiles;
    result.pdfFiles = a->pdfFiles + b->pdfFiles;
    result.jarFiles = a->jarFiles + b->jarFiles;
    result.htmlFiles = a->htmlFiles + b->htmlFiles;
    result.xhtmlFiles = a->xhtmlFiles + b->xhtmlFiles;
    result.xmlFiles = a->xmlFiles + b->xmlFiles;
    result.tsFiles = a->tsFiles + b->tsFiles;
    result.jsFiles = a->jsFiles + b->jsFiles;
    result.cFiles = a->cFiles + b->cFiles;
    result.pythonFiles = a->pythonFiles + b->pythonFiles;
    result.javaFiles = a->javaFiles + b->javaFiles;
    result.packageFiles = a->packageFiles + b->packageFiles;
    result.logFiles = a->logFiles + b->logFiles;
    result.classFiles = a->classFiles + b->classFiles;
    result.docFiles = a->docFiles + b->docFiles;
    result.calcFiles = a->calcFiles + b->calcFiles;
    result.sqlFiles = a->sqlFiles + b->sqlFiles;
    result.csvFiles = a->csvFiles + b->csvFiles;
    result.cssFiles = a->cssFiles + b->cssFiles;
    result.hiddenFiles = a->hiddenFiles + b->hiddenFiles;
    result.hiddenDirs = a->hiddenDirs + b->hiddenDirs;

    // Add file type sizes
    result.textSize = a->textSize + b->textSize;
    result.musicSize = a->musicSize + b->musicSize;
    result.filmSize = a->filmSize + b->filmSize;
    result.imageSize = a->imageSize + b->imageSize;
    result.binarySize = a->binarySize + b->binarySize;
    result.compressedSize = a->compressedSize + b->compressedSize;
    result.texSize = a->texSize + b->texSize;
    result.jsonSize = a->jsonSize + b->jsonSize;
    result.yamlSize = a->yamlSize + b->yamlSize;
    result.exeSize = a->exeSize + b->exeSize;
    result.classSize = a->classSize + b->classSize;
    result.templateSize = a->templateSize + b->templateSize;
    result.pdfSize = a->pdfSize + b->pdfSize;
    result.jarSize = a->jarSize + b->jarSize;
    result.docSize = a->docSize + b->docSize;
    result.calcSize = a->calcSize + b->calcSize;
    result.cSize = a->cSize + b->cSize;
    result.pythonSize = a->pythonSize + b->pythonSize;
    result.javaSize = a->javaSize + b->javaSize;
    result.packageSize = a->packageSize + b->packageSize;
    result.logSize = a->logSize + b->logSize;
    result.htmlSize = a->htmlSize + b->htmlSize;
    result.xmlSize = a->xmlSize + b->xmlSize;
    result.tsSize = a->tsSize + b->tsSize;
    result.jsSize = a->jsSize + b->jsSize;
    result.xhtmlSize = a->xhtmlSize + b->xhtmlSize;
    result.sqlSize = a->sqlSize + b->sqlSize;
    result.csvSize = a->csvSize + b->csvSize;
    result.cssSize = a->cssSize + b->cssSize;
    result.hiddenFilesSize = a->hiddenFilesSize + b->hiddenFilesSize;
    result.hiddenDirsSize = a->hiddenDirsSize + b->hiddenDirsSize;

    return result;
}

int remove_duplicates(const char *inputFileName, const char *outputFileName) {
    // Open input file for reading
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }
    // Open output file for writing
    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile);
        return EXIT_FAILURE;
    }

    char prevLine[MAX_LINE_LENGTH] = ""; // Store the previous line for duplicate check
    char currLine[MAX_LINE_LENGTH]; // Store the current line being read

    // Read through each line of the input file
    while (fgets(currLine, sizeof(currLine), inputFile) != NULL) {
        // If the current line is different from the previous line, write it to the output file
        if (strcmp(currLine, prevLine) != 0) {
            fputs(currLine, outputFile); // Write the unique line to the output file
            strcpy(prevLine, currLine); // Update the previous line
        }
    }
    // Close files
    fclose(inputFile);
    fclose(outputFile);

    return EXIT_SUCCESS;
}

int copy_file(const char *inputFileName, const char *outputFileName) {
    // Open the input file for reading
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }
    // Open the output file for writing
    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile); // Close the input file before returning
        return EXIT_FAILURE;
    }
    // Buffer to hold chunks of data being copied
    char buffer[MAX_LINE_LENGTH];
    size_t bytesRead;
    // Read from input file and write to output file in chunks
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, outputFile) != bytesRead) {
            perror("Error writing to output file");
            fclose(inputFile);
            fclose(outputFile);
            return EXIT_FAILURE;
        }
    }
    // Check for reading errors
    if (ferror(inputFile)) {
        perror("Error reading from input file");
        fclose(inputFile);
        fclose(outputFile);
        return EXIT_FAILURE;
    }
    // Close both files
    fclose(inputFile);
    fclose(outputFile);

    return EXIT_SUCCESS;
}

double get_time_difference(const struct timeval start, const struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void process_merge_files(char **mergeFileNames, const int mergeFileCount, const char *outputFileName, int *totalCount) {
    int totalCountTmp = 0;
    for (int i = 0; i < mergeFileCount; ++i) {
        const char *tmpFileName = mergeFileNames[i];
        append_file(tmpFileName, outputFileName, &totalCountTmp);
        *totalCount += totalCountTmp;
        totalCountTmp = 0;
    }
}

/**
 * Prints the elapsed time for processing a specific directory or task, formatted to an appropriate
 * precision based on the duration.
 *
 * This function outputs a message indicating the time taken to process a given directory or task.
 * The precision of the displayed time adjusts dynamically: 2 decimal places for times under 1 second,
 * 1 decimal place for times between 1 and 10 seconds, and no decimal places for times of 10 seconds or more.
 * The message is written to a specified file stream.
 *
 * @param directory  The name of the directory or description of the task being processed.
 * @param elapsed    The time taken, in seconds, to process the directory or task.
 * @param output     A pointer to the output file stream where the message will be printed.
 * @param message    A descriptive message about the operation being timed, e.g., "directory".
 */
void print_elapsed_time(const char *directory, const double elapsed, FILE *output, const char *message) {
    if (directory) {
        // If directory is not NULL, include it in the output
        fprintf(output, "Time taken to process %s '%s': %.*f seconds\n",
                message, directory, (elapsed < 1) ? 2 : (elapsed < 10) ? 1 : 0, elapsed);
    } else {
        // If directory is NULL, omit the '%s'
        fprintf(output, "Time taken to process %s: %.*f seconds\n",
                message, (elapsed < 1) ? 2 : (elapsed < 10) ? 1 : 0, elapsed);
    }
}

int array_size(char **array) {
    size_t count = 0;
    while (array && array[count]) {
        count++;
    }
    return count;
}

char **concatenate_string_arrays(char **array1, char **array2) {
    // Step 1: Get the sizes of both input arrays
    int size1 = array_size(array1); // Length of array1
    int size2 = array_size(array2); // Length of array2

    // Step 2: Allocate memory for the resulting array
    char **result = malloc(sizeof(char *) * (size1 + size2 + 1));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation for concatenated array failed.\n");
        return NULL;
    }

    // Step 3: Copy strings from array1 to result
    for (int i = 0; i < size1; i++) {
        result[i] = strdup(array1[i]); // Duplicate strings to avoid memory conflicts
        if (result[i] == NULL) {
            fprintf(stderr, "Memory allocation failed while copying array1.\n");
            // Free already allocated memory
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            return NULL;
        }
    }

    // Step 4: Copy strings from array2 to result
    for (int i = 0; i < size2; i++) {
        result[size1 + i] = strdup(array2[i]); // Duplicate strings
        if (result[size1 + i] == NULL) {
            fprintf(stderr, "Memory allocation failed while copying array2.\n");
            // Free already allocated memory
            for (int j = 0; j < size1 + i; j++) {
                free(result[j]);
            }
            free(result);
            return NULL;
        }
    }

    // Step 5: Null-terminate the resulting array
    result[size1 + size2] = NULL;

    return result;
}

void display_directories_merging(char *mergeFileName, char **directories) {
    fprintf(stdout, "Directories:\n");
    for (int i = 0; directories[i] != NULL; i++) {
        fprintf(stdout, "%d. %s\n", i + 1, directories[i]);
    }
    fprintf(stdout, "will be merged with file %s.\n\n", mergeFileName);
}

int countRowsInFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    int rows = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            rows++;
        }
    }

    fclose(file);
    return rows;
}

void processStatistics(char **stat_file_names, const int stat_file_count, int printStd) {
    FileEntry *entries;
    FileStatistics fileStats;
    char **directories = NULL;
    int rootCount = 0;

    check_input_files(stat_file_names, &directories, &rootCount);
    for (int i = 0; i < stat_file_count; i++) {
        int totalCount = countRowsInFile(stat_file_names[i]);
        entries = malloc(totalCount * sizeof(FileEntry));
        read_entries(stat_file_names[i], &entries, totalCount, &totalCount);
        printf("\nStatistics for file %s: \n", stat_file_names[i]);
        if (printStd) {
            printToStdOut(entries, totalCount);
        }
        compute_file_statistics(entries, totalCount, &fileStats, directories);
        printFileStatistics(fileStats);
    }
}

void generate_tmp_file_names(char **directories, const char *outputFileName, char ***tmpFileNames, char **outputTmpFileName) {
    int count = 0;

    // Count the number of directories
    while (directories[count] != NULL) {
        count++;
    }

    // Allocate memory for tmpFileNames array
    *tmpFileNames = malloc(sizeof(char *) * (count + 1)); // +1 for NULL-termination
    if (*tmpFileNames == NULL) {
        perror("Error allocating memory for tmpFileNames");
        exit(EXIT_FAILURE);
    }

    // Generate outputTmpFileName
    const size_t nameLength = strlen(outputFileName) + 5; // "_tmp" + null terminator
    *outputTmpFileName = malloc(nameLength);
    if (*outputTmpFileName == NULL) {
        perror("Error allocating memory for outputTmpFileName");
        free(*tmpFileNames);
        exit(EXIT_FAILURE);
    }
    snprintf(*outputTmpFileName, nameLength, "%s_tmp", outputFileName);

    // Populate tmpFileNames array
    for (int i = 0; i < count; i++) {
        const size_t tmpLength = strlen(outputFileName) + 10; // "_tmp_" + max digits for 'i' + null terminator
        (*tmpFileNames)[i] = malloc(tmpLength);
        if ((*tmpFileNames)[i] == NULL) {
            perror("Error allocating memory for tmpFileNames element");
            // Free already allocated memory before exiting
            for (int j = 0; j < i; j++) {
                free((*tmpFileNames)[j]);
            }
            free(*tmpFileNames);
            free(*outputTmpFileName);
            exit(EXIT_FAILURE);
        }
        snprintf((*tmpFileNames)[i], tmpLength, "%s_tmp_%d", outputFileName, i);
    }

    // NULL-terminate the tmpFileNames array
    (*tmpFileNames)[count] = NULL;
}

