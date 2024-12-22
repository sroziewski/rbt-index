# Minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Project name
project(rbt_index)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)  # Or 17 if you are using C++17, adjust as needed
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug symbols for debugging
set(CMAKE_BUILD_TYPE Debug)

# Include header file directories if needed (e.g., current directory)
include_directories(${CMAKE_SOURCE_DIR})

# Add your source files
add_executable(rbt_size_create
        rbt_size_create.c      # Your main entry point
        rbtree.c    # Implementation of Red-Black Tree
)

# Add arguments to the executable for IDEs/debugging
set_target_properties(rbt_size_create PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "/home/simon/playground/test.out.abc"
)