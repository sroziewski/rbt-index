#ifndef LCONSTS_H
#define LCONSTS_H

#include <pthread.h>
#include <sys/types.h>

// Constants
#define RESIZE_FACTOR 2
#define INITIAL_CAPACITY 128
#define MAX_LINE_LENGTH 4096
#define INITIAL_ENTRIES_CAPACITY 400000

// Structures
typedef struct FileEntry {
    char path[MAX_LINE_LENGTH];
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

#endif // LCONSTS_H