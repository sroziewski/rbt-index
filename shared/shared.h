#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <magic.h>
#include <pthread.h>
#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <omp.h>
#include <stdbool.h>
#include <unistd.h>

#include "../shared/lconsts.h"


#ifndef SHARED_H
#define SHARED_H

int process_file(const char *filename);
void check_input_files(char **mergeFileNames, char ***tmpFileNames, int *rootCount);
bool is_valid_file_type(const char *type);
int is_size_t(const char *str);
void get_dir_root(const char *fileName, char ***root, int *count);

#endif //SHARED_H
