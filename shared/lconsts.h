#ifndef LCONSTS_H
#define LCONSTS_H

#include <pthread.h>
#include <sys/types.h>

// Constants
#define RESIZE_FACTOR 2
#define INITIAL_CAPACITY 128
#define MAX_TYPE_LENGTH 128
#define MAX_LINE_LENGTH 4096
#define INITIAL_ENTRIES_CAPACITY 400000

// Structures
typedef struct FileEntry {
    char path[MAX_LINE_LENGTH];
    off_t size;
    int isDir;
    int isHidden;
    size_t childrenCount;
    char type[128];
} FileEntry;

typedef struct TaskQueue {
    char **tasks;
    int head;
    int tail;
    int capacity;
    pthread_mutex_t mutex;
} TaskQueue;

#include <stdbool.h>

// Structure to hold file statistics
typedef struct FileStatistics {
    // Global statistics
    long long totalSize;
    int totalFiles;
    int totalDirs;

    // File type counters
    int textFiles, musicFiles, filmFiles, imageFiles, binaryFiles, compressedFiles, texFiles;
    int jsonFiles, yamlFiles, exeFiles, templateFiles, pdfFiles, jarFiles, htmlFiles;
    int xhtmlFiles, xmlFiles, tsFiles, jsFiles, cFiles, pythonFiles, javaFiles;
    int packageFiles, logFiles, classFiles, docFiles, calcFiles, sqlFiles, csvFiles, cssFiles;
    int hiddenFiles;

    // File type sizes
    long long textSize, musicSize, filmSize, imageSize, binarySize, compressedSize, texSize;
    long long jsonSize, yamlSize, exeSize, classSize, templateSize, pdfSize, jarSize;
    long long docSize, calcSize, cSize, pythonSize, javaSize, packageSize, logSize;
    long long htmlSize, xmlSize, tsSize, jsSize, xhtmlSize, sqlSize, csvSize, cssSize;
    long long hiddenSize;

} FileStatistics;

#endif // LCONSTS_H