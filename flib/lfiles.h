#ifndef L_FILES_H
#define L_FILES_H

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
#include "../shared/lconsts.h"

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

int isCsvFile(const char *filePath);

int isCssFile(const char *filePath);

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

void processDirectory(TaskQueue *taskQueue, FileEntry **entries, int *count, int *capacity, FileStatistics *fileStats,
                      long long sizeThreshold, int skipDirs);

// File size string utility
char *getFileSizeAsString(long long fileSizeBytes);

// Print size details
void printSizeDetails(const char *type, int count, long long size);

void release_temporary_resources(char **first, ...);

void free_directories(char ***directories);

void read_entries(const char *filename, FileEntry **entries, size_t fixed_count, int *count);

void printToStdOut(FileEntry *entries, int count);

void printToFile(FileEntry *entries, int count, const char *filename);

void deleteFile(char *filename);

char *removeTrailingSlash(const char *token);

int findEntryIndexAdded(const FileEntry *entries, int count, const char *path);

void resizeEntries(FileEntry **entries, int *count);

void accumulateChildrenAndSize(FileEntry *entries, size_t count);

int process_arguments(int argc, char **argv, int *skipDirs, long long *sizeThreshold, char **outputFileName, char **tmpFileName, char ***directories, int *directoryCount);

void initializeFileStatistics(FileStatistics *fileStats);

void printFileStatistics(FileStatistics fileStats);

#endif // L_FILES_H
