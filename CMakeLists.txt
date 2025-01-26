# Minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Project name
project(rbt_index)

# Add support for OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)  # Or 17 if you are using C++17, adjust as needed
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug symbols for debugging
set(CMAKE_BUILD_TYPE Debug)

# Include header file directories (if needed)
include_directories(${CMAKE_SOURCE_DIR})

# Find libmagic (file type detection library)
find_library(MAGIC_LIB magic)
if(NOT MAGIC_LIB)
    message(FATAL_ERROR "libmagic not found! Ensure libmagic-dev (or equivalent) is installed.")
else()
    message(STATUS "Found libmagic library: ${MAGIC_LIB}")
endif()

# Find OpenSSL library
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})     # Include OpenSSL headers
else()
    message(FATAL_ERROR "OpenSSL not found! Ensure libssl-dev (or equivalent) is installed.")
endif()

# Add your source files for list_files
add_executable(list_files
        list_files.c  # Your main entry point for list_files
        flib/lfiles.c
        shared/shared.c
        # File containing the implementation for list_files
)

# Link libraries: libmagic, OpenMP, and OpenSSL (libcrypto) for list_files
target_link_libraries(list_files PRIVATE ${MAGIC_LIB} OpenMP::OpenMP_C OpenSSL::Crypto)

# Add arguments to the list_files executable for IDEs/debugging (optional)
set_target_properties(list_files PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "/home/simon/playground -o /home/simon/playground/playground.list"
)

# Add another executable: rbt_create
add_executable(rbt_create
        rbt_create.c
        rbtlib/rbtree.c
        shared/shared.c
)

# Link libraries: OpenSSL for rbt_create
target_link_libraries(rbt_create PRIVATE OpenSSL::Crypto)

# Add another executable: rbt_search
add_executable(rbt_search
        rbt_search.c
        rbtlib/rbtree.c
        shared/shared.c
        rbtlib/search.c
)

# Link libraries: OpenSSL for rbt_search
target_link_libraries(rbt_search PRIVATE OpenSSL::Crypto)

# Add another executable: rbt_name_create
add_executable(rbt_name_create
        rbt_name_create.c
        rbtlib/rbtree.c  # Your main entry point for rbt_name_create
)

# Link libraries: OpenSSL for rbt_name_create
target_link_libraries(rbt_name_create PRIVATE OpenSSL::Crypto)

# Add arguments to the rbt_name_create executable for IDEs/debugging (optional)
set_target_properties(rbt_name_create PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "/home/simon/playground/playground.list"
)

# Add another executable: rbt_size_create
add_executable(rbt_size_create
        rbt_size_create.c
        rbtlib/rbtree.c  # Your main entry point for rbt_size_create
)

# Link libraries: OpenSSL for rbt_size_create
target_link_libraries(rbt_size_create PRIVATE OpenSSL::Crypto)

# Add arguments to the rbt_size_create executable for IDEs/debugging (optional)
set_target_properties(rbt_size_create PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "/home/simon/playground/playground.list"
)
