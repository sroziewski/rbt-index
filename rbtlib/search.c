#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include "rbtree.h"
#include "search.h"

int MAX_THREADS = 1;

// Global thread counter
int active_threads = 0;
pthread_mutex_t thread_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void search_tree_by_filename_and_type(Node *root, const char *namePattern, const char *targetType) {
    if (root == NULL) {
        return;
    }
    // printf("Active threads: %d\n", active_threads);
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
    if (regexec(&regex, root->key.name, 0, NULL, 0) == 0 && strcmp(root->key.type, targetType) == 0) {
        printf("Found file: %s (Type: %s, Size: %zu, Path: %s)\n",
               root->key.name, root->key.type, root->key.size, root->key.path);
    }

    // Free the regex memory after usage
    regfree(&regex);

    // Initiate thread creation or run sequentially if max threads are reached
    pthread_t leftThread, rightThread;
    SearchArgs leftArgs = {root->left, namePattern, targetType};
    SearchArgs rightArgs = {root->right, namePattern, targetType};

    int create_left_thread = 0, create_right_thread = 0;

    // Check and increment the global thread counter
    pthread_mutex_lock(&thread_counter_mutex);
    if (active_threads < MAX_THREADS) {
        active_threads++;
        create_left_thread = 1;
    }
    if (active_threads < MAX_THREADS) {
        active_threads++;
        create_right_thread = 1;
    }
    pthread_mutex_unlock(&thread_counter_mutex);

    // Create or execute the left subtree search
    if (create_left_thread) {
        pthread_create(&leftThread, NULL, search_tree_thread, &leftArgs);
    } else {
        search_tree_by_filename_and_type(root->left, namePattern, targetType);
    }

    // Create or execute the right subtree search
    if (create_right_thread) {
        pthread_create(&rightThread, NULL, search_tree_thread, &rightArgs);
    } else {
        search_tree_by_filename_and_type(root->right, namePattern, targetType);
    }

    // Join threads if they were created
    if (create_left_thread) {
        pthread_join(leftThread, NULL);
        pthread_mutex_lock(&thread_counter_mutex);
        active_threads--;
        pthread_mutex_unlock(&thread_counter_mutex);
    }
    if (create_right_thread) {
        pthread_join(rightThread, NULL);
        pthread_mutex_lock(&thread_counter_mutex);
        active_threads--;
        pthread_mutex_unlock(&thread_counter_mutex);
    }
}

void *search_tree_thread(void *args) {
    const SearchArgs *searchArgs = (SearchArgs *) (args);
    search_tree_by_filename_and_type(searchArgs->root, searchArgs->namePattern, searchArgs->targetType);
    return NULL;
}

void initialize_threads() {
    const long cores = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of cores
    if (cores <= 0) {
        perror("Failed to determine the number of processors");
        MAX_THREADS = 1; // Fallback to a single thread if detection fails
    } else if (cores >= 14) {
        MAX_THREADS = 14; // Limit to 16 threads if 16 or more cores are available
    } else {
        MAX_THREADS = 6; // Otherwise, use up to 8 threads
    }
    printf("Number of cores available: %ld, MAX_THREADS set to: %d\n", cores, MAX_THREADS);
}
