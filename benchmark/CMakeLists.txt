find_package(benchmark "1.8.3" REQUIRED benchmark Main PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
# Set result variables
find_package(benchmark)


add_executable(plain_swiss_insert "plain/swiss/insert.cpp")
target_link_libraries(plain_swiss_insert valla::core)
target_link_libraries(plain_swiss_insert benchmark::benchmark)

add_executable(plain_swiss_read "plain/swiss/read.cpp")
target_link_libraries(plain_swiss_read valla::core)
target_link_libraries(plain_swiss_read benchmark::benchmark)


