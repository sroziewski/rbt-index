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
    char *originalFileName = NULL;
    char *tmpFileName = NULL;

    // Array for storing directory paths
    char **directories = NULL;
    int directoryCount = 0;

    if (process_arguments(argc, argv, &skipDirs, &sizeThreshold, &originalFileName, &tmpFileName, &directories, &directoryCount) != EXIT_SUCCESS) {
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

    long long totalSize = 0;
    int totalFiles = 0;
    int totalDirs = 0;
    int textFiles = 0, musicFiles = 0, filmFiles = 0, imageFiles = 0, binaryFiles = 0, compressedFiles = 0, texFiles =
            0;
    int jsonFiles = 0, yamlFiles = 0, exeFiles = 0, templateFiles = 0, pdfFiles = 0, jarFiles = 0, htmlFiles = 0,
            xmlFiles = 0, tsFiles = 0, jsFiles = 0, xhtmlFiles = 0;
    int cFiles = 0, pythonFiles = 0, javaFiles = 0, packageFiles = 0, logFiles = 0, classFiles = 0, docFiles = 0,
            calcFiles = 0, sqlFiles = 0, csvFiles = 0, cssFiles = 0;

    int hiddenFiles = 0;

    long long textSize = 0, musicSize = 0, filmSize = 0, imageSize = 0, binarySize = 0, compressedSize = 0, texSize = 0;
    long long jsonSize = 0, yamlSize = 0, exeSize = 0, classSize = 0, templateSize = 0, pdfSize = 0, jarSize = 0,
            docSize = 0, calcSize = 0;
    long long cSize = 0, pythonSize = 0, javaSize = 0, packageSize = 0, logSize = 0, htmlSize = 0, xmlSize = 0, tsSize =
            0, jsSize = 0, xhtmlSize = 0, sqlSize = 0, csvSize = 0, cssSize = 0;
    long long hiddenSize = 0;

    processDirectory(&taskQueue, &entries, &count, &capacity, &totalSize, &totalFiles, &totalDirs,
                     &textFiles, &textSize, &musicFiles, &musicSize,
                     &filmFiles, &filmSize, &imageFiles, &imageSize,
                     &binaryFiles, &binarySize, &compressedFiles, &compressedSize,
                     &hiddenFiles, &hiddenSize, &jsonFiles, &jsonSize,
                     &yamlFiles, &yamlSize, &exeFiles, &exeSize,
                     &cFiles, &cSize, &pythonFiles, &pythonSize,
                     &javaFiles, &javaSize, &packageFiles, &packageSize,
                     &logFiles, &logSize, &classFiles, &classSize,
                     &templateFiles, &templateSize, &pdfFiles, &pdfSize,
                     &jarFiles, &jarSize, &htmlFiles, &htmlSize,
                     &xhtmlFiles, &xhtmlSize, &xmlFiles, &xmlSize,
                     &tsFiles, &tsSize, &jsFiles, &jsSize,
                     &docFiles, &docSize, &calcFiles, &calcSize,
                     &texFiles, &texSize, &sqlFiles, &sqlSize, &csvFiles, &csvSize, &cssFiles, &cssSize,
                     skipDirs, sizeThreshold);
    totalDirs--; // Exclude the root directory
    printf("### Total counts before qsort %d ###", count);
    if (count < INITIAL_ENTRIES_CAPACITY) {
        printf("Less than %d entries, sorting in-memory\n", INITIAL_ENTRIES_CAPACITY);
        qsort(entries, count, sizeof(FileEntry), compareFileEntries);
        printToFile(entries, count, originalFileName);
    }
    else {
        printf("More than %d entries, sorting to temporary file\n", INITIAL_ENTRIES_CAPACITY);
        char command[MAX_LINE_LENGTH];
        printToFile(entries, count, tmpFileName);
        snprintf(command, sizeof(command), "sort --parallel=12 -t \"|\" -k1,1 %s -o %s", tmpFileName, originalFileName);
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
    read_entries(originalFileName, &entries, count, &outputCount);
    resizeEntries(&entries, &count); // Resize the array to the actual number of entries
    accumulateChildrenAndSize(entries, count);

    printf("### Total counts after accumulateChildrenAndSize %d ###", count);

    printToFile(entries, count, originalFileName);

    printf("\nSummary:\n");
    if (hiddenFiles > 0) {
        printf("Total Number of Hidden Files: %d\n", hiddenFiles);
        printf("Total Size of Hidden Files: %s (%lld bytes) \n", getFileSizeAsString(hiddenSize), hiddenSize);
    }
    printSizeDetails("Text", textFiles, textSize);
    printSizeDetails("Music", musicFiles, musicSize);
    printSizeDetails("Film", filmFiles, filmSize);
    printSizeDetails("Image", imageFiles, imageSize);
    printSizeDetails("Compressed", compressedFiles, compressedSize);
    printSizeDetails("Binary", binaryFiles, binarySize);
    printSizeDetails("JSON", jsonFiles, jsonSize);
    printSizeDetails("Csv", csvFiles, csvSize);
    printSizeDetails("YAML", yamlFiles, yamlSize);
    printSizeDetails("Python", pythonFiles, pythonSize);
    printSizeDetails("Java", javaFiles, javaSize);
    printSizeDetails("Ts", tsFiles, tsSize);
    printSizeDetails("Js", jsFiles, jsSize);
    printSizeDetails("Sql", sqlFiles, sqlSize);
    printSizeDetails("Html", htmlFiles, htmlSize);
    printSizeDetails("Css", cssFiles, cssSize);
    printSizeDetails("Xhtml", xhtmlFiles, xhtmlSize);
    printSizeDetails("Xml", xmlFiles, xmlSize);
    printSizeDetails("Packages", packageFiles, packageSize);
    printSizeDetails("Log", logFiles, logSize);
    printSizeDetails("Class", classFiles, classSize);
    printSizeDetails("Template", templateFiles, templateSize);
    printSizeDetails("Pdf", pdfFiles, pdfSize);
    printSizeDetails("Doc", docFiles, docSize);
    printSizeDetails("LaTex", texFiles, texSize);
    printSizeDetails("Calc", calcFiles, calcSize);
    printSizeDetails("Jar", jarFiles, jarSize);
    printSizeDetails("C Source", cFiles, cSize);
    printSizeDetails("EXE", exeFiles, exeSize);

    printf("------------------------------------\n");
    printf("Total Number of Directories: %d\n", totalDirs);
    printf("Total Number of Files: %d\n", totalFiles);
    printf("Total Size of Files: %lld bytes (%s)\n", totalSize, getFileSizeAsString(totalSize));

    free(entries);
    freeQueue(&taskQueue);

    release_temporary_resources(&tmpFileName, NULL);
    free_directories(&directories);

    return EXIT_SUCCESS;
}
