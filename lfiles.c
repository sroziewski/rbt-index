#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <omp.h>
#include <magic.h>
#include <pthread.h>
#include <errno.h>
#include <libgen.h>
#include <ctype.h>

#define INITIAL_CAPACITY 10
#define RESIZE_FACTOR 2

typedef struct FileEntry {
    char path[1024];
    off_t size;
    char type[128];
} FileEntry;

typedef struct TaskQueue {
    char **tasks;
    int head;
    int tail;
    int capacity;
    pthread_mutex_t mutex;
} TaskQueue;

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

void processDirectory(TaskQueue *taskQueue, FileEntry **entries, int *count, int *capacity,
                      long long *totalSize, int *totalFiles, int *totalDirs,
                      int *textFiles, long long *textSize,
                      int *musicFiles, long long *musicSize,
                      int *filmFiles, long long *filmSize,
                      int *imageFiles, long long *imageSize,
                      int *binaryFiles, long long *binarySize,
                      int *compressedFiles, long long *compressedSize,
                      int *hiddenFiles, long long *hiddenSize,
                      int *jsonFiles, long long *jsonSize,
                      int *yamlFiles, long long *yamlSize,
                      int *exeFiles, long long *exeSize,
                      int *cFiles, long long *cSize,
                      int *pythonFiles, long long *pythonSize,
                      int *javaFiles, long long *javaSize,
                      int *packageFiles, long long *packageSize,
                      int *logFiles, long long *logSize,
                      int *classFiles, long long *classSize,
                      int *templateFiles, long long *templateSize,
                      int *pdfFiles, long long *pdfSize,
                      int *jarFiles, long long *jarSize,
                      int *htmlFiles, long long *htmlSize,
                      int *xhtmlFiles, long long *xhtmlSize,
                      int *xmlFiles, long long *xmlSize,
                      int *tsFiles, long long *tsSize,
                      int *jsFiles, long long *jsSize,
                      int *docFiles, long long *docSize,
                      int *calcFiles, long long *calcSize,
                      int *texFiles, long long *texSize,
                      int *sqlFiles, long long *sqlSize,
                      int *csvFiles, long long *csvSize,
                      int *cssFiles, long long *cssSize,
                      int skipDirs, long long sizeThreshold) {
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
        (*totalDirs)++;

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
        }
        closedir(dp);

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < numEntries; ++i) {
            char fullPath[1024];
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
                                    int current = *count;
                                    (*count)++;
                                    snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s",
                                             fullPath);
                                    (*entries)[current].size = fileStat.st_size;
                                    snprintf((*entries)[current].type, sizeof((*entries)[current].type), "%s",
                                             getFileTypeCategory(mimeType, fullPath));
                                }
#pragma omp atomic
                                (*totalSize) += fileStat.st_size;

#pragma omp atomic
                                (*totalFiles)++;

                                const char *fileType = getFileTypeCategory(mimeType, fullPath);
                                if (dirEntries[i][0] == '.') {
#pragma omp atomic
                                    (*hiddenFiles)++;
#pragma omp atomic
                                    (*hiddenSize) += fileStat.st_size;
                                }
                                if (strcmp(fileType, "T_TEXT") == 0) {
#pragma omp atomic
                                    (*textFiles)++;
#pragma omp atomic
                                    (*textSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JSON") == 0) {
#pragma omp atomic
                                    (*jsonFiles)++;
#pragma omp atomic
                                    (*jsonSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_AUDIO") == 0) {
#pragma omp atomic
                                    (*musicFiles)++;
#pragma omp atomic
                                    (*musicSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_FILM") == 0) {
#pragma omp atomic
                                    (*filmFiles)++;
#pragma omp atomic
                                    (*filmSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_IMAGE") == 0) {
#pragma omp atomic
                                    (*imageFiles)++;
#pragma omp atomic
                                    (*imageSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_COMPRESSED") == 0) {
#pragma omp atomic
                                    (*compressedFiles)++;
#pragma omp atomic
                                    (*compressedSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_YAML") == 0) {
#pragma omp atomic
                                    (*yamlFiles)++;
#pragma omp atomic
                                    (*yamlSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_EXE") == 0) {
#pragma omp atomic
                                    (*exeFiles)++;
#pragma omp atomic
                                    (*exeSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_C") == 0) {
#pragma omp atomic
                                    (*cFiles)++;
#pragma omp atomic
                                    (*cSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PYTHON") == 0) {
#pragma omp atomic
                                    (*pythonFiles)++;
#pragma omp atomic
                                    (*pythonSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JAVA") == 0) {
#pragma omp atomic
                                    (*javaFiles)++;
#pragma omp atomic
                                    (*javaSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_LOG") == 0) {
#pragma omp atomic
                                    (*logFiles)++;
#pragma omp atomic
                                    (*logSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PACKAGE") == 0) {
#pragma omp atomic
                                    (*packageFiles)++;
#pragma omp atomic
                                    (*packageSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CLASS") == 0) {
#pragma omp atomic
                                    (*classFiles)++;
#pragma omp atomic
                                    (*classSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_TEMPLATE") == 0) {
#pragma omp atomic
                                    (*templateFiles)++;
#pragma omp atomic
                                    (*templateSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_PDF") == 0) {
#pragma omp atomic
                                    (*pdfFiles)++;
#pragma omp atomic
                                    (*pdfSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JAR") == 0) {
#pragma omp atomic
                                    (*jarFiles)++;
#pragma omp atomic
                                    (*jarSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_HTML") == 0) {
#pragma omp atomic
                                    (*htmlFiles)++;
#pragma omp atomic
                                    (*htmlSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_XML") == 0) {
#pragma omp atomic
                                    (*xmlFiles)++;
#pragma omp atomic
                                    (*xmlSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_XHTML") == 0) {
#pragma omp atomic
                                    (*xhtmlFiles)++;
#pragma omp atomic
                                    (*xhtmlSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_TS") == 0) {
#pragma omp atomic
                                    (*tsFiles)++;
#pragma omp atomic
                                    (*tsSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_JS") == 0) {
#pragma omp atomic
                                    (*jsFiles)++;
#pragma omp atomic
                                    (*jsSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_DOC") == 0) {
#pragma omp atomic
                                    (*docFiles)++;
#pragma omp atomic
                                    (*docSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CALC") == 0) {
#pragma omp atomic
                                    (*calcFiles)++;
#pragma omp atomic
                                    (*calcSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_LATEX") == 0) {
#pragma omp atomic
                                    (*texFiles)++;
#pragma omp atomic
                                    (*texSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_SQL") == 0) {
#pragma omp atomic
                                    (*sqlFiles)++;
#pragma omp atomic
                                    (*sqlSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CSV") == 0) {
#pragma omp atomic
                                    (*csvFiles)++;
#pragma omp atomic
                                    (*csvSize) += fileStat.st_size;
                                } else if (strcmp(fileType, "T_CSS") == 0) {
#pragma omp atomic
                                    (*cssFiles)++;
#pragma omp atomic
                                    (*cssSize) += fileStat.st_size;
                                } else {
#pragma omp atomic
                                    (*binaryFiles)++;
#pragma omp atomic
                                    (*binarySize) += fileStat.st_size;
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
                        enqueue(taskQueue, fullPath);
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

void printSizeDetails(FILE *outputFile, const char *type, const int count, const long long size) {
    if (count > 0) {
        printf("Total Number of %s Files: %d\n", type, count);
        char *sizeStr = getFileSizeAsString(size);
        printf("Total Size of %s Files: %lld bytes (%s)\n", type, size, sizeStr);
        free(sizeStr);
    }
}
