message ("-- Checking if GoogleTest is installed ...")

find_package (GTest REQUIRED)
if (NOT GTest_FOUND)
    message (WARNING "-- GoogleTest not installed")
    return ()

endif ()

include (GoogleTest)

add_custom_target (tests)
add_dependencies (tests gtest_threadsorter)

message ("-- Done configuring gtest")


aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRC_GTEST)
add_executable (gtest_threadsorter EXCLUDE_FROM_ALL ${SRC_GTEST})
target_link_libraries (gtest_threadsorter gtest gtest_main)
