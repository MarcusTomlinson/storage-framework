set(QT_CLIENT_LIB_PUBLIC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Folder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Root.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Uploader.cpp
)

add_subdirectory(internal)
