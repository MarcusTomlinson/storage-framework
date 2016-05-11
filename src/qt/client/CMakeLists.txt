include_directories(${Qt5Core_INCLUDE_DIRS})

add_subdirectory(internal)

set(src
    ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Folder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Root.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Uploader.cpp
)

add_library(storage-framework-qt-client SHARED ${src})
qt5_use_modules(storage-framework-qt-client Core Network)
target_link_libraries(storage-framework-qt-client Qt5::Core Qt5::Network)
