cmake_minimum_required (VERSION 3.12)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED true)

project ("Thread Sorter")

message ("-- Building on Linux ...")

# configure compiler gcc with Ccache
set (CMAKE_C_COMPILER "gcc")
set (CMAKE_CXX_COMPILER "g++")

message ("-- Using C Compiler: ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_VERSION})")
message ("-- Using C++ Compiler: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_VERSION})")

option (BUILD_SHARED_LIBS "Build using shared libraries" ON)

# configure output dir of static and shared library
include (GNUInstallDirs)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

set (DIR_SRC_ROOT "${PROJECT_SOURCE_DIR}/src")

include_directories (${DIR_SRC_ROOT})
aux_source_directory (${DIR_SRC_ROOT} PROJECT_SOURCE_FILES)

add_library (yhgthreadsorter ${PROJECT_SOURCE_FILES})


add_subdirectory ("snippet")
