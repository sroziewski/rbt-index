#include "flib/lfiles.h"
#include "shared/shared.h"

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
    char *mergeFileName = NULL;
    char *parentDirectory = NULL;
    char *accFileName = NULL;
    // Array for storing directory paths
    char **directories = NULL;
    char **tmpFileNames = NULL;
    char **mergeFileNames = NULL;
    char **statFileNames = NULL;
    char **rootDirectories = NULL;
    int directoryCount = 0;
    int mergeFileCount = 0;
    int statFileCount = 0;
    int accFileCount = 0;
    int totalCount = 0;
    int stepCount = 0;
    bool printStd = false;

    FileEntry *entries;
    FileStatistics fileStats;

    if (process_arguments(argc, argv, &skipDirs, &sizeThreshold, &outputFileName, &outputTmpFileName, &tmpFileNames,
                          &directories, &mergeFileNames, &statFileNames, &directoryCount, &mergeFileName, &accFileName,
                          &mergeFileCount, &statFileCount, &printStd, &parentDirectory, &stepCount, &accFileCount) != EXIT_SUCCESS) {
        printf("Error processing arguments\n");
        free_multiple_arrays(&directories, &tmpFileNames, &mergeFileNames, NULL);
        return EXIT_FAILURE;
    }
    if (parentDirectory != NULL) {
        fprintf(stdout, "Parent directory: %s\n", parentDirectory);
        fprintf(stdout, "Step count: %d\n\n", stepCount);
        int rootCount = 0;
        if (is_file(outputFileName)) {
            check_input_files(&outputFileName, &rootDirectories, &rootCount);
        }
        else {
            rootDirectories = malloc(sizeof(char *));
            rootDirectories[0] = NULL;
            rootCount = 1;
        }
        read_directories(parentDirectory, &directories, rootDirectories, stepCount, &directoryCount);
        generate_tmp_file_names(directories, outputFileName, &tmpFileNames, &outputTmpFileName);
    }
    if (mergeFileNames != NULL) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        int rootCount = 0;
        check_input_files(mergeFileNames, &directories, &rootCount);
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
        print_elapsed_time(NULL, elapsed, stdout, "merge files");
        fprintf(stdout, "Time taken to process merge files: %.1f seconds\n", elapsed);
    }
    if (mergeFileName != NULL) {
        display_directories_merging(mergeFileName, directories);
    }
    if (statFileNames == NULL && mergeFileNames == NULL && directories != NULL && directories[0] != NULL) {
        const int numCores = omp_get_max_threads();
        omp_set_num_threads(numCores);
        fprintf(stdout, "Using %d cores.\n", numCores);
        int const numIters = parentDirectory == NULL ? argc - 2 : directoryCount;
        for (int i = 0; directories[i] != NULL && i < numIters; i++) {
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
            print_elapsed_time(directories[i], elapsed, stdout, "directory");
        }
        deleteFile(outputFileName);
        for (int i = 0; directories[i] != NULL && i < argc - 2; i++) {
            int cnt = 0;
            if (append_file(tmpFileNames[i], outputFileName, &cnt) != EXIT_SUCCESS) {
                fprintf(stderr, "Failed to append file %s to %s\n", tmpFileNames[i], outputFileName);
            }
        }
        entries = malloc(totalCount * sizeof(FileEntry));
        int totalOutputCount = 0;
        read_entries(outputFileName, &entries, totalCount, &totalOutputCount);
        if (directoryCount > 1) {
            sort_and_write_results_to_file(outputTmpFileName, outputFileName, &totalOutputCount, totalOutputCount,
                                           entries, false);
            copy_file(outputFileName, outputTmpFileName);
            remove_duplicates(outputTmpFileName, outputFileName);
            read_entries(outputFileName, &entries, totalCount, &totalOutputCount);
        }
    }

    if (mergeFileName != NULL) {
        fprintf(stdout, "\n* The result will be merged with existing output file: %s\n", mergeFileName);
        int rootCount = -1;
        check_input_files(&mergeFileName, &mergeFileNames, &rootCount);
        directories = concatenate_string_arrays(directories, mergeFileNames);
        rootCount = array_size(directories);
        directories = remove_duplicate_directories(directories, rootCount, &rootCount);
        int appendedCount = 0;
        if (append_file(outputFileName, mergeFileName, &appendedCount) != EXIT_SUCCESS) {
            fprintf(stderr, "Failed to append file %s to %s\n", outputFileName, outputTmpFileName);
        }
        entries = malloc(totalCount * sizeof(FileEntry));
        int totalOutputCount = 0;
        read_entries(mergeFileName, &entries, appendedCount, &totalOutputCount);
        sort_and_write_results_to_file(outputTmpFileName, outputFileName, &totalOutputCount, totalOutputCount, entries,
                                       false);
        copy_file(outputFileName, outputTmpFileName);
        remove_duplicates(outputTmpFileName, mergeFileName);
        read_entries(mergeFileName, &entries, totalOutputCount, &totalCount);
        deleteFile(outputFileName);
    }
    if (mergeFileName == NULL && mergeFileNames == NULL && directories == NULL && statFileNames != NULL) {
        processStatistics(statFileNames, statFileCount, printStd);
    }
    if (mergeFileName == NULL && mergeFileNames != NULL) {
        read_entries(outputFileName, &entries, totalCount, &totalCount);
    }
    if (accFileName != NULL) {
        read_entries(accFileName, &entries, accFileCount, &totalCount);
        accumulateChildrenAndSize(entries, totalCount);
        printToFile(entries, totalCount, outputFileName, NEW);
        int rootCount = -1;
        check_input_files(&accFileName, &directories, &rootCount);
    }
    if (statFileNames == NULL) {
        compute_file_statistics(entries, totalCount, &fileStats, directories);
        if (printStd) {
            printToStdOut(entries, totalCount);
        }
        printFileStatistics(fileStats);
    }
    deleteFiles(tmpFileNames);
    deleteFile(outputTmpFileName);
    free_multiple_arrays(&tmpFileNames, &mergeFileNames, NULL);
    release_temporary_resources(&outputTmpFileName, NULL);

    return EXIT_SUCCESS;
}
