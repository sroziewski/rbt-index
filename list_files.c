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

    // Array for storing directory paths
    char **directories = NULL;
    char **tmpFileNames = NULL;
    int directoryCount = 0;

    if (process_arguments(argc, argv, &skipDirs, &sizeThreshold, &outputFileName, &tmpFileNames, &directories, &directoryCount) != EXIT_SUCCESS) {
        printf("Error processing arguments\n");
        free_directories(&directories);
        free_directories(&tmpFileNames);
        return EXIT_FAILURE;
    }
    const int numCores = omp_get_max_threads();
    omp_set_num_threads(numCores);
    fprintf(stdout, "Using %d cores.\n", numCores);
    int totalCount = 0;
    FileStatistics fileStats = {0};
    for (int i = 0; directories[i] != NULL && i < argc - 2; i++) {
        printf("\nProcessing directory: %s\n", directories[i]);
        FileStatistics currentFileStats = {0}; // Initialize all fields to 0
        int currentCount = 0;
        if (processDirectoryTask(&currentFileStats, directories[i], outputFileName, tmpFileNames[i], sizeThreshold, skipDirs, &currentCount) != EXIT_SUCCESS) {
            fprintf(stderr, "An error occurred while processing directory: %s\n", directories[i]);
        }
        if (append_file(tmpFileNames[i], outputFileName) != EXIT_SUCCESS) {
            fprintf(stderr, "Failed to append file %s to %s\n", tmpFileNames[i], outputFileName);
        }
        fileStats = addFileStatistics(&fileStats, &currentFileStats);
        totalCount += currentCount;
    }

    FileEntry *entries = malloc(totalCount * sizeof(FileEntry));
    int totalOutputCount = 0;
    read_entries(outputFileName, &entries, totalCount, &totalOutputCount);
    printToFile(entries, totalOutputCount, outputFileName, NEW);
    printToStdOut(entries, totalOutputCount);
    printFileStatistics(fileStats);

    free_directories(&directories);
    free_directories(&tmpFileNames);

    return EXIT_SUCCESS;
}
