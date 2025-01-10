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

#include "../shared/lconsts.h"

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
    if (strstr(mimeType, "image/") == mimeType) {
        return "T_IMAGE";
    }
    if (strstr(mimeType, "video/") == mimeType) {
        return "T_FILM";
    }
    if (strstr(mimeType, "audio/") == mimeType) {
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

const char *getFileName(const char *path) {
    char *pathCopy = strdup(path); // Strdup to avoid modifying the input path
    const char *baseName = basename(pathCopy);
    const char *result = strdup(baseName);
    free(pathCopy);
    return result;
}

int compareFileEntries(const void *a, const void *b) {
    const FileEntry *entryA = (FileEntry *) a;
    const FileEntry *entryB = (FileEntry *) b;
    return strcasecmp(entryA->path, entryB->path); // Case-insensitive comparison
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

void processDirectory(TaskQueue *taskQueue, FileEntry **entries, int *count, int *capacity, FileStatistics *fileStats,
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

#pragma omp atomic
        fileStats->totalDirs++;

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
            (*entries)[current].size = 0; // Size is 0 for directories
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
#pragma omp atomic
                                fileStats->totalSize += fileStat.st_size;

#pragma omp atomic
                                fileStats->totalFiles++;

                                const char *fileType = getFileTypeCategory(mimeType, fullPath);
                                if (dirEntries[i][0] == '.') {
#pragma omp atomic
                                    fileStats->hiddenFiles++;
#pragma omp atomic
                                    fileStats->hiddenSize += fileStat.st_size;
                                }
                                if (strcmp(fileType, "T_TEXT") == 0) {
#pragma omp atomic
                                    fileStats->textFiles++;
#pragma omp atomic
                                    fileStats->textSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JSON") == 0) {
#pragma omp atomic
                                    fileStats->jsonFiles++;
#pragma omp atomic
                                    fileStats->jsonSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_AUDIO") == 0) {
#pragma omp atomic
                                    fileStats->musicFiles++;
#pragma omp atomic
                                    fileStats->musicSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_FILM") == 0) {
#pragma omp atomic
                                    fileStats->filmFiles++;
#pragma omp atomic
                                    fileStats->filmSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_IMAGE") == 0) {
#pragma omp atomic
                                    fileStats->imageFiles++;
#pragma omp atomic
                                    fileStats->imageSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_COMPRESSED") == 0) {
#pragma omp atomic
                                    fileStats->compressedFiles++;
#pragma omp atomic
                                    fileStats->compressedSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_YAML") == 0) {
#pragma omp atomic
                                    fileStats->yamlFiles++;
#pragma omp atomic
                                    fileStats->yamlSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_EXE") == 0) {
#pragma omp atomic
                                    fileStats->exeFiles++;
#pragma omp atomic
                                    fileStats->exeSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_C") == 0) {
#pragma omp atomic
                                    fileStats->cFiles++;
#pragma omp atomic
                                    fileStats->cSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PYTHON") == 0) {
#pragma omp atomic
                                    fileStats->pythonFiles++;
#pragma omp atomic
                                    fileStats->pythonSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JAVA") == 0) {
#pragma omp atomic
                                    fileStats->javaFiles++;
#pragma omp atomic
                                    fileStats->javaSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_LOG") == 0) {
#pragma omp atomic
                                    fileStats->logFiles++;
#pragma omp atomic
                                    fileStats->logSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PACKAGE") == 0) {
#pragma omp atomic
                                    fileStats->packageFiles++;
#pragma omp atomic
                                    fileStats->packageSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CLASS") == 0) {
#pragma omp atomic
                                    fileStats->classFiles++;
#pragma omp atomic
                                    fileStats->classSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_TEMPLATE") == 0) {
#pragma omp atomic
                                    fileStats->templateFiles++;
#pragma omp atomic
                                    fileStats->templateSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PDF") == 0) {
#pragma omp atomic
                                    fileStats->pdfFiles++;
#pragma omp atomic
                                    fileStats->pdfSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JAR") == 0) {
#pragma omp atomic
                                    fileStats->jarFiles++;
#pragma omp atomic
                                    fileStats->jarSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_HTML") == 0) {
#pragma omp atomic
                                    fileStats->htmlFiles++;
#pragma omp atomic
                                    fileStats->htmlSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_XML") == 0) {
#pragma omp atomic
                                    fileStats->xmlFiles++;
#pragma omp atomic
                                    fileStats->xmlSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_XHTML") == 0) {
#pragma omp atomic
                                    fileStats->xhtmlFiles++;
#pragma omp atomic
                                    fileStats->xhtmlSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_TS") == 0) {
#pragma omp atomic
                                    fileStats->tsFiles++;
#pragma omp atomic
                                    fileStats->tsSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JS") == 0) {
#pragma omp atomic
                                    fileStats->jarFiles++;
#pragma omp atomic
                                    fileStats->jarSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_DOC") == 0) {
#pragma omp atomic
                                    fileStats->docFiles++;
#pragma omp atomic
                                    fileStats->docSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CALC") == 0) {
#pragma omp atomic
                                    fileStats->calcFiles++;
#pragma omp atomic
                                    fileStats->calcSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_LATEX") == 0) {
#pragma omp atomic
                                    fileStats->textFiles++;
#pragma omp atomic
                                    fileStats->textSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_SQL") == 0) {
#pragma omp atomic
                                    fileStats->sqlFiles++;
#pragma omp atomic
                                    fileStats->sqlSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CSV") == 0) {
#pragma omp atomic
                                    fileStats->csvFiles++;
#pragma omp atomic
                                    fileStats->csvSize += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CSS") == 0) {
#pragma omp atomic
                                    fileStats->cssFiles++;
#pragma omp atomic
                                    fileStats->cssSize += fileStat.st_size;
                                } else {
#pragma omp atomic
                                    fileStats->binaryFiles++;
#pragma omp atomic
                                    fileStats->binarySize += fileStat.st_size;
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
                            (*entries)[current].size = 0; // Size is 0 for directories
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
        free(*entries);
    }

    // Allocate memory for the fixed amount of entries
    *entries = malloc(fixed_count * sizeof(FileEntry));
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
        char *token = strtok(buffer, "|");
        if (!token) {
            fprintf(stderr, "Error parsing path in line: %s\n", buffer);
            continue;
        }
        strncpy(entry.path, token, sizeof(entry.path) - 1);
        entry.path[sizeof(entry.path) - 1] = '\0';

        token = strtok(NULL, "|");
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

        token = strtok(NULL, "|");
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
            token = strtok(NULL, "|");
            while (token) {
                if (strncmp(token, "C_COUNT", 8) == 0) {
                    token = strtok(NULL, "|");
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
                token = strtok(NULL, "|");
            }
            if (i >= 1 && strcmp((*entries)[i - 1].path, entry.path) == 0 && (*entries)[i - 1].isDir == 1) {
                (*entries)[i - 1] = entry;
                isAdded = true;
            } else if (i >= 2 && strcmp((*entries)[i - 2].path, entry.path) == 0 && (*entries)[i - 2].isDir == 1) {
                (*entries)[i - 2] = entry;
                isAdded = true;
            }
        } else {
            // Look for hidden flag for non-directory entries
            if (strstr(entry.type, "F_HIDDEN") != NULL) {
                entry.isHidden = true;
            }
        }
        // Store the entry in the array
        if (!isAdded && *entry.path != '\0') {
            (*entries)[i++] = entry;
            (*count)++;
        }
    }

    printf("Read %ld entries from the file\n", i);
    fclose(file);
}

void printToFile(FileEntry *entries, const int count, const char *filename) {
    FILE *outputFile = fopen(filename, "w");
    if (!outputFile) {
        perror("Failed to open file");
        return; // Exit the function if the file couldn't be opened
    }
    for (int i = 0; i < count; i++) {
        const char *fileName = getFileName(entries[i].path);
        const int isHidden = (fileName[0] == '.'); // Check if the file is hidden
        fprintf(outputFile, "%s|%ld|%s", entries[i].path, entries[i].size, entries[i].type);
        // If the entry is a directory, add the count of children
        if (entries[i].isDir) {
            fprintf(outputFile, "|C_COUNT|%zu", entries[i].childrenCount);
        }
        if (isHidden) {
            fprintf(outputFile, "|F_HIDDEN");
        }
        fprintf(outputFile, "\n"); // End the line
    }
    fclose(outputFile);
}

void printToStdOut(FileEntry *entries, const int count) {
    for (int i = 0; i < count; i++) {
        const char *fileName = getFileName(entries[i].path);
        const int isHidden = (fileName[0] == '.'); // Check if the file is hidden
        char *sizeStr = getFileSizeAsString(entries[i].size);
        printf("File: %s, Size: %s (%ld bytes), Type: %s",
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

void deleteFile(char *filename) {
    const int result = remove(filename);
    free(filename);
    if (result != 0) {
        perror("Failed to delete the file");
    }
}

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

void accumulateChildrenAndSize(FileEntry *entries, const size_t count) {
    if (entries == NULL || count == 0) {
        return;
    }
    // Iterate through entries
    for (size_t i = 0; i < count; i++) {
        // Only accumulate for directories
        if (entries[i].isDir) {
            entries[i].childrenCount = 0; // Reset the children count
            entries[i].size = 0; // Reset the size (to accumulate later)
            // Check subsequent entries to see if they belong to this directory
            for (size_t j = i + 1; j < count; j++) {
                // Check if the current entry's path is part of the directory
                if (strstr(entries[j].path, entries[i].path) == entries[j].path &&
                    entries[j].path[strlen(entries[i].path)] == '/') {
                    // Current directory is a parent of entries[j]
                    entries[i].childrenCount++;
                    entries[i].size += entries[j].size;
                } else {
                    if (strstr(entries[j].path, entries[i].path) != entries[j].path) {
                        // Stop checking if subsequent paths are no longer under this directory
                        break;
                    }
                }
            }
        }
    }
}

void free_directories(char ***directories) {
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

/**
 * Processes command line arguments and extracts information for further processing.
 *
 * This method parses arguments passed via command line and initializes several variables
 * holding configuration information such as flags, thresholds, filenames, and directories.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of strings representing the command line arguments.
 * @param skipDirs A pointer to an integer that will be set to 1 if the `--skip-dirs` flag is used, otherwise 0.
 * @param sizeThreshold A pointer to a `long long` that will hold the size threshold in bytes (converted from MB provided via `-M` flag).
 * @param originalFileName A pointer to a char pointer that will be set to the address of the original output file name (provided via `-o`).
 * @param tmpFileName A pointer to a char pointer that will point to a dynamically allocated temporary file name (based on the original).
 * @param directories A pointer to a dynamic array of strings representing directory paths passed as positional arguments.
 * @param directoryCount A pointer to an integer that will be set to the number of directories provided as input.
 * @return Returns `EXIT_SUCCESS` (value 0) on success, or `EXIT_FAILURE` (value 1) if any error occurs (e.g., invalid arguments or memory allocation failure).
 */
int process_arguments(const int argc, char **argv, int *skipDirs, long long *sizeThreshold, char **originalFileName,
                      char **tmpFileName, char ***directories, int *directoryCount) {
    *skipDirs = 0; // Default: don't skip directories
    *sizeThreshold = 0; // Default: no size threshold
    *originalFileName = NULL;
    *tmpFileName = NULL;
    *directories = NULL;
    *directoryCount = 0;

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
                release_temporary_resources(tmpFileName, *directories, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                *originalFileName = argv[++i];
                *tmpFileName = malloc(strlen(*originalFileName) + 5); // Allocate memory for tmpFileName
                if (*tmpFileName == NULL) {
                    perror("Memory allocation failed (tmpFileName)");
                    release_temporary_resources(tmpFileName, *directories, NULL);
                    return EXIT_FAILURE;
                }
                strcpy(*tmpFileName, *originalFileName);
                strcat(*tmpFileName, "tmp"); // Appends "tmp" to tmpFileName
            } else {
                fprintf(stderr, "Output file name expected after -o\n");
                release_temporary_resources(tmpFileName, *directories, NULL);
                return EXIT_FAILURE;
            }
        } else if (argv[i][0] != '-') {
            // Treat as a directory path (non-option argument)
            *directories = realloc(*directories, sizeof(char *) * (*directoryCount + 2));
            // +2 for new entry & NULL terminator
            if (*directories == NULL) {
                perror("Memory allocation failed (directories)");
                release_temporary_resources(tmpFileName, *directories, NULL);
                return EXIT_FAILURE;
            }
            (*directories)[*directoryCount] = strdup(argv[i]);
            if ((*directories)[*directoryCount] == NULL) {
                perror("Memory allocation failed (directory entry)");
                release_temporary_resources(tmpFileName, *directories, NULL);
                return EXIT_FAILURE;
            }
            (*directories)[++(*directoryCount)] = NULL; // Null-terminate the array
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            release_temporary_resources(tmpFileName, *directories, NULL);
            return EXIT_FAILURE;
        }
    }

    // Ensure -o parameter is used
    if (!*originalFileName) {
        fprintf(stderr, "Error: The -o <outputfile> option is required.\n");
        fprintf(stderr, "Usage: %s <directory_path(s)> [-M maxSizeInMB] [--skip-dirs] -o <outputfile>\n", argv[0]);
        release_temporary_resources(tmpFileName, *directories, NULL);
        return EXIT_FAILURE;
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