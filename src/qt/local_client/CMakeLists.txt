set(src
    ${CMAKE_CURRENT_SOURCE_DIR}/AccountImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DownloaderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FolderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RootImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UploaderImpl.cpp
)

include_directories(${GLIB_DEPS_INCLUDE_DIRS})

add_library(storage-framework-qt-local-client-internal SHARED ${src})
qt5_use_modules(storage-framework-qt-local-client-internal Core Network)
target_link_libraries(storage-framework-qt-local-client-internal Qt5::Core Qt5::Network)
