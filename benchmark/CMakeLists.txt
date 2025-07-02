find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)

add_executable(hashid_insert "hashid_insert.cpp")
target_link_libraries(hashid_insert valla::core)
target_link_libraries(hashid_insert benchmark::benchmark)

add_executable(insert "insert.cpp")
target_link_libraries(insert valla::core)
target_link_libraries(insert benchmark::benchmark)

add_executable(read "read.cpp")
target_link_libraries(read valla::core)
target_link_libraries(read benchmark::benchmark)

add_executable(iterator "iterator.cpp")
target_link_libraries(iterator valla::core)
target_link_libraries(iterator benchmark::benchmark)
