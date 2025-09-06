find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)

add_executable(plain_swiss_concurrent_insert "plain/swiss/concurrent_insert.cpp")
target_link_libraries(plain_swiss_concurrent_insert valla::core)
target_link_libraries(plain_swiss_concurrent_insert benchmark::benchmark)
target_link_libraries(plain_swiss_concurrent_insert TBB::tbb)

add_executable(plain_swiss_insert "plain/swiss/insert.cpp")
target_link_libraries(plain_swiss_insert valla::core)
target_link_libraries(plain_swiss_insert benchmark::benchmark)

add_executable(plain_swiss_concurrent_read "plain/swiss/concurrent_read.cpp")
target_link_libraries(plain_swiss_concurrent_read valla::core)
target_link_libraries(plain_swiss_concurrent_read benchmark::benchmark)
target_link_libraries(plain_swiss_concurrent_read TBB::tbb)

add_executable(plain_swiss_read "plain/swiss/read.cpp")
target_link_libraries(plain_swiss_read valla::core)
target_link_libraries(plain_swiss_read benchmark::benchmark)

add_executable(plain_hash_id_map_insert "plain/hash_id_map/insert.cpp")
target_link_libraries(plain_hash_id_map_insert valla::core)
target_link_libraries(plain_hash_id_map_insert benchmark::benchmark)

add_executable(plain_hash_id_map_read "plain/hash_id_map/read.cpp")
target_link_libraries(plain_hash_id_map_read valla::core)
target_link_libraries(plain_hash_id_map_read benchmark::benchmark)

