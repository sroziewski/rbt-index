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
#include <time.h>
#include <sys/time.h>

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
char *getFileName(const char *path);

// File entry comparison
int compareFileEntries(const void *a, const void *b);

// Task queue manipulation functions
void initQueue(TaskQueue *queue, int capacity);

void resizeQueue(TaskQueue *queue);

void enqueue(TaskQueue *queue, const char *path);

char *dequeue(TaskQueue *queue);

void freeQueue(TaskQueue *queue);

void processDirectory(TaskQueue *taskQueue, FileEntry **entries, int *count, int *capacity,
                      long long sizeThreshold, int skipDirs);

// Print size details
void printSizeDetails(const char *type, int count, long long size);

void release_temporary_resources(char **first, ...);

void free_array(char ***directories);

void read_entries(const char *filename, FileEntry **entries, size_t fixed_count, int *count);

void printToStdOut(FileEntry *entries, int count);

void printToFile(FileEntry *entries, int count, const char *filename, const char *mode);

void deleteFile(char *filename);

char *removeTrailingSlash(const char *token);

int findEntryIndexAdded(const FileEntry *entries, int count, const char *path);

void resizeEntries(FileEntry **entries, int *count);

void accumulateChildrenAndSize(FileEntry *entries, size_t count);

int process_arguments(int argc, char **argv, int *skipDirs, long long *sizeThreshold, char **outputFileName, char **outputTmpFileName, char ***tmpFileNames, char ***directories, char ***mergeFileNames, char ***statFileNames, int *directoryCount, char **mergeFileName, char **accFileName, int *mergeFileCount, int *statFileCount, bool *printStd, char **parentDirectory, int *stepCount, int *accFileCount);

void initializeFileStatistics(FileStatistics *fileStats);

void printFileStatistics(FileStatistics fileStats);

int processDirectoryTask(const char *directory, char *outputFileName, char *tmpFileName, long long sizeThreshold, int skipDirs, int *totalCount);

int append_file(const char *tmpFileName, const char *outputFileName, int *totalCount);

FileStatistics addFileStatistics(const FileStatistics *a, const FileStatistics *b);

int sort_and_write_results_to_file(char *tmpFileName, char *outputFileName, int *totalCount, int count, FileEntry *entries, int acc);

int remove_duplicates(const char *inputFileName, const char *outputFileName);

int copy_file(const char *inputFileName, const char *outputFileName);

void initializeFileEntries(FileEntry *entries, size_t count);

void compute_file_statistics(const FileEntry *entries, int count, FileStatistics *stats, char **directories);

char **remove_duplicate_directories(char **directories, int count, int *new_count);

void free_multiple_arrays(char ***first_directory, ...);

void process_merge_files(char **mergeFileNames, int mergeFileCount, const char *outputFileName, int *totalCount);

void deleteFiles(char **tmpFileNames);

char **concatenate_string_arrays(char **array1, char **array2);

int array_size(char **array);

void display_directories_merging(char *mergeFileName, char **directories);

void replaceChar(char *str, char oldChar, char newChar);

void processStatistics(char **stat_file_names, int stat_file_count, int printStd);

int countRowsInFile(const char *filename);

int read_directories(const char *parentDir, char ***directories, char **rootDirectories, int step, int *directoryCount);

int is_file(const char *path);

void generate_tmp_file_names(char **directories, const char *outputFileName, char ***tmpFileNames, char **outputTmpFileName);

#endif // L_FILES_H
