#include "flib/lfiles.h"
/**
 * @brief Entry point of the application to analyze and process files and directories.
 *
 * This function processes command-line arguments to configure various operations,
 * such as skipping directories, setting a size threshold for files, and specifying
 * an output file. It utilizes multi-threading for efficiency and processes a directory
 * structure to gather statistics, classify files by type, and generate a detailed
 * output in the specified file.
 *
 * @param argc Number of command-line arguments, including the program name.
 * @param argv Array of command-line arguments.
 *
 * The following parameters are supported:
 * - argv[1]: Directory path to process (mandatory).
 * - `--skip-dirs`: Optional flag to skip directories while processing files.
 * - `-M <maxSizeInMB>`: Optional argument to set a size threshold (in MB) for files.
 * - `-o <outputfile>`: Mandatory argument specifying the path to the output file.
 *
 * @return EXIT_SUCCESS on successful completion, or EXIT_FAILURE in case of an error.
 *
 * Functionality details:
 * - Parses command-line arguments to configure behavior.
 * - Initializes required resources, such as file pointers and task queues.
 * - Enqueues the specified directory path for processing.
 * - Allocates memory to store file entries and other related data structures.
 * - Uses multi-threading with a maximum number of threads available.
 * - Processes files and directories, gathering statistics:
 *   - Counts total files, directories, and hidden files.
 *   - Calculates total sizes and classifies file types into categories
 *     (e.g., text, music, film, image, binary, etc.).
 * - Generates an output file with file details and - if required -
 *   uses temporary files for sorting intermediate results.
 * - Produces a human-readable summary of the file statistics, displayed on the console.
 *
 * Important considerations:
 * - The `-o` option is mandatory to specify the output file.
 * - Memory allocation failures and file operation errors are handled with error messages
 *   and proper cleanup.
 * - Hidden files (file names starting with a '.') are flagged accordingly in the output.
 */
int main(int argc, char *argv[]) {
    long long sizeThreshold = 0;
    int skipDirs = 0;
    char *originalFileName = NULL;
    char *tmpFileName = NULL;
    char *tmpFileNameSrt = NULL;

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
                release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                originalFileName = argv[i];
                tmpFileName = malloc(strlen(originalFileName) + 5); //
                if (tmpFileName == NULL) {
                    perror("Memory allocation failed (tmpFileName)");
                    exit(1);
                }
                tmpFileNameSrt = malloc(strlen(originalFileName) + 9); //
                strcpy(tmpFileName, originalFileName);
                strcpy(tmpFileNameSrt, tmpFileName);
                strcat(tmpFileName, "tmp");    // Appends "tmp" to tmpFileName
                strcat(tmpFileNameSrt, "srt"); // Appends "srt" to tmpFileNameSrt
            } else {
                fprintf(stderr, "Output file name expected after -o\n");
                release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
                return EXIT_FAILURE;
            }
        }
    }

    // Ensure -o parameter is used
    if (!originalFileName) {
        fprintf(stderr, "Error: The -o <outputfile> option is required.\n");
        fprintf(stderr, "Usage: %s <directory_path> [-M maxSizeInMB] [--skip-dirs] -o outputfile\n", argv[0]);
        release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
        return EXIT_FAILURE;
    }

    int numCores = omp_get_max_threads();
    omp_set_num_threads(numCores);
    fprintf(stdout, "Using %d cores.\n", numCores);

    TaskQueue taskQueue;
    initQueue(&taskQueue, INITIAL_CAPACITY);
    enqueue(&taskQueue, argv[1]);

    int capacity = INITIAL_CAPACITY;
    int count = 0;
    FileEntry *entries = malloc(capacity * sizeof(FileEntry));
    if (!entries) {
        perror("malloc");
        release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
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
    printf("Read %d entries \n", count);
    if (count < INITIAL_ENTRIES_CAPACITY) {
        qsort(entries, count, sizeof(FileEntry), compareFileEntries);
    }
    else {
        char command[MAX_LINE_LENGTH];
        printToFile(entries, count, tmpFileName);
        snprintf(command, sizeof(command), "sort --parallel=12 -t \"|\" -k1,1 %s -o %s", tmpFileName, tmpFileNameSrt);
        int ret = system(command);
        if (ret == -1) {
            perror("system() failed");
            release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
            return EXIT_FAILURE;
        }
        if (WEXITSTATUS(ret) != 0) {
            fprintf(stderr, "Command exited with non-zero status: %d\n", WEXITSTATUS(ret));
            release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);
            return EXIT_FAILURE;
        }
        // read_entries(tmpFileNameSrt, &entries, count);
    }
    printToStdOut(entries, count);
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

    printf("#####################\n");
    printf("Total Number of Directories: %d\n", totalDirs);
    printf("Total Number of Files: %d\n", totalFiles);
    printf("Total Size of Files: %lld bytes (%s)\n", totalSize, getFileSizeAsString(totalSize));

    free(entries);
    freeQueue(&taskQueue);
    release_temporary_resources(tmpFileName, tmpFileNameSrt, NULL);

    return EXIT_SUCCESS;
}
