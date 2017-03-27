find_package(DoxygenBuilder)

add_doxygen(
    storage-framework-doc
    INPUT ${CMAKE_CURRENT_SOURCE_DIR}
    ALL
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION share/doc/${LIBNAME}
)
