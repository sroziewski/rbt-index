#include "rbtree.h"

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [--save] [--load filename] [--clean filename]\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *prefix = "shared_memory_fname_";
    Node *finalRoot = NULL;

    if (argc == 3 && strcmp(argv[1], "--load") == 0) {
        // Handle the --load command

        read_tree_from_file_to_shared_memory(argv[2], prefix);

        // finalRoot = load_rbt_from_file(argv[2]);
        // if (!finalRoot) {
        //     fprintf(stderr, "Failed to load Red-Black Tree from file: %s\n", argv[2]);
        //     return EXIT_FAILURE;
        // }
        //
        // printf("Red-Black Tree successfully loaded from file: %s\n", argv[2]);
        // printf("Files stored in Red-Black Tree in sorted order by filename:\n");
        // write_tree_to_shared_memory(finalRoot);
    }
    // Handle the --clean command
    else if (argc == 3 && strcmp(argv[1], "--clean") == 0) {
        return remove_shared_memory_object(argv, prefix);
    }
    else {
        // Handle the normal processing and storing workflow
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        char **lines = NULL;
        size_t numLines = 0;
        char buffer[8 * 1024];

        while (fgets(buffer, sizeof(buffer), file)) {
            remove_trailing_newline(buffer);
            if (*buffer) {
                lines = realloc(lines, sizeof(char *) * ++numLines);
                if (!lines) {
                    perror("Failed to allocate memory for lines");
                    fclose(file);
                    return EXIT_FAILURE;
                }
                lines[numLines - 1] = strdup(buffer);
                if (!lines[numLines - 1]) {
                    perror("Failed to duplicate buffer");
                    fclose(file);
                    return EXIT_FAILURE;
                }
            }
        }
        fclose(file);

        int totalProcessedCount = 0;

        for (size_t i = 0; i < numLines; i++) {
            const FileInfo key = parseFileData(lines[i]);
            if (key.filename && key.filepath && key.filetype) {
                insert(&finalRoot, key);
                totalProcessedCount++;
            }
        }

        printf("\nFiles stored in Red-Black Tree in sorted order by filename:\n");
        inorder(finalRoot);

        printf("Total lines successfully processed: %d\n", totalProcessedCount);

        for (size_t i = 0; i < numLines; i++) {
            free(lines[i]);
        }
        free(lines);

        // Check if the program is run with the --save option
        if (argc == 3 && strcmp(argv[2], "--save") == 0) {
            char *storeFilename = add_rbt_extension(argv[1]);  // Use the input file's name as the base and append `.rbt`
            write_tree_to_file(finalRoot, storeFilename);
            // store_rbt_to_file(finalRoot, storeFilename);
            // printf("Red-Black Tree has been stored in file: %s\n", storeFilename);
            free(storeFilename);
        }
        else {
            write_tree_to_shared_memory(finalRoot, argv[1], prefix);
        }
    }

    // Free the tree if it was loaded or created
    freeTree(finalRoot);

    return EXIT_SUCCESS;
}
