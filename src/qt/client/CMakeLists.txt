set(QT_CLIENT_LIB_PUBLIC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Folder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Root.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Uploader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/AccountBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/DownloaderBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/FileBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/FolderBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/ItemBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/RootBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/RuntimeBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/UploaderBase.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/DownloaderBase.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/UploaderBase.h
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
add_library(qt-client-lib-public OBJECT ${QT_CLIENT_LIB_PUBLIC_SOURCE})
set_target_properties(qt-client-lib-public PROPERTIES AUTOMOC TRUE)

add_subdirectory(internal)
