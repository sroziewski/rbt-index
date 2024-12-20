# **Red-Black Tree File Indexer**
This project implements a **Red-Black Tree-based File Indexing System** in **C**. The program parses file data, stores it in a Red-Black Tree for sorted management, and allows serialization and deserialization of the tree to/from files or shared memory.
## **Features**
1. **Red-Black Tree Implementation**:
    - Insert and manage file data in a sorted manner using a Red-Black Tree.
    - Each node represents a file's information (filename, size, path, and type).

2. **File Serialization**:
    - Serialize the Red-Black Tree to a binary file for persistent storage.
    - Files are saved with a custom `.rbt` extension.

3. **File Deserialization**:
    - Load a previously serialized Red-Black Tree from binary files.

4. **Shared Memory**:
    - Write the Red-Black Tree structure to shared memory for inter-process communication.

5. **Command-Line Interface**:
    - Load: `--load <file>`
    - Store: `--store`

6. **Built-in Memory Management**:
    - Includes utility functions to free all memory used by the Red-Black Tree.

## **Project Structure**
``` 
rbt_index/
├── main.c                   # Main program entry point
├── red_black_tree.c         # Core Red-Black Tree implementation
├── serialization.c          # Serialization and deserialization functions
├── shared_memory.c          # Shared memory operations
├── utils.c                  # Utility functions (e.g., string management)
├── CMakeLists.txt           # Build configuration for CMake
└── README.md                # Project documentation
```
## **Requirements**
- **Operating System**: Linux (tested on Ubuntu). Shared memory features may be OS-dependent.
- **Development Tools**:
    - GCC or Clang (for compiling C code)
    - `make` and `cmake` (for the build system)

- **Libraries**: None required (uses standard C libraries)
- Tested IDE: IntelliJ or Clion.

## **Setup and Build Instructions**
1. **Clone the Repository**:
``` sh
   git clone https://github.com/your-username/rbt_index.git
   cd rbt_index
```
1. **Build the Project**:
``` sh
   mkdir build
   cd build
   cmake ..
   make
```
1. **Run the Program**:
``` sh
   ./rbt_index <file> [--store] [--load filename]
```
## **Usage**
### **Command-Line Arguments**:
1. **Load a Tree from a File**:
``` sh
      ./rbt_index --load mytree.rbt
```
1. **Store Tree to a File**: After processing input, save the Red-Black Tree to a file:
``` sh
      ./rbt_index input_file.txt --store
```
1. **Default Behavior (In-Memory Processing)**: Processes input and stores the Red-Black Tree in shared memory.
``` sh
      ./rbt_index input_file.txt
```
## **File Input Format**
Input file data must have the following format for each line:
``` 
<filepath><SEP><filesize><SEP><filetype>
```
- `<SEP>` = delimiter between values (`<SEP>` can be customized in the code).
- **Example**:
``` 
  /path/to/file1<SEP>12345<SEP>text/plain
  /path/to/file2<SEP>67890<SEP>image/png
```
## **How It Works**
1. **File Parsing**:
    - Reads file data from the given input text file.
    - Parses and extracts `filepath`, `filesize`, and `filetype`.

2. **Red-Black Tree Insertion**:
    - Inserts these extracted values into the Red-Black Tree.
    - Nodes are sorted by filenames to maintain order.

3. **Serialization/Deserialization**:
    - Serialize the Red-Black Tree into a binary file for storage.
    - Deserialize it back into memory when needed.

4. **Shared Memory**:
    - Writes serialized tree data to a shared memory segment for inter-process communication.

## **Contributing**
Contributions are welcome! If you'd like to contribute:
1. Fork the repository.
2. Create a new feature branch:
``` sh
   git checkout -b feature-name
```
1. Commit your changes:
``` sh
   git commit -m "Describe the feature"
```
1. Push the branch:
``` sh
   git push origin feature-name
```
1. Open a Pull Request on GitHub.
