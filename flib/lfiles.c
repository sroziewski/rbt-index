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
#include "../shared/shared.h"

const char *FILE_TYPES[] = {
    "T_DIR", "T_TEXT", "T_BINARY", "T_IMAGE", "T_JSON", "T_AUDIO", "T_FILM",
    "T_COMPRESSED", "T_YAML", "T_EXE", "T_C", "T_PYTHON", "T_JS",
    "T_JAVA", "T_LOG", "T_PACKAGE", "T_CLASS", "T_TEMPLATE", "T_PHP", "T_MATHEMATICA",
    "T_PDF", "T_JAR", "T_HTML", "T_XML", "T_XHTML", "T_MATLAB", "T_FORTRAN", "T_SCIENCE", "T_CPP",
    "T_TS", "T_DOC", "T_CALC", "T_LATEX", "T_SQL", "T_PRESENTATION", "T_DATA", "T_LIBRARY", "T_OBJECT",
    "T_CSV", "T_CSS", "T_LINK_DIR", "T_LINK_FILE"
};

// Check if the file has the .json extension (case insensitive)
int isJsonFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".json") == 0 || dot && strcasecmp(dot, ".jsonl") == 0
    || dot && strcasecmp(dot, ".ndjson") == 0 || dot && strcasecmp(dot, ".geojson") == 0;
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
    return dot && strcasecmp(dot, ".c") == 0 || dot && strcasecmp(dot, ".h") == 0;
}

int isCppFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".cpp") == 0 || dot && strcasecmp(dot, ".hpp") == 0 ||
           dot && strcasecmp(dot, ".cxx") == 0 || dot && strcasecmp(dot, ".cc") == 0 ||
           dot && strcasecmp(dot, ".hh") == 0 || dot && strcasecmp(dot, ".hxx") == 0 ||
           dot && strcasecmp(dot, ".tpp") == 0 || dot && strcasecmp(dot, ".inl") == 0;
}

// Check if the file has a .py extension (case insensitive)
int isPythonFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".py") == 0 || dot && strcasecmp(dot, ".pyw") == 0 ||
           dot && strcasecmp(dot, ".pyc") == 0 || dot && strcasecmp(dot, ".pyd") == 0;
}

// Check if the file has a .java extension (case insensitive)
int isJavaFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".java") == 0;
}

// Check if the file has a .java extension (case insensitive)
int isCompressedFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".7z") == 0 || dot && strcasecmp(dot, ".tbz") == 0 ||
           dot && strcasecmp(dot, ".bz2") == 0 || dot && strcasecmp(dot, ".tar") == 0 ||
           dot && strcasecmp(dot, ".arj") == 0 || dot && strcasecmp(dot, ".rar") == 0 ||
           dot && strcasecmp(dot, ".zip") == 0 || dot && strcasecmp(dot, ".gz") == 0 ||
           dot && strcasecmp(dot, ".xz") == 0 || dot && strcasecmp(dot, ".tgz") == 0;
}

int isPackageFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".deb") == 0 || dot && strcasecmp(dot, ".rpm") == 0 ||
           dot && strcasecmp(dot, ".pkg") == 0 || dot && strcasecmp(dot, ".msi") == 0 ||
           dot && strcasecmp(dot, ".apk") == 0 || dot && strcasecmp(dot, ".snap") == 0 ||
           dot && strcasecmp(dot, ".flatpak") == 0 || dot && strcasecmp(dot, ".egg") == 0 ||
           dot && strcasecmp(dot, ".pyz") == 0 || dot && strcasecmp(dot, ".whl") == 0;
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
    return dot && strcasecmp(dot, ".pdf") == 0 ||dot && strcasecmp(dot, ".eps") == 0;
}

int isJarFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".jar") == 0 || dot && strcasecmp(dot, ".war") == 0 || dot && strcasecmp(dot, ".ear")
           == 0;
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
    return dot && strcasecmp(dot, ".doc") == 0 || dot && strcasecmp(dot, ".rtf") == 0 ||
           dot && strcasecmp(dot, ".docx") == 0 || dot && strcasecmp(dot, ".odt") == 0 ||
           dot && strcasecmp(dot, ".pages") == 0;
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

int isPresentationFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".ppt") == 0 || dot && strcasecmp(dot, ".pptx") == 0 ||
        dot && strcasecmp(dot, ".odp") == 0 || dot && strcasecmp(dot, ".key") == 0;
}

int isCsvFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".csv") == 0;
}

int isCssFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".css") == 0 || dot && strcasecmp(dot, ".scss") == 0;
}

int isPhpFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".php") == 0 || dot && strcasecmp(dot, ".php3") == 0 || dot &&
           strcasecmp(dot, ".php4") == 0 || dot && strcasecmp(dot, ".php5") == 0 || dot && strcasecmp(dot, ".php6") == 0
           || dot && strcasecmp(dot, ".php7") == 0 || dot && strcasecmp(dot, ".php8") == 0 || dot && strcasecmp(
               dot, ".phtml") == 0;
}

int isMathematicaFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".nb") == 0 || dot && strcasecmp(dot, ".wl") == 0 || dot && strcasecmp(dot, ".mt") ==
           0;
}

int isMatlabFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".m") == 0 || dot && strcasecmp(dot, ".mat") == 0 || dot && strcasecmp(dot, ".mlx") ==
           0 || dot && strcasecmp(dot, ".mex") == 0;
}

int isFortranFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".f") == 0 || dot && strcasecmp(dot, ".f90") == 0 || dot && strcasecmp(dot, ".f95") ==
           0 || dot && strcasecmp(dot, ".for") == 0
           || dot && strcasecmp(dot, ".f77") == 0 || dot && strcasecmp(dot, ".f03") == 0 || dot &&
           strcasecmp(dot, ".f08") == 0 || dot && strcasecmp(dot, ".fpp") == 0 || dot && strcasecmp(dot, ".mod") == 0;
}

int isScienceFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".jnl") == 0 || dot && strcasecmp(dot, ".tif") == 0;
}

int isDataFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".nc") == 0 || dot && strcasecmp(dot, ".grd") == 0 ||
           dot && strcasecmp(dot, ".hdf") == 0 || dot && strcasecmp(dot, ".cdf") == 0 ||
           dot && strcasecmp(dot, ".ncdf") == 0 || dot && strcasecmp(dot, ".pkl") == 0 ||
           dot && strcasecmp(dot, ".bin") == 0 || dot && strcasecmp(dot, ".h5") == 0 ||
           dot && strcasecmp(dot, ".db") == 0 || dot && strcasecmp(dot, ".mdb") == 0 ||
           dot && strcasecmp(dot, ".accdb") == 0 || dot && strcasecmp(dot, ".dat") == 0 ||
           dot && strcasecmp(dot, ".shp") == 0 || dot && strcasecmp(dot, ".npy") == 0 ||
           dot && strcasecmp(dot, ".npz") == 0 || dot && strcasecmp(dot, ".pb") == 0;
}

int isLibFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".so") == 0 || dot && strcasecmp(dot, ".a") == 0 ||
           dot && strcasecmp(dot, ".dll") == 0 || dot && strcasecmp(dot, ".lib") == 0 ||
           dot && strcasecmp(dot, ".ocx") == 0 || dot && strcasecmp(dot, ".dylib") == 0 ||
           dot && strcasecmp(dot, ".bundle") == 0 || dot && strcasecmp(dot, ".rlib") == 0 ||
           dot && strcasecmp(dot, ".klib") == 0 || dot && strcasecmp(dot, ".mod") == 0 ||
           dot && strcasecmp(dot, ".pak") == 0 || dot && strcasecmp(dot, ".framework") == 0;
}

