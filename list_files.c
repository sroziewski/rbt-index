#include "flib/lfiles.h"
/**
 * @brief Entry point of the program. Processes directories and files based on given arguments.
 *
 * This function processes command line arguments, initializes processing resources, and handles
 * directories and files based on specified thresholds, identifying details such as file types, sizes,
 * and counts for different categories. Sorting and summarizing results are performed on the output.
 * If needed, temporary resources and external sorting are utilized. Finally, the results are formatted
 * and saved to a specified file.
 *
 * @param argc Argument count.
 * @param argv Argument vector containing program arguments.
 * @return EXIT_SUCCESS on successful execution; EXIT_FAILURE on errors.
 *
 * This function performs the following steps:
 * - Parses command line arguments.
 * - Initializes OpenMP with the maximum number of threads.
 * - Creates and initializes a task queue to manage directory processing.
 * - Allocates memory for storing file entries.
 * - Processes directories and files iteratively, collecting statistics such as file counts, sizes,
 *   and classifications by type (e.g., text, music, binaries, etc.).
 * - If the number of entries exceeds a specific threshold, employs external sorting using temporary
 *   files; otherwise, uses in-memory sorting.
 * - Accumulates children statistics and total sizes of entries.
 * - Produces detailed outputs and summaries for each type of file.
 * - Releases all allocated resources and temporary files upon completion.
 *
 * The processed output includes statistics such as:
 * - Total number of files categorized by type (e.g., "Hidden", "Text", "Music").
 * - Total sizes of files for each category.
 * - Number of directories encountered.
 * - Overall statistics for the total count and size of files.
 */
int main(const int argc, char *argv[]) {
    long long sizeThreshold = 0;
    int skipDirs = 0;
    char *outputFileName = NULL;
    char *outputTmpFileName = NULL;
    char *addFileName = NULL;

    // Array for storing directory paths
    char **directories = NULL;
    char **tmpFileNames = NULL;
    char **mergeFileNames = NULL;
    int directoryCount = 0;
    int mergeFileCount = 0;
    int totalCount = 0;

    FileEntry *entries;
    FileStatistics fileStats;

    if (process_arguments(argc, argv, &skipDirs, &sizeThreshold, &outputFileName, &outputTmpFileName, &tmpFileNames,
                          &directories, &mergeFileNames, &directoryCount, &addFileName,
                          &mergeFileCount) != EXIT_SUCCESS) {
        printf("Error processing arguments\n");
        free_multiple_arrays(&directories, &tmpFileNames, &mergeFileNames, NULL);
        return EXIT_FAILURE;
    }
    if (mergeFileNames != NULL) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        int rootCount = 0;
        check_merge_files(mergeFileNames, &directories, &rootCount);
        directories = remove_duplicate_directories(directories, rootCount, &rootCount);
        deleteFile(outputFileName);
        deleteFile(outputTmpFileName);
        process_merge_files(mergeFileNames, mergeFileCount, outputTmpFileName, &totalCount);

        entries = malloc(totalCount * sizeof(FileEntry));
        int totalOutputCount = 0;
        read_entries(outputTmpFileName, &entries, totalCount, &totalOutputCount);
        sort_and_write_results_to_file(outputTmpFileName, outputFileName, &totalOutputCount, totalOutputCount, entries,
                                       false);
        copy_file(outputFileName, outputTmpFileName);
        remove_duplicates(outputTmpFileName, outputFileName);

        gettimeofday(&end, NULL);
        // Calculate and display elapsed time
        double elapsed = get_time_difference(start, end);
        fprintf(stdout, "Time taken to process merge files: %.1f seconds\n", elapsed);
    }
    if (addFileName) {
        fprintf(stdout, "\n* The result will be merged with existing output file: %s\n", addFileName);
    }
    if (mergeFileNames == NULL && directories != NULL && directories[0] != NULL) {
        const int numCores = omp_get_max_threads();
        omp_set_num_threads(numCores);
        fprintf(stdout, "Using %d cores.\n", numCores);

        for (int i = 0; directories[i] != NULL && i < argc - 2; i++) {
            fprintf(stdout, "\nProcessing directory: %s\n", directories[i]);
            // Start Timer
            struct timeval start, end;
            gettimeofday(&start, NULL);

            int currentCount = 0;
            if (processDirectoryTask(directories[i], outputFileName, tmpFileNames[i], sizeThreshold,
                                     skipDirs, &currentCount) != EXIT_SUCCESS) {
                fprintf(stderr, "An error occurred while processing directory: %s\n", directories[i]);
            }
            totalCount += currentCount;
            // End Timer
            gettimeofday(&end, NULL);
            // Calculate and display elapsed time
            double elapsed = get_time_difference(start, end);
            fprintf(stdout, "Time taken to process directory '%s': %.1f seconds\n", directories[i], elapsed);
        }
        deleteFile(outputFileName);
        for (int i = 0; directories[i] != NULL && i < argc - 2; i++) {
            int cnt = 0;
            if (append_file(tmpFileNames[i], outputFileName, &cnt) != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to append file %s to %s\n", tmpFileNames[i], outputFileName);
            }
        }
        if (directoryCount > 1) {
            entries = malloc(totalCount * sizeof(FileEntry));
            int totalOutputCount = 0;
            read_entries(outputFileName, &entries, totalCount, &totalOutputCount);
            sort_and_write_results_to_file(outputTmpFileName, outputFileName, &totalOutputCount, totalOutputCount,
                                           entries,
                                           false);
            copy_file(outputFileName, outputTmpFileName);
            remove_duplicates(outputTmpFileName, outputFileName);
        }
    }

    if (addFileName != NULL) {
        int appendedCount = 0;
        if (append_file(outputFileName, addFileName, &appendedCount) != EXIT_SUCCESS) {
            fprintf(stderr, "Failed to append file %s to %s\n", outputFileName, outputTmpFileName);
        }
        entries = malloc(totalCount * sizeof(FileEntry));
        int totalOutputCount = 0;
        read_entries(addFileName, &entries, appendedCount, &totalOutputCount);
        sort_and_write_results_to_file(outputTmpFileName, outputFileName, &totalOutputCount, totalOutputCount, entries,
                                       false);
        copy_file(outputFileName, outputTmpFileName);
        remove_duplicates(outputTmpFileName, addFileName);
    }
    if (mergeFileNames != NULL) {
        read_entries(outputFileName, &entries, totalCount, &totalCount);
    }
    compute_file_statistics(entries, totalCount, &fileStats, directories);

    printFileStatistics(fileStats);

    // printToStdOut(entries, totalOutputCount);
    // printFileStatistics(fileStats);

    free_multiple_arrays(&directories, &tmpFileNames, &mergeFileNames, NULL);
    release_temporary_resources(&outputTmpFileName, NULL);

    deleteFiles(tmpFileNames);
    deleteFile(outputTmpFileName);

    return EXIT_SUCCESS;
}
