
add_library(core STATIC
    "../include/valla/declarations.hpp"
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
