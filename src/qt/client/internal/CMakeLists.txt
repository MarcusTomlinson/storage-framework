set(QT_CLIENT_LIB_INTERNAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AccountImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DownloaderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FolderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RootImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UploaderImpl.cpp
)

add_library(storage-framework-qt-client SHARED
    $<TARGET_OBJECTS:qt-client-lib-public>
    ${QT_CLIENT_LIB_INTERNAL_SRC})
qt5_use_modules(storage-framework-qt-client Core Network)
target_link_libraries(storage-framework-qt-client Qt5::Core Qt5::Network)
