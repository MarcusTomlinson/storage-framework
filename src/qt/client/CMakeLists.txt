include_directories(${Qt5Core_INCLUDE_DIRS})

add_subdirectory(internal)

set(src
    ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Directory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Root.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Uploader.cpp
)

set(QT_CLIENT_LIB_SRC ${QT_CLIENT_LIB_SRC} ${src})

add_library(storage-framework-qt-client SHARED ${QT_CLIENT_LIB_SRC})
