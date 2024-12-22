#include "lfiles.h"

int main(int argc, char *argv[]) {
    long long sizeThreshold = 0;
    int skipDirs = 0;
    FILE *outputFile = NULL;  // Initially set to NULL since -o is mandatory

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--skip-dirs") == 0) {
            skipDirs = 1;  // Enable directory skipping
        } else if (strcmp(argv[i], "-M") == 0) {
            if (i + 1 < argc && isdigit(argv[i + 1][0])) {
                char *endptr;
                double sizeInMB = strtod(argv[++i], &endptr);
                sizeThreshold = (long long)(sizeInMB * 1024 * 1024);
            } else {
                fprintf(stderr, "Invalid or missing size argument after -M\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputFile = fopen(argv[++i], "w");
                if (!outputFile) {
                    perror("Could not open output file");
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Output file name expected after -o\n");
                return EXIT_FAILURE;
            }
        }
    }

    // Ensure -o parameter is used
    if (!outputFile) {
        fprintf(stderr, "Error: The -o <outputfile> option is required.\n");
        fprintf(stderr, "Usage: %s <directory_path> -M maxSizeInMB [--skip-dirs] -o outputfile\n", argv[0]);
        return EXIT_FAILURE;
    }

    int numCores = omp_get_max_threads();
    omp_set_num_threads(numCores);
    fprintf(stderr, "Using %d cores.\n", numCores);

    TaskQueue taskQueue;
    initQueue(&taskQueue, INITIAL_CAPACITY);
    enqueue(&taskQueue, argv[1]);

    int capacity = INITIAL_CAPACITY;
    int count = 0;
    FileEntry *entries = (FileEntry *)malloc(capacity * sizeof(FileEntry));
    if (!entries) {
        perror("malloc");
        if (outputFile != stdout) fclose(outputFile);
        return EXIT_FAILURE;
    }

    long long totalSize = 0;
    int totalFiles = 0;
    int totalDirs = 0;
    int textFiles = 0, musicFiles = 0, filmFiles = 0, imageFiles = 0, binaryFiles = 0, compressedFiles = 0, texFiles = 0;
    int jsonFiles = 0, yamlFiles = 0, exeFiles = 0, templateFiles = 0, pdfFiles = 0, jarFiles = 0, htmlFiles = 0, xmlFiles = 0, tsFiles = 0, jsFiles = 0, xhtmlFiles = 0;
    int cFiles = 0, pythonFiles = 0, javaFiles = 0, packageFiles = 0, logFiles = 0, classFiles = 0, docFiles = 0, calcFiles = 0, sqlFiles = 0;

    int hiddenFiles = 0;

    long long textSize = 0, musicSize = 0, filmSize = 0, imageSize = 0, binarySize = 0, compressedSize = 0, texSize = 0;
    long long jsonSize = 0, yamlSize = 0, exeSize = 0, classSize = 0, templateSize = 0, pdfSize = 0, jarSize = 0, docSize = 0, calcSize = 0;
    long long cSize = 0, pythonSize = 0, javaSize = 0, packageSize = 0, logSize = 0, htmlSize = 0, xmlSize = 0, tsSize = 0, jsSize = 0, xhtmlSize = 0, sqlSize = 0;
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
                     &texFiles, &texSize, &sqlFiles, &sqlSize,
                     skipDirs, sizeThreshold);

    qsort(entries, count, sizeof(FileEntry), compareFileEntries);

    for (int i = 0; i < count; i++) {
        const char* fileName = getFileName(entries[i].path);
        int isHidden = (fileName[0] == '.');
        fprintf(outputFile, "%s<SEP>%ld<SEP>%s",
                entries[i].path, entries[i].size, entries[i].type);
	printf("File: %s, Size: %s (%ld bytes), Type: %s",
                entries[i].path, getFileSizeAsString(entries[i].size), entries[i].size, entries[i].type);
        if (isHidden) {
            fprintf(outputFile, ", F_HIDDEN");
            printf(", F_HIDDEN");
        }
        fprintf(outputFile, "\n");
        printf("\n");
    }

    printf("\nSummary:\n");
    printf("Total Number of Directories: %d\n", totalDirs);
    printf("Total Number of Files: %d\n", totalFiles);
    printf("Total Size of Files: %lld bytes (%s)\n", totalSize, getFileSizeAsString(totalSize));
    if (hiddenFiles > 0) {
        printf("Total Number of Hidden Files: %d\n", hiddenFiles);
        printf("Total Size of Hidden Files: %s (%lld bytes) \n", getFileSizeAsString(hiddenSize), hiddenSize);
    }

    printSizeDetails(outputFile, "Text", textFiles, textSize);
    printSizeDetails(outputFile, "Music", musicFiles, musicSize);
    printSizeDetails(outputFile, "Film", filmFiles, filmSize);
    printSizeDetails(outputFile, "Image", imageFiles, imageSize);
    printSizeDetails(outputFile, "Compressed", compressedFiles, compressedSize);
    printSizeDetails(outputFile, "Binary", binaryFiles, binarySize);
    printSizeDetails(outputFile, "JSON", jsonFiles, jsonSize);
    printSizeDetails(outputFile, "YAML", yamlFiles, yamlSize);
    printSizeDetails(outputFile, "Python", pythonFiles, pythonSize);
    printSizeDetails(outputFile, "Java", javaFiles, javaSize);
    printSizeDetails(outputFile, "Ts", tsFiles, tsSize);
    printSizeDetails(outputFile, "Js", jsFiles, jsSize);
    printSizeDetails(outputFile, "Sql", sqlFiles, sqlSize);
    printSizeDetails(outputFile, "Html", htmlFiles, htmlSize);
    printSizeDetails(outputFile, "Xhtml", xhtmlFiles, xhtmlSize);
    printSizeDetails(outputFile, "Xml", xmlFiles, xmlSize);
    printSizeDetails(outputFile, "Packages", packageFiles, packageSize);
    printSizeDetails(outputFile, "Log", logFiles, logSize);
    printSizeDetails(outputFile, "Class", classFiles, classSize);
    printSizeDetails(outputFile, "Template", templateFiles, templateSize);
    printSizeDetails(outputFile, "Pdf", pdfFiles, pdfSize);
    printSizeDetails(outputFile, "Doc", docFiles, docSize);
    printSizeDetails(outputFile, "LaTex", texFiles, texSize);
    printSizeDetails(outputFile, "Calc", calcFiles, calcSize);
    printSizeDetails(outputFile, "Jar", jarFiles, jarSize);
    printSizeDetails(outputFile, "C Source", cFiles, cSize);
    printSizeDetails(outputFile, "EXE", exeFiles, exeSize);

    free(entries);
    freeQueue(&taskQueue);
    if (outputFile != stdout) fclose(outputFile);

    return EXIT_SUCCESS;
}