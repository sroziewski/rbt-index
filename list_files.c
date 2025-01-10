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
int main(int argc, char *argv[]) {
    long long sizeThreshold = 0;
    int skipDirs = 0;
    char *outputFileName = NULL;
    char *tmpFileName = NULL;

    // Array for storing directory paths
    char **directories = NULL;
    int directoryCount = 0;

    if (process_arguments(argc, argv, &skipDirs, &sizeThreshold, &outputFileName, &tmpFileName, &directories,
                          &directoryCount) != EXIT_SUCCESS) {
        printf("Error processing arguments\n");
        return EXIT_FAILURE;
    }
    // Process directories
    if (directories != NULL) {
        printf("Processing %s:\n", directoryCount > 1 ? "directories" : "directory");
        for (int i = 0; directories[i] != NULL; i++) {
            printf("- %s\n", directories[i]);
        }
    }
    int numCores = omp_get_max_threads();
    omp_set_num_threads(numCores);
    fprintf(stdout, "Using %d cores.\n", numCores);

    TaskQueue taskQueue;
    initQueue(&taskQueue, INITIAL_CAPACITY);
    enqueue(&taskQueue, removeTrailingSlash(argv[1]));

    int capacity = INITIAL_CAPACITY;
    int count = 0;
    FileEntry *entries = malloc(capacity * sizeof(FileEntry));
    if (!entries) {
        perror("malloc");
        release_temporary_resources(&tmpFileName, directories, NULL);
        return EXIT_FAILURE;
    }

    FileStatistics fileStats;
    initializeFileStatistics(&fileStats); // Initialize the structure

    processDirectory(&taskQueue, &entries, &count, &capacity, &fileStats, sizeThreshold, skipDirs);
    fileStats.totalDirs--; // Exclude the root directory

    printf("### Total counts before qsort %d ###", count);
    if (count < INITIAL_ENTRIES_CAPACITY) {
        printf("Less than %d entries, sorting in-memory\n", INITIAL_ENTRIES_CAPACITY);
        qsort(entries, count, sizeof(FileEntry), compareFileEntries);
        printToFile(entries, count, outputFileName);
    } else {
        printf("More than %d entries, sorting to temporary file\n", INITIAL_ENTRIES_CAPACITY);
        char command[MAX_LINE_LENGTH];
        printToFile(entries, count, tmpFileName);
        snprintf(command, sizeof(command), "sort --parallel=12 -t \"|\" -k1,1 %s -o %s", tmpFileName, outputFileName);
        int ret = system(command);
        deleteFile(tmpFileName);
        if (ret == -1) {
            perror("system() failed");
            release_temporary_resources(&tmpFileName, NULL);
            free_directories(&directories);
            return EXIT_FAILURE;
        }
        if (WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Command exited with non-zero status: %d\n", WEXITSTATUS(ret));
            release_temporary_resources(&tmpFileName, NULL);
            free_directories(&directories);
            return EXIT_FAILURE;
        }
    }
    int outputCount = 0;
    read_entries(outputFileName, &entries, count, &outputCount);
    resizeEntries(&entries, &count); // Resize the array to the actual number of entries
    accumulateChildrenAndSize(entries, count);

    printf("### Total counts after accumulateChildrenAndSize %d ###", count);

    printToFile(entries, count, outputFileName);

    printf("\nSummary:\n");
    if (fileStats.hiddenFiles > 0) {
        printf("Total Number of Hidden Files: %d\n", fileStats.hiddenFiles);
        printf("Total Size of Hidden Files: %s (%lld bytes) \n", getFileSizeAsString(fileStats.hiddenSize),
               fileStats.hiddenSize);
    }
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
    printSizeDetails("Template", fileStats.templateFiles, fileStats.templateSize);
    printSizeDetails("Pdf", fileStats.pdfFiles, fileStats.pdfSize);
    printSizeDetails("Doc", fileStats.docFiles, fileStats.docSize);
    printSizeDetails("LaTex", fileStats.texFiles, fileStats.texSize);
    printSizeDetails("Calc", fileStats.calcFiles, fileStats.calcSize);
    printSizeDetails("Jar", fileStats.jarFiles, fileStats.jarSize);
    printSizeDetails("C Source", fileStats.cFiles, fileStats.cSize);
    printSizeDetails("EXE", fileStats.exeFiles, fileStats.exeSize);

    printf("------------------------------------\n");
    printf("Total Number of Directories: %d\n", fileStats.totalDirs);
    printf("Total Number of Files: %d\n", fileStats.totalFiles);
    printf("Total Size of Files: %lld bytes (%s)\n", fileStats.totalSize, getFileSizeAsString(fileStats.totalSize));

    free(entries);
    freeQueue(&taskQueue);

    release_temporary_resources(&tmpFileName, NULL);
    free_directories(&directories);

    return EXIT_SUCCESS;
}
