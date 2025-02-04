## Project Overview
This project implements a set of programs to create, maintain, and search red-black trees (RBT). Each program serves a specific purpose, such as creating red-black trees based on filenames or file sizes, performing searches within the trees, and listing files.
### Programs in This Project:
1. **rbt_name_create.c**
   A program to create a red-black tree using filenames as keys.
2. **rbt_size_create.c**
   A program to create a red-black tree using file sizes as keys.
3. **rbt_search.c**
   A program for searching red-black trees based on a name or type pattern.
4. **list_files.c**
   A utility program to list files in directories.

## File Descriptions
### **1. rbt_name_create.c**
The `rbt_name_create.c` program is used to create a red-black tree where filenames are the keys. The program utilizes a filename comparator (`strcmp`) to ensure proper ordering in the tree.
- **Key Features**:
    - Reads input arguments to initialize and populate the red-black tree.
    - Uses `createRbt` and `insert_filename` functions to construct the tree.
    - Stores the tree in shared memory with the prefix `"shared_memory_fname_"`.

- **Relevant Function**:
    - `main()`:
      Serves as the entry point to the program.
      Calls `createRbt()` with filename-specific parameters to populate the red-black tree.

### **2. rbt_size_create.c**
The `rbt_size_create.c` program implements functionality to create a red-black tree using file sizes as keys. It defines a numeric comparator to handle ordering by file size.
- **Key Features**:
    - Initializes the program with input arguments and creates the RBT.
    - Uses `createRbt` and `insert_filesize` functions to construct the tree.
    - Stores the tree in shared memory with the prefix `"shared_memory_fsize_"`.

- **Relevant Function**:
    - `main()`:
      Acts as the starting point for creating the tree.
      Calls `createRbt()` with file size as the sorting key.

### **3. rbt_search.c**
The `rbt_search.c` program allows for advanced searching of red-black trees. It supports searching nodes by filename patterns or file types.
- **Key Features**:
    - Implements multithreaded search functionality with thread synchronization.
    - Supports search functions for name (`search_tree_for_name_and_type`) and type.
    - Allows users to search the tree while utilizing a thread pool for concurrent operations.

- **Important Functions**:
    - `void search_tree_for_name_and_type(Node *root, const char *namePattern, const char *targetType)`:
      Performs a search operation on the tree using specific patterns for filenames and file types.
    - `void initialize_threads()`:
      Initializes threading structures.
    - `int main(int argc, char *argv[])`:
      Coordinates program initialization and execution of the search functionality.

### **4. list_files.c**
The `list_files.c` program is a utility to list files in a directory structure. It interacts with the tree creation and management programs to list relevant files.
- **Relevant Function**:
    - `int main(const int argc, char *argv[])`:
      The starting point of the program, responsible for parsing arguments and listing files.

## Compilation
Use a build system of your choice to compile the project. Ensure the required C standard libraries are included in your system.
For example, using `gcc`:
``` sh
gcc rbt_name_create.c -o rbt_name_create
gcc rbt_size_create.c -o rbt_size_create
gcc rbt_search.c -o rbt_search -pthread
gcc list_files.c -o list_files
```
## Usage
Below is an example of how to use each program:
### **1. rbt_name_create**
``` sh
./rbt_name_create [arguments]
```
Creates a red-black tree where filenames are used as keys and stores it in shared memory.
### **2. rbt_size_create**
``` sh
./rbt_size_create [arguments]
```
Creates a red-black tree where file sizes are used as keys and stores it in shared memory.
### **3. rbt_search**
``` sh
./rbt_search [arguments]
```
Searches the red-black tree for specific patterns or types. Use appropriate command-line arguments to specify the target tree and search parameters.
#### Advanced Example:
``` sh
./rbt_search -f rbt_size_simon.lst.rbt.mem -s --size 10M-50M -t T_COMPRESSED
```
- `-f rbt_size_simon.lst.rbt.mem`: Specifies the shared memory file containing the tree.
- `-s`: Enables the search mode.
- `--size 10M-50M`: Filters nodes with file sizes between 10 MB and 50 MB.
- `-t T_COMPRESSED`: Specifies the file type as `T_COMPRESSED` for further filtering.

### **4. list_files**
``` sh
./list_files [arguments]
```
Lists files in a directory based on the supplied arguments.
## Multithreading in rbt_search
The program `rbt_search.c` leverages multithreading for improved performance when searching the red-black tree. It uses:
- Synchronization via `PTHREAD_MUTEX_INITIALIZER`.
- A global thread counter to manage active threads.

## Notes
- Ensure the input arguments are valid and match the expected format for each function.
- The shared memory prefixes (`shared_memory_fname_` and `shared_memory_fsize_`) allow for organized tree storage.
- Use multi-threaded searching via `rbt_search` to efficiently process large datasets and complex queries.
