
add_library(core STATIC
    "../include/valla/compact_bucket_hash_set.hpp"
    "../include/valla/compact_cleary_hash_set.hpp"
    "../include/valla/compact_flat_hash_set.hpp"
    "../include/valla/compact_hash.hpp"
    "../include/valla/concepts.hpp"
    "../include/valla/debug.hpp"
    "../include/valla/dtdb_h_impl.hpp"
    "../include/valla/dtdb_h.hpp"
    "../include/valla/dtdb_s_impl.hpp"
    "../include/valla/dtdb_s.hpp"
    "../include/valla/equal_to.hpp"
    "../include/valla/hash_id_map.hpp"
    "../include/valla/hash.hpp"
    "../include/valla/indexed_hash_set.hpp"
    "../include/valla/ostream.hpp"
    "../include/valla/slot.hpp"
    "../include/valla/statistics.hpp"
    "../include/valla/succinct_flat_hash_set.hpp"
    "../include/valla/succinct_indexed_hash_set.hpp"
    "../include/valla/uint64tcoder.hpp"
    "../include/valla/utils.hpp"

    "sdsl/bits.cpp"
    "sdsl/bp_support_algorithm.cpp"
    "sdsl/coder_elias_delta.cpp"
    "sdsl/coder_elias_gamma.cpp"
    "sdsl/coder_fibonacci.cpp"
    "sdsl/config.cpp"
    "sdsl/construct_config.cpp"
    "sdsl/construct_isa.cpp"
    "sdsl/construct_lcp_helper.cpp"
    "sdsl/construct_sa_se.cpp"
    "sdsl/csa_alphabet_strategy.cpp"
    "sdsl/io.cpp"
    "sdsl/louds_tree.cpp"
    "sdsl/memory_management.cpp"
    "sdsl/nn_dict_dynamic.cpp"
    "sdsl/ram_filebuf.cpp"
    "sdsl/ram_fs.cpp"
    "sdsl/rrr_vector_15.cpp"
    "sdsl/sd_vector.cpp"
    "sdsl/sfstream.cpp"
    "sdsl/uint128_t.cpp"
    "sdsl/uint256_t.cpp"
    "sdsl/util.cpp"
    "sdsl/wt_helper.cpp"
)
set_target_properties(core PROPERTIES OUTPUT_NAME valla_core)

# Create an alias for simpler reference
add_library(valla::core ALIAS core)

target_link_libraries(core absl::flat_hash_set absl::flat_hash_map)

# Use include depending on building or using from installed location
target_include_directories(core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Install the target and create export-set
install(
    TARGETS core
    EXPORT vallacoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install export file
install(EXPORT vallacoreTargets
    NAMESPACE valla::
    COMPONENT core
    FILE "vallacoreTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/valla"
)

# Generate build tree export file
export(EXPORT vallacoreTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/vallacoreTargets.cmake"
    NAMESPACE valla::
)
