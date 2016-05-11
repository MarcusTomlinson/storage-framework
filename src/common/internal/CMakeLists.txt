set(src
    ${CMAKE_CURRENT_SOURCE_DIR}/mimetype.cpp
)

include_directories(${GIO_DEPS_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})

add_library(storage-framework-common-internal STATIC ${src})
