enable_testing()

find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(GTest)

# Helper function to create and register a test executable
function(add_gtest test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE absl::flat_hash_set absl::flat_hash_map GTest::GTest GTest::Main)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add each test source file as a separate test executable
add_gtest(tree_compression                        "tree_compression.cpp")