#include "shared.h"
#include "lconsts.h"

const char *_FILE_TYPES[] = {
    "T_DIR", "T_TEXT", "T_BINARY", "T_IMAGE", "T_JSON", "T_AUDIO", "T_FILM",
    "T_COMPRESSED", "T_YAML", "T_EXE", "T_C", "T_PYTHON", "T_JS",
    "T_JAVA", "T_LOG", "T_PACKAGE", "T_CLASS", "T_TEMPLATE", "T_PHP", "T_MATHEMATICA",
    "T_PDF", "T_JAR", "T_HTML", "T_XML", "T_XHTML", "T_MATLAB", "T_FORTRAN", "T_SCIENCE", "T_CPP",
    "T_TS", "T_DOC", "T_CALC", "T_LATEX", "T_SQL", "T_PRESENTATION", "T_DATA", "T_LIBRARY", "T_OBJECT",
    "T_CSV", "T_CSS", "T_LINK_DIR", "T_LINK_FILE"
};

/**
 * Helper function to check if a string is a valid size_t (non-negative integer).
 * @param str: A string to check.
 * @return 1 if valid, 0 otherwise.
 */
int is_size_t(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0; // Non-digit character found
        }
        str++;
    }
    return 1; // Entire string is numeric
}

// Function to check if a value belongs to _FILE_TYPES
bool is_valid_file_type(const char *type) {
    for (int i = 0; i < FILE_TYPES_COUNT; i++) {
        if (strcmp(_FILE_TYPES[i], type) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Process a file line by line, ensuring each row contains between 3 and 6 columns
 * and that the second column is of type size_t.
 * @param filename: The path to the input file.
 * @return 0 on success, non-zero on error.
 */
int process_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;

    // Process each line of the file
    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        // Remove trailing newline character, if present
        line[strcspn(line, "\n")] = '\0';

        // Make a copy of the line since `strtok` modifies the input
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, line, sizeof(lineCopy));
        lineCopy[sizeof(lineCopy) - 1] = '\0';

        // Split the line into columns using '|'
        int columnCount = 0;
        char *columns[7] = {NULL}; // To store up to 6 columns
        char *token = strtok(line, SEP);

        while (token != NULL && columnCount < 7) {
            columns[columnCount++] = token;
            token = strtok(NULL, SEP);
        }

        // Check the number of columns (between 3 and 6)
        if (columnCount < 3 || columnCount > 7) {
            fprintf(stderr, "Error: Invalid row at line %d. Expected 3-6 columns, got %d. Row content: \"%s\"\n",
                    lineNumber, columnCount, lineCopy);
            exit(EXIT_FAILURE);
        }

        // Validate the second column as size_t
        if (!is_size_t(columns[1])) {
            fprintf(stderr, "Error: Invalid size_t value in the second column at line %d. Row content: \"%s\"\n",
                    lineNumber, lineCopy);
            continue; // Go to the next line
        }

        // Validate the 3rd column against _FILE_TYPES
        if (!is_valid_file_type(columns[2])) {
            fprintf(stderr, "Error: Invalid file type in the third column at line %d. Row content: \"%s\"\n",
                    lineNumber, lineCopy);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    return EXIT_SUCCESS;
}

/**
 * Prints the elapsed time for processing a specific directory or task, formatted to an appropriate
 * precision based on the duration.
 *
 * This function outputs a message indicating the time taken to process a given directory or task.
 * The precision of the displayed time adjusts dynamically: 2 decimal places for times under 1 second,
 * 1 decimal place for times between 1 and 10 seconds, and no decimal places for times of 10 seconds or more.
 * The message is written to a specified file stream.
 *
 * @param directory  The name of the directory or description of the task being processed.
 * @param elapsed    The time taken, in seconds, to process the directory or task.
 * @param output     A pointer to the output file stream where the message will be printed.
 * @param message    A descriptive message about the operation being timed, e.g., "directory".
 */
void print_elapsed_time(const char *directory, const double elapsed, FILE *output, const char *message) {
    if (directory) {
        // If directory is not NULL, include it in the output
        fprintf(output, "Time taken to process %s '%s': %.*f seconds\n",
                message, directory, (elapsed < 1) ? 2 : (elapsed < 10) ? 1 : 0, elapsed);
    } else {
        // If directory is NULL, omit the '%s'
        fprintf(output, "Time taken to process %s: %.*f seconds\n",
                message, (elapsed < 1) ? 2 : (elapsed < 10) ? 1 : 0, elapsed);
    }
}

double get_time_difference(const struct timeval start, const struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

char *getFileSizeAsString(const long long fileSizeBytesIn) {
    const double fileSizeBytes = (double) fileSizeBytesIn;
    const double kB = 1024.0;
    const double MB = 1024.0 * 1024.0;
    const double GB = 1024.0 * 1024.0 * 1024.0;
    char *result = malloc(20 * sizeof(char)); // Allocate memory for the result

    if (fileSizeBytes >= GB) {
        snprintf(result, 20, "%.2f GB", fileSizeBytes / GB);
    } else if (fileSizeBytes >= MB) {
        snprintf(result, 20, "%.2f MB", fileSizeBytes / MB);
    } else if (fileSizeBytes >= kB) {
        snprintf(result, 20, "%.2f kB", fileSizeBytes / kB);
    } else {
        snprintf(result, 20, "%.0f bytes", fileSizeBytes);
    }

    return result;
}

/**
 * Extracts unique root tokens from a delimiter-separated text file and stores them in a dynamically
 * allocated array while updating the root count.
 *
 * This function reads a text file line by line, extracts the first token from each line using the
 * '|' delimiter, and adds unique tokens to the provided root array. It skips consecutive lines
 * starting with the same token and resizes the root array as needed to accommodate more tokens.
 * Memory for added tokens is dynamically allocated, and the caller is responsible for freeing it.
 *
 * @param fileName  The name of the input file to process. Each line of the file should conform to
 *                  the expected format with tokens separated by '|'.
 * @param root      A double pointer to an array of strings where the unique root tokens will be
 *                  stored. This array will be dynamically resized during execution.
 * @param count     A pointer to an integer representing the number of elements currently in the
 *                  root array. It will be updated as new tokens are added.
 */
void get_dir_root(const char *fileName, char ***root, int *count) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE_LENGTH]; // Buffer to hold a line from the file
    int capacity = MAX_LINE_LENGTH; // Initial size for the root array
    char *previousToken = NULL; // To keep track of the previous token

    // Read the first line
    if (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Extract the first token from the first line
        const char *firstToken = strtok(buffer, SEP);
        if (firstToken == NULL) {
            fprintf(stderr, "Invalid file format: no valid token in the first line.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Store the first token into the root array
        previousToken = strdup(firstToken);
        if (previousToken == NULL) {
            perror("Error duplicating token");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        (*root)[*count] = malloc(sizeof(char) * strlen(previousToken) + 1); // Example: allocate memory for a string
        snprintf((*root)[*count], strlen(previousToken) + 1, "%s", previousToken);

        // (*root)[*count] = previousToken;
        (*count)++;
    }

    // Process the rest of the lines
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Extract the first token from the current line
        const char *lineToken = strtok(buffer, SEP);
        if (lineToken == NULL) {
            continue; // Skip lines that do not contain a valid token
        }

        // If the current line starts with the previous token, skip it
        if (strstr(lineToken, previousToken) != NULL) {
            continue;
        }

        // Add the token to the root array
        char *newToken = strdup(lineToken);
        if (newToken == NULL) {
            perror("Error duplicating token");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Expand the root array if necessary
        if (*count >= capacity) {
            capacity *= 2;
            char **newRoot = realloc(*root, capacity * sizeof(char *));
            if (newRoot == NULL) {
                perror("Error reallocating memory for root array");
                fclose(file);
                exit(EXIT_FAILURE);
            }
            *root = newRoot;
        }

        (*root)[*count] = malloc(sizeof(char) * strlen(newToken) + 1); // Example: allocate memory for a string
        snprintf((*root)[*count], strlen(newToken) + 1, "%s", newToken);

        // (*root)[*count] = newToken;
        (*count)++;

        // Update the `previousToken`
        previousToken = newToken;
    }
    (*root)[*count] = NULL; // Mark the end of the array
    fclose(file);
}

/**
 * Function to validate and process a list of input file names.
 * This function verifies that the input files exist, are regular files,
 * and processes them if valid. It also manages memory allocation or reallocation
 * for storing root directories.
 *
 * @param inputFileNames: A pointer to an array of null-terminated strings representing input file names.
 * @param rootDirectories: A pointer to an array of null-terminated strings that store root directory paths.
 *                         May be reallocated based on the number of input files processed.
 * @param rootCount: A pointer to an integer that stores the total count of unique root directories.
 *                   Updated based on the processed input files.
 *
 * The function terminates the program with an appropriate error message if memory allocation,
 * file access, or processing fails.
 */
void check_input_files(char **inputFileNames, char ***rootDirectories, int *rootCount) {
    if (inputFileNames == NULL) {
        fprintf(stderr, "Error: inputFileNames is NULL.\n");
        exit(EXIT_FAILURE);
    }
    if (*rootDirectories == NULL) {
        *rootDirectories = realloc(*rootDirectories, sizeof(char *) * (MAX_LINE_LENGTH + 2));
        if (*rootDirectories == NULL) {
            perror("Error allocating memory for rootDirectories");
            exit(EXIT_FAILURE);
        }
    } else {
        *rootDirectories = realloc(*rootDirectories, sizeof(char *) * (MAX_LINE_LENGTH + 2)); // Reallocate memory
        if (*rootDirectories == NULL) {
            perror("Error reallocating memory for rootDirectories");
            exit(EXIT_FAILURE);
        }
    }
    int tmp = 0;

    // *tmpFileNames = malloc(MAX_LINE_LENGTH * sizeof(char *));
    if (*rootDirectories == NULL) {
        perror("Error allocating memory for rootDirectories in check_merge_files");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; inputFileNames[i] != NULL; i++) {
        get_dir_root(inputFileNames[i], rootDirectories, &tmp);
        const char *fileName = inputFileNames[i];
        fprintf(stdout, "Checking input file format: %s\n", fileName);
        // Check if the file is a regular file
        struct stat fileStat;
        if (stat(fileName, &fileStat) != 0) {
            perror("Error accessing file");
            fprintf(stderr, "Error: Cannot access file '%s'.\n", fileName);
            continue;
        }
        if (!S_ISREG(fileStat.st_mode)) {
            fprintf(stderr, "Error: Merge file '%s' is not a regular file.\n", fileName);
            exit(EXIT_FAILURE);
        }
        // Check and process the file using process_file
        if (process_file(fileName) != EXIT_SUCCESS) {
            fprintf(stderr, "Error: Failed to process file '%s'.\n", fileName);
        }
        if (*rootCount == -1) {
            break;
        }
    }
    *rootCount = tmp;
}