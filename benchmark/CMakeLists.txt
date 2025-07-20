find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)

add_executable(plain_uint_swiss_insert "plain/uint/swiss/insert.cpp")
target_link_libraries(plain_uint_swiss_insert valla::core)
target_link_libraries(plain_uint_swiss_insert benchmark::benchmark)

add_executable(plain_uint_swiss_read "plain/uint/swiss/read.cpp")
target_link_libraries(plain_uint_swiss_read valla::core)
target_link_libraries(plain_uint_swiss_read benchmark::benchmark)

add_executable(plain_uint_swiss_iterator "plain/uint/swiss/iterator.cpp")
target_link_libraries(plain_uint_swiss_iterator valla::core)
target_link_libraries(plain_uint_swiss_iterator benchmark::benchmark)
