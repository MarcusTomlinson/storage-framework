find_package(DoxygenBuilder)

add_doxygen(
    storage-framework-doc
    INPUT
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include/unity/storage
    STRIP_FROM_PATH
        ${CMAKE_SOURCE_DIR}
    STRIP_FROM_INC_PATH
        ${CMAKE_SOURCE_DIR}/include
    EXCLUDE_PATTERNS
        */internal/*
        */registry/*
    ALL
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION share/doc/storage-framework
)