int isObjectFile(const char *filePath) {
    const char *dot = strrchr(filePath, '.');
    return dot && strcasecmp(dot, ".o") == 0 || dot && strcasecmp(dot, ".obj") == 0 ||
           dot && strcasecmp(dot, ".lo") == 0 || dot && strcasecmp(dot, ".ko") == 0 ||
           dot && strcasecmp(dot, ".mex") == 0 || dot && strcasecmp(dot, ".aout") == 0;
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
        if (isCppFile(filePath)) {
            return "T_CPP";
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
        if (isScienceFile(filePath)) {
            return "T_SCIENCE";
        }
        if (isFortranFile(filePath)) {
            return "T_FORTRAN";
        }
        if (isMathematicaFile(filePath)) {
            return "T_MATHEMATICA";
        }
        if (isMatlabFile(filePath)) {
            return "T_MATLAB";
        }
        if (isPhpFile(filePath)) {
            return "T_PHP";
        }
        if (isDataFile(filePath)) {
            return "T_DATA";
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
    if (isDataFile(filePath)) {
        return "T_DATA";
    }
    if (isPresentationFile(filePath)) {
        return "T_PRESENTATION";
    }
    if (isMathematicaFile(filePath)) {
        return "T_MATHEMATICA";
    }
    if (isMatlabFile(filePath)) {
        return "T_MATLAB";
    }
    if (isScienceFile(filePath)) {
        return "T_SCIENCE";
    }
    if (isObjectFile(filePath)) {
        return "T_OBJECT";
    }
    if (isLibFile(filePath)) {
        return "T_LIBRARY";
    }
    if (isFortranFile(filePath)) {
        return "T_FORTRAN";
    }
    if (isPhpFile(filePath)) {
        return "T_PHP";
    }
    if (isCppFile(filePath)) {
        return "T_PHP";
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

void findParent(const char *path, char *parent) {
    const char *last_slash = strrchr(path, '/'); // Locate the last "/" in the path
    if (last_slash != NULL) {
        // If "/" is found, calculate the length of the parent directory part
        const size_t length = last_slash - path;
        if (length == 0) {
            // If the last slash is at the beginning, the parent is root "/"
            strcpy(parent, "/");
        } else {
            // Copy the parent part of the path to the output and null-terminate it
            strncpy(parent, path, length);
            parent[length] = '\0'; // Null terminate
        }
    } else {
        // If no slash is found, we're dealing with a filename in the current directory
        strcpy(parent, ".");
    }
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
            struct stat fileStatCurrentPath;
            if (lstat(currentPath, &fileStatCurrentPath) == 0 && S_ISLNK(fileStatCurrentPath.st_mode)) {
                char target_path[MAX_LINE_LENGTH];
                const ssize_t len = readlink(currentPath, target_path, sizeof(target_path) - 1);
                if (len == -1) {
                    perror("readlink");
                }
                target_path[len] = '\0';
                (*count)++;
                snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s", currentPath);
                (*entries)[current].size = fileStatCurrentPath.st_size;
                (*entries)[current].isDir = 0;
                (*entries)[current].isLink = 1;
                snprintf((*entries)[current].linkTarget, sizeof((*entries)[current].linkTarget), "%s", target_path);
                (*entries)[current].childrenCount = childrenCount; // Store the children count
                snprintf((*entries)[current].type, sizeof((*entries)[current].type), "T_LINK_DIR");
            } else if (S_ISDIR(fileStatCurrentPath.st_mode)) {
                (*count)++;
                snprintf((*entries)[current].path, sizeof((*entries)[current].path), "%s", currentPath);
                (*entries)[current].size = fileStatCurrentPath.st_size;
                (*entries)[current].isDir = 1; // Mark as a directory
                (*entries)[current].isLink = 0;
                (*entries)[current].childrenCount = childrenCount; // Store the children count
                snprintf((*entries)[current].type, sizeof((*entries)[current].type), "T_DIR");
            }
        }

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < numEntries; ++i) {
            char fullPath[MAX_LINE_LENGTH];
            struct stat fileStat;
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, dirEntries[i]);

            struct stat fileStatForParent;
            char parentDir[MAX_LINE_LENGTH];
            findParent(fullPath, parentDir);
            if (lstat(parentDir, &fileStatForParent) == 0 && S_ISLNK(fileStatForParent.st_mode)) {
                //  we don't want to proceed when parent is a link
                continue;
            }

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
                                    lstat(fullPath, &fileStat);
                                    (*entries)[current].size = fileStat.st_size;
                                    (*entries)[current].isDir = 0; // Mark as a file
                                    (*entries)[current].isLink = 0;
                                    (*entries)[current].childrenCount = 0; // Files don't have children
                                    snprintf((*entries)[current].type, sizeof((*entries)[current].type), "%s",
                                             getFileTypeCategory(mimeType, fullPath));

                                    if (S_ISLNK(fileStat.st_mode)) {
                                        snprintf((*entries)[current].type, sizeof((*entries)[current].type),
                                                 "T_LINK_FILE");
                                    }
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
                            (*entries)[current].size = fileStat.st_size;
                            (*entries)[current].isDir = 1; // Mark as a directory
                            (*entries)[current].isLink = 0;
                            (*entries)[current].childrenCount = 0; // Initialize children count (updated when processed)
                            snprintf((*entries)[current].type, sizeof((*entries)[current].type), "T_DIR");
                        }
                        if (!skipDirs) {
                            // If directories need to be enqueued for further exploration
                            struct stat fileStatForQueue;
                            char parent[MAX_LINE_LENGTH];
                            findParent(fullPath, parent);
                            if (lstat(parent, &fileStatForQueue) == 0 && !S_ISLNK(fileStatForQueue.st_mode)) {
                                //  we don't want to proceed when parent is a link
                                enqueue(taskQueue, fullPath);
                            }
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
 * Reads entries from a file into a fixed-size array of FileEntry structures.
 *
 * This function parses a file line by line, extracting information about paths,
 * file types, sizes, and additional metadata. The extracted entries are stored
 * in a preallocated array of FileEntry structures. It supports parsing specific
 * flags (e.g., `F_HIDDEN`, `C_COUNT`, `L_TARGET`) and handles various file types,
 * such as directories and symbolic links. Errors related to parsing or invalid
 * data are logged, but the function continues processing subsequent lines.
 *
 * The function stops reading when the fixed number of entries (`fixed_count`)
 * is reached or when the file ends.
 *
 * @param filename      The path to the file containing file and directory entries in
 *                      a predefined format.
 * @param entries       A pointer to the dynamically allocated array of FileEntry structures.
 *                      This array is preallocated with a fixed size (`fixed_count`) to store
 *                      the entries parsed from the file.
 * @param fixed_count   The maximum number of FileEntry structures to process and store
 *                      in the `entries` array.
 * @param count         A pointer to an integer that stores the number of successfully added
 *                      entries in the `entries` array.
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
        if (strcmp(entry.type, "T_LINK_FILE") == 0) {
            entry.isLink = true;
            token = strtok(NULL, SEP);
            if (strncmp(token, "L_TARGET", 9) == 0) {
                // token = strtok(NULL, "");  // Get the rest of the string after "L_TARGET"
                if (buffer != NULL) {
                    strncpy(entry.linkTarget, buffer, sizeof(entry.linkTarget) - 1);
                    entry.linkTarget[sizeof(entry.linkTarget) - 1] = '\0'; // Ensure null-termination
                } else {
                    fprintf(stderr, "Missing target path after L_TARGET: %s\n", entry.path);
                }
            }
        }
        // Check if the entry is a directory and process extra flags
        if (strcmp(entry.type, "T_DIR") == 0 || strcmp(entry.type, "T_LINK_DIR") == 0) {
            if (strcmp(entry.type, "T_DIR") == 0) {
                entry.isDir = true;
            } else if (strcmp(entry.type, "T_LINK_DIR") == 0) {
                entry.isLink = true;
            }
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
                    entry.isHidden = true;
                } else if (strncmp(token, "L_TARGET", 9) == 0) {
                    token = strtok(NULL, ""); // Get the rest of the string after "L_TARGET"
                    if (token != NULL) {
                        strncpy(entry.linkTarget, token, sizeof(entry.linkTarget) - 1);
                        entry.linkTarget[sizeof(entry.linkTarget) - 1] = '\0'; // Ensure null-termination
                    } else {
                        fprintf(stderr, "Missing target path after L_TARGET: %s\n", entry.path);
                    }
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
 * Writes the information of a file or directory collection into a specified file in a formatted manner.
 *
 * The function iterates over an array of FileEntry structures and writes their attributes to a given
 * file. Each entry's path, size, and type are written. Additional metadata, such as the number of child
 * entries for directories, symbolic link targets, or a hidden file flag, is appended where applicable.
 * Hidden files are identified based on their names starting with a period (`.`). The file is created
 * or opened with the specified mode and closed upon completion.
 *
 * @param entries   A pointer to an array of FileEntry structures containing file and directory data to
 *                  be written to the file.
 * @param count     The number of entries in the `entries` array to be processed.
 * @param filename  The path to the file where the information will be written.
 * @param mode      The mode in which the file will be opened (e.g., "w" for write, "a" for append).
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
        if (entries[i].isDir || strcmp(entries[i].type, "T_LINK_DIR") == 0) {
            fprintf(outputFile, "%sC_COUNT%s%zu", SEP, SEP, entries[i].childrenCount);
        }
        if (strcmp(entries[i].type, "T_LINK_DIR") == 0 || strcmp(entries[i].type, "T_LINK_FILE") == 0) {
            fprintf(outputFile, "%sL_TARGET%s%s", SEP, SEP, entries[i].linkTarget);
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
int read_directories(const char *parentDir, char ***directories, char **rootDirectories, const int step,
                     int *directoryCount) {
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
                (*directoryCount)++;
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
                    fprintf(stderr, "Error: '%s' is not a valid step value (must be an integer between 2 and 10000)\n",
                            argv[i + 1]);
                    exit(EXIT_FAILURE);
                }
                *stepValue = (int) value; // Store the step value
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
    return -1; // "--step" option not provided, that's ok
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
               (strcmp(entries[i].type, "T_DIR") == 0)
                   ? "Dir:"
                   : (strcmp(entries[i].type, "T_FILE") == 0)
                         ? "File:"
                         : (strcmp(entries[i].type, "T_LINK_DIR") == 0 || strcmp(entries[i].type, "T_LINK_FILE") == 0)
                               ? "Link:"
                               : "Unknown:",
               entries[i].path, sizeStr, entries[i].size, entries[i].type);

        // If the entry is a directory, add the count of children
        if (entries[i].isDir || strcmp(entries[i].type, "T_LINK_DIR") == 0) {
            printf(", C_COUNT: %zu", entries[i].childrenCount);
        }
        if (strcmp(entries[i].type, "T_LINK_DIR") == 0 || strcmp(entries[i].type, "T_LINK_FILE") == 0) {
            printf(", L_TARGET: %s", entries[i].linkTarget);
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
 * Processes command-line arguments for a file and directory handling application.
 *
 * This function parses and validates the provided command-line arguments to configure
 * the application's behavior. It handles options for output file specification,
 * merging files, accumulating data, generating file statistics, and more. The function
 * ensures that conflicting options are not used simultaneously and initializes related
 * data structures accordingly. Memory management for dynamically allocated arrays is
 * handled internally, and errors are reported if improper arguments are detected.
 *
 * @param argc              The count of command-line arguments provided by the user.
 * @param argv              An array of character pointers representing the command-line arguments.
 * @param skipDirs          A pointer to an integer flag (0 or 1) indicating whether directories
 *                           should be skipped during processing.
 * @param sizeThreshold     A pointer to a long long integer representing the minimum file size in
 *                           bytes for processing.
 * @param outputFileName    A pointer to a string that stores the name of the output file.
 * @param outputTmpFileName A pointer to a string that stores the name of a temporary output file.
 * @param tmpFileNames      A pointer to an array of strings for temporary file names.
 * @param directories       A pointer to an array of strings representing directory paths to process.
 * @param mergeFileNames    A pointer to an array of strings containing file names to be merged.
 * @param statFileNames     A pointer to an array of strings for file names to use in generating statistics.
 * @param directoryCount    A pointer to an integer tracking the number of directories parsed from arguments.
 * @param mergeFileName     A pointer to a string storing the name of a single file used for a merge operation.
 * @param accFileName       A pointer to a string storing the name of a file used for data accumulation.
 * @param mergeFileCount    A pointer to an integer tracking the number of files specified for merging.
 * @param statFileCount     A pointer to an integer tracking the number of files specified for statistics generation.
 * @param printStd          A pointer to a boolean flag indicating whether to print results to stdout.
 * @param listOnly          A pointer to a boolean flag indicating whether to list files and directories without processing them.
 * @param parentDirectory   A pointer to a string storing the path of the parent directory to process.
 * @param stepCount         A pointer to an integer tracking the number of processing steps to execute.
 * @param accFileCount      A pointer to an integer tracking the number of records read from the accumulated file.
 *
 * @return An integer representing the success or failure of the argument processing:
 *         - `EXIT_SUCCESS` (typically 0) on successful processing of arguments.
 *         - `EXIT_FAILURE` (typically non-zero) if errors are encountered or invalid arguments are provided.
 */
int process_arguments(const int argc, char **argv, int *skipDirs, long long *sizeThreshold, char **outputFileName,
                      char **outputTmpFileName,
                      char ***tmpFileNames, char ***directories, char ***mergeFileNames, char ***statFileNames,
                      int *directoryCount,
                      char **mergeFileName, char **accFileName, int *mergeFileCount, int *statFileCount, bool *printStd,
                      bool *listOnly, char **parentDirectory,
                      int *stepCount, int *accFileCount) {
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
    *accFileName = NULL;

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
        }
        else if (strcmp(argv[i], "--listOnly") == 0) {
            *listOnly = true;
        }
        else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
            printf("Usage: %s [options] <directory_path(s)>\n", argv[0]);
            printf("Options:\n");
            printf("  -o <outputfile>          Specify the output file name.\n");
            printf("  --merge <filename>       Merge operation with the specified file name.\n");
            printf("  -m <filename(s)>         Merge multiple files (specify one or more filenames).\n");
            printf("  --stats <filename(s)>    Generate statistics for one or more specified files.\n");
            printf("  --acc <filename>         Accumulate data from the specified file.\n");
            printf("  --print                  Print the output to stdout.\n");
            printf("  -M <maxSizeInMB>         Specify the maximum file size (in MB).\n");
            printf("  --skip-dirs              Skip processing directories.\n");
            printf("  --help                   Show this help message and exit.\n\n");
            printf("Constraints:\n");
            printf("  - `--merge` cannot be used with `-m` or `--stats`.\n");
            printf("  - `--acc` cannot be used with `-m`, `--merge`, or `--stats`.\n");
            printf("  - `-o` cannot be used with `--merge`.\n");
            printf("  - At least one of `-o`, `--merge`, `--stats`, or `--acc` must be specified.\n\n");
            printf("Examples:\n");
            printf("  %s -o output.txt dir1 dir2\n", argv[0]);
            printf("  %s --merge merged.txt -o output.txt dir1\n", argv[0]);
            printf("  %s --stats stats1.txt stats2.txt\n", argv[0]);
            printf("  %s -m file1.txt file2.txt -o output.txt\n\n", argv[0]);
            printf("Description:\n");
            printf("  This tool processes files and directories based on the specified options.\n");
            printf("  Use it to merge, analyze, and store files with various operational flags.\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "--merge") == 0) {
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
        } else if (strcmp(argv[i], "--acc") == 0) {
            if (*mergeFileName != NULL || *mergeFileNames != NULL || *statFileNames != NULL) {
                fprintf(stderr, "Error: --acc cannot be used with --merge, -m or --stats.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
            if (i + 1 < argc) {
                *accFileName = argv[++i];
                *accFileCount = countRowsInFile(*accFileName);
            } else {
                fprintf(stderr, "File name for --acc is missing\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-m") == 0) {
            if (*accFileName != NULL) {
                fprintf(stderr, "Error: --acc cannot be used with -m.\n");
                free_multiple_arrays(directories, tmpFileNames, mergeFileNames, statFileNames, NULL);
                return EXIT_FAILURE;
            }
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
            "Error: The -o <outputfile> option is required otherwise use --merge <filename> or --stats <filename(s)> or --acc <filename>.\nTry with --help\n");
        fprintf(
            stderr,
            "Usage: %s [1. 4. <directory_path(s)>] [2. -m <filename(s)>] [-M maxSizeInMB] [--skip-dirs] [1. 2. -o <outputfile>] [4. --merge <filename>] [5. --stats <filename(s)>] [6. --acc <filename>]\n",
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
        } else if (strcmp(argv[i], "--step") == 0) {
            // do nothing here
        } else if (strcmp(argv[i], "--acc") == 0) {
            // do nothing here
        }
        else if (strcmp(argv[i], "--listOnly") == 0) {
            // do nothing here
        }
        else {
            if (!belongs_to_array(argv[i], *mergeFileNames, mergeFileCountTmp) && !belongs_to_array(
                    argv[i], *statFileNames,
                    statFileCountTmp) && (*mergeFileName == NULL || *mergeFileName != NULL && strcmp(
                                              *mergeFileName, argv[i]) != 0) &&
                (*accFileName == NULL || *accFileName != NULL && strcmp(*accFileName, argv[i]) != 0)) {
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
    printSizeDetails("Object", fileStats.objFiles, fileStats.objSize);
    printSizeDetails("Php", fileStats.phpFiles, fileStats.phpSize);
    printSizeDetails("Template", fileStats.templateFiles, fileStats.templateSize);
    printSizeDetails("Pdf", fileStats.pdfFiles, fileStats.pdfSize);
    printSizeDetails("Doc", fileStats.docFiles, fileStats.docSize);
    printSizeDetails("LaTex", fileStats.texFiles, fileStats.texSize);
    printSizeDetails("Calc", fileStats.calcFiles, fileStats.calcSize);
    printSizeDetails("Jar", fileStats.jarFiles, fileStats.jarSize);
    printSizeDetails("C Source", fileStats.cFiles, fileStats.cSize);
    printSizeDetails("Cpp Source", fileStats.cppFiles, fileStats.cppSize);
    printSizeDetails("EXE", fileStats.exeFiles, fileStats.exeSize);
    printSizeDetails("Mathematica", fileStats.mathematicaFiles, fileStats.mathematicaSize);
    printSizeDetails("Matlab", fileStats.matlabFiles, fileStats.matlabSize);
    printSizeDetails("Fortran", fileStats.fortranFiles, fileStats.fortranSize);
    printSizeDetails("Science", fileStats.scienceFiles, fileStats.scienceSize);
    printSizeDetails("Data", fileStats.dataFiles, fileStats.dataSize);
    printSizeDetails("Presentation", fileStats.presentationFiles, fileStats.presentationSize);
    printSizeDetails("Library", fileStats.libFiles, fileStats.libSize);

    if (fileStats.links > 0) {
        printf("------------------------------------\n");
        printf("Total Number of Link Files and Directories: %d\n", fileStats.links);
        printf("Total Size of Link Files and Directories: %s (%lld bytes) \n", getFileSizeAsString(fileStats.linksSize),
               fileStats.linksSize);
    }

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
        snprintf(command, sizeof(command), "sort --parallel=12 -t '%s' -k1,1 %s -o %s", SEP, outputFileName,
                 tmpFileName);
        printf("Running command: %s\n", command);
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
        } else if (entry->isLink) {
            stats->links++;
            stats->linksSize += entry->size;
        } else {
            stats->totalFiles++;
        }

        // File type counters using string comparison
        if (entry->isHidden && !entry->isDir && !entry->isLink) {
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
        }
        else if (strcmp(entry->type, "T_PHP") == 0) {
            stats->phpFiles++;
            stats->phpSize += entry->size;
        }
        else if (strcmp(entry->type, "T_MATHEMATICA") == 0) {
            stats->mathematicaFiles++;
            stats->mathematicaSize += entry->size;
        }
        else if (strcmp(entry->type, "T_MATLAB") == 0) {
            stats->matlabFiles++;
            stats->matlabSize += entry->size;
        }
        else if (strcmp(entry->type, "T_FORTRAN") == 0) {
            stats->fortranFiles++;
            stats->fortranSize += entry->size;
        }
        else if (strcmp(entry->type, "T_SCIENCE") == 0) {
            stats->scienceFiles++;
            stats->scienceSize += entry->size;
        }
        else if (strcmp(entry->type, "T_DATA") == 0) {
            stats->dataFiles++;
            stats->dataSize += entry->size;
        }
        else if (strcmp(entry->type, "T_PRESENTATION") == 0) {
            stats->presentationFiles++;
            stats->presentationSize += entry->size;
        }
        else if (strcmp(entry->type, "T_LIBRARY") == 0) {
            stats->libFiles++;
            stats->libSize += entry->size;
        }
        else if (strcmp(entry->type, "T_OBJECT") == 0) {
            stats->objFiles++;
            stats->objSize += entry->size;
        }
        else if (strcmp(entry->type, "T_CPP") == 0) {
            stats->cppFiles++;
            stats->cppSize += entry->size;
        }
        else if (!entry->isDir && !entry->isLink) {
            stats->binaryFiles++;
            stats->binarySize += entry->size;
        }
    }
    free_array(&unique_directories);
}

/**
 * Processes directory tasks, handling directory traversal, file filtering, and
 * outputting results to specified files.
 *
 * This function initializes a task queue for managing directories to be processed and
 * dynamically allocates memory for file entries. It processes directory paths using
 * the `processDirectory` function, updates file statistics, and writes the sorted
 * results to the specified output and temporary files.
 *
 * The task queue ensures that directories are handled sequentially. Memory allocation
 * failures or critical errors during processing are managed by cleaning up resources
 * and returning failure status.
 *
 * @param directory      A string representing the path of the directory to be processed.
 *                       The function ensures that the path does not include a trailing slash.
 * @param outputFileName A string specifying the name of the file where the results
 *                       should be written after processing and sorting.
 * @param tmpFileName    A string specifying the name of the temporary file used during
 *                       intermediate stages of result processing.
 * @param sizeThreshold  A long long integer representing the minimum file size (in bytes)
 *                       that should be considered when processing files.
 * @param skipDirs       An integer flag (0 or 1) indicating whether directories should
 *                       be included in the final output and file statistics.
 * @param totalCount     A pointer to an integer tracking the total number of entries
 *                       processed across all directories.
 * @param listOnly       A pointer to a boolean flag indicating whether the function
 *                       should only list file names without additional processing.
 *
 * @return An integer indicating the success (EXIT_SUCCESS) or failure (EXIT_FAILURE) of the function.
 */
int processDirectoryTask(const char *directory, char *outputFileName, char *tmpFileName,
                         const long long sizeThreshold, const int skipDirs, int *totalCount, const bool *listOnly) {
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
    if (*listOnly) {
        printToFile(entries, count, outputFileName, NEW);
        free(entries);
        freeQueue(&taskQueue);
        printf("------------------------------------\n");
        printf("Results written to file: %s\n", outputFileName);
        exit(EXIT_SUCCESS);
    }
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
        char errorMessage[256];
        snprintf(errorMessage, sizeof(errorMessage), "Error opening temporary file: %s", tmpFileName);
        perror(errorMessage);
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
    result.phpFiles = a->phpFiles + b->phpFiles;
    result.mathematicaFiles = a->mathematicaFiles + b->mathematicaFiles;
    result.matlabFiles = a->matlabFiles + b->matlabFiles;
    result.fortranFiles = a->fortranFiles + b->fortranFiles;
    result.scienceFiles = a->scienceFiles + b->scienceFiles;
    result.dataFiles = a->dataFiles + b->dataFiles;
    result.presentationFiles = a->presentationFiles + b->presentationFiles;
    result.libFiles = a->libFiles + b->libFiles;
    result.objFiles = a->objFiles + b->objFiles;
    result.cppFiles = a->cppFiles + b->cppFiles;

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
    result.phpSize = a->phpSize + b->phpSize;
    result.mathematicaSize = a->mathematicaSize + b->mathematicaSize;
    result.matlabSize = a->matlabSize + b->matlabSize;
    result.fortranSize = a->fortranSize + b->fortranSize;
    result.scienceSize = a->scienceSize + b->scienceSize;
    result.dataSize = a->dataSize + b->dataSize;
    result.presentationSize = a->presentationSize + b->presentationSize;
    result.libSize = a->libSize + b->libSize;
    result.objSize = a->objSize + b->objSize;
    result.cppSize = a->cppSize + b->cppSize;

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

void process_merge_files(char **mergeFileNames, const int mergeFileCount, const char *outputFileName, int *totalCount) {
    int totalCountTmp = 0;
    for (int i = 0; i < mergeFileCount; ++i) {
        const char *tmpFileName = mergeFileNames[i];
        append_file(tmpFileName, outputFileName, &totalCountTmp);
        *totalCount += totalCountTmp;
        totalCountTmp = 0;
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

void generate_tmp_file_names(char **directories, const char *outputFileName, char ***tmpFileNames,
                             char **outputTmpFileName) {
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
