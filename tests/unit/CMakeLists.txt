enable_testing()

find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(GTest)

# Helper function to create and register a test executable
function(add_gtest test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE valla::core GTest::GTest GTest::Main)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add each test source file as a separate test executable
add_gtest(uint_tree_compression                         "uint/tree_compression.cpp")

add_gtest(uint_tree_database                            "uint/tree_database.cpp")

add_gtest(uint_delta_tree_compression                   "uint/delta_tree_compression.cpp")

add_gtest(double_tree_compression                       "double/tree_compression.cpp")