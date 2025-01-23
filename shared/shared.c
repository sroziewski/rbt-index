#include "shared.h"

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

// Function to check if a value belongs to FILE_TYPES
bool is_valid_file_type(const char *type) {
    for (int i = 0; i < FILE_TYPES_COUNT; i++) {
        if (strcmp(FILE_TYPES[i], type) == 0) {
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

        // Validate the 3rd column against FILE_TYPES
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
 * Verifies, processes, and validates a set of merge files while managing a temporary filename list and root count.
 *
 * This function iterates through an array of provided merge file names, validates their existence and format,
 * and processes their contents to extract directory roots. It ensures that `tmpFileNames` is dynamically allocated
 * or resized as necessary and tracks the number of root elements identified. Each merge file is checked to confirm
 * it is a regular file before processing. Errors during memory allocation, file access, or invalid merge file formats
 * result in error messages being displayed and appropriate action taken (e.g., program termination).
 *
 * - Dynamically manages memory for the `tmpFileNames` array to store extracted roots.
 * - For each file, calls `get_dir_root` to extract and update directory roots.
 * - Ensures each file in `mergeFileNames` exists and is a regular file.
 * - Uses `process_file` to handle file-specific processing for valid files.
 *
 * @param mergeFileNames An array of strings representing paths to merge files to be processed.
 *                       The array must be terminated with a NULL pointer.
 * @param tmpFileNames   A pointer to a dynamic array of strings that will store temporary filenames used during processing.
 *                       If `*tmpFileNames` is NULL, the function allocates memory for the array.
 * @param rootCount      A pointer to an integer used to count the number of root directory entries extracted
 *                       from the processed merge files.
 */
void check_input_files(char **mergeFileNames, char ***tmpFileNames, int *rootCount) {
    if (mergeFileNames == NULL) {
        fprintf(stderr, "Error: mergeFileNames is NULL.\n");
        exit(EXIT_FAILURE);
    }
    if (*tmpFileNames == NULL) {
        *tmpFileNames = realloc(*tmpFileNames, sizeof(char *) * (MAX_LINE_LENGTH + 2));
        if (*tmpFileNames == NULL) {
            perror("Error allocating memory for tmpFileNames");
            exit(EXIT_FAILURE);
        }
    } else {
        *tmpFileNames = realloc(*tmpFileNames, sizeof(char *) * (MAX_LINE_LENGTH + 2)); // Reallocate memory
        if (*tmpFileNames == NULL) {
            perror("Error reallocating memory for tmpFileNames");
            exit(EXIT_FAILURE);
        }
    }
    int tmp = 0;

    // *tmpFileNames = malloc(MAX_LINE_LENGTH * sizeof(char *));
    if (*tmpFileNames == NULL) {
        perror("Error allocating memory for tmpFileNames in check_merge_files");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; mergeFileNames[i] != NULL; i++) {
        get_dir_root(mergeFileNames[i], tmpFileNames, &tmp);
        const char *fileName = mergeFileNames[i];
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