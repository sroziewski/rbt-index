#ifndef FILE_UTILS_H
#define FILE_UTILS_H

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

// Macros
#define INITIAL_CAPACITY 10
#define RESIZE_FACTOR 2

// Structures
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

// Function declarations

// File type detection functions
int isJsonFile(const char *filePath);
int isYamlFile(const char *filePath);
int isExeFile(const char *filePath);
int isCFile(const char *filePath);
int isPythonFile(const char *filePath);
int isJavaFile(const char *filePath);
int isCompressedFile(const char *filePath);
int isPackageFile(const char *filePath);
int isLogFile(const char *filePath);
int isClassFile(const char *filePath);
int isTemplateFile(const char *filePath);
int isPdfFile(const char *filePath);
int isJarFile(const char *filePath);
int isHtmlFile(const char *filePath);
int isXmlFile(const char *filePath);
int isXhtmlFile(const char *filePath);
int isTsFile(const char *filePath);
int isJsFile(const char *filePath);
int isTexFile(const char *filePath);
int isDocFile(const char *filePath);
int isSqlFile(const char *filePath);
int isCalcFile(const char *filePath);

// File type categorization
const char *getFileTypeCategory(const char *mimeType, const char *filePath);

// Filename extraction utility
const char *getFileName(const char *path);

// File entry comparison
int compareFileEntries(const void *a, const void *b);

// Task queue manipulation functions
void initQueue(TaskQueue *queue, int capacity);
void resizeQueue(TaskQueue *queue);
void enqueue(TaskQueue *queue, const char *path);
char *dequeue(TaskQueue *queue);
void freeQueue(TaskQueue *queue);

// Directory processing function
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
                      int skipDirs, int sizeThreshold);

// File size string utility
char *getFileSizeAsString(double fileSizeBytes);

// Print size details
void printSizeDetails(FILE *outputFile, const char *type, int count, long long size);

#endif // FILE_UTILS_H