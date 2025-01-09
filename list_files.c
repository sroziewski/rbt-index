#include "flib/lfiles.h"
/**
 * The main entry point for the program. This function processes files and directories
 * specified via command-line arguments, calculates file statistics, and produces an
 * output file summarizing the results. It may also skip certain directories,
 * limit processing to files below a given size threshold, or use multithreading for
 * performance optimization.
 *
 * @param argc The number of command-line arguments passed to the program.
 * @param argv The array of command-line arguments. The following options are supported:
 *      - `--skip-dirs`: Skips processing of directories.
 *      - `-M <size>`: Sets a size threshold (in MB) for processing files.
 *      - `-o <outputfile>`: Specifies the output file name. This option is mandatory.
 *
 * @return Returns `EXIT_SUCCESS` (typically 0) on successful execution.
 *         Returns `EXIT_FAILURE` (typically 1) if an error occurs, such as invalid input
 *         arguments, memory allocation issues, or external command failures.
 */
int main(int argc, char *argv[]) {
    long long sizeThreshold = 0;
    int skipDirs = 0;
    char *originalFileName = NULL;
    char *tmpFileName = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--skip-dirs") == 0) {
            skipDirs = 1; // Enable directory skipping
        } else if (strcmp(argv[i], "-M") == 0) {
            if (i + 1 < argc && isdigit(argv[i + 1][0])) {
                char *endptr;
                double sizeInMB = strtod(argv[++i], &endptr);
                sizeThreshold = (long long) (sizeInMB * 1024 * 1024);
            } else {
                fprintf(stderr, "Invalid or missing size argument after -M\n");
                release_temporary_resources(tmpFileName, originalFileName, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                originalFileName = argv[++i];
                tmpFileName = malloc(strlen(originalFileName) + 5); //
                if (tmpFileName == NULL) {
                    perror("Memory allocation failed (tmpFileName)");
                    exit(1);
                }
                strcpy(tmpFileName, originalFileName);
                strcat(tmpFileName, "tmp");    // Appends "tmp" to tmpFileName
            } else {
                fprintf(stderr, "Output file name expected after -o\n");
                release_temporary_resources(tmpFileName, originalFileName, NULL);
                return EXIT_FAILURE;
            }
        }
    }

    // Ensure -o parameter is used
    if (!originalFileName) {
        fprintf(stderr, "Error: The -o <outputfile> option is required.\n");
        fprintf(stderr, "Usage: %s <directory_path> [-M maxSizeInMB] [--skip-dirs] -o outputfile\n", argv[0]);
        release_temporary_resources(tmpFileName, originalFileName, NULL);
        return EXIT_FAILURE;
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
        release_temporary_resources(tmpFileName, originalFileName, NULL);
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
    if (count < INITIAL_ENTRIES_CAPACITY) {
        qsort(entries, count, sizeof(FileEntry), compareFileEntries);
        printToFile(entries, count, originalFileName);
        printToStdOut(entries, count);
    }
    else {
        char command[MAX_LINE_LENGTH];
        printToFile(entries, count, tmpFileName);
        snprintf(command, sizeof(command), "sort --parallel=12 -t \"|\" -k1,1 %s -o %s", tmpFileName, originalFileName);
        int ret = system(command);
        deleteFile(tmpFileName);
        if (ret == -1) {
            perror("system() failed");
            release_temporary_resources(tmpFileName, originalFileName, NULL);
            return EXIT_FAILURE;
        }
        if (WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Command exited with non-zero status: %d\n", WEXITSTATUS(ret));
            release_temporary_resources(tmpFileName, originalFileName, NULL);
            return EXIT_FAILURE;
        }
        read_entries(originalFileName, &entries, count);
    }
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

    return EXIT_SUCCESS;
}
