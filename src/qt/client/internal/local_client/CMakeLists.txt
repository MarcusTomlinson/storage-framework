set(QT_LOCAL_CLIENT_LIB_INTERNAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/AccountImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DownloaderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FolderImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ItemImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RootImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Runtime_create.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuntimeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UploaderImpl.cpp
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/DownloaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/UploaderImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/local_client/RuntimeImpl.h
    ${CMAKE_SOURCE_DIR}/include/unity/storage/qt/client/internal/RuntimeBase.h
)

include_directories(${GLIB_DEPS_INCLUDE_DIRS})

add_library(storage-framework-qt-local-client SHARED
    $<TARGET_OBJECTS:qt-client-lib-public>
    ${QT_LOCAL_CLIENT_LIB_INTERNAL_SRC})
qt5_use_modules(storage-framework-qt-local-client Concurrent Core Network)
set_target_properties(storage-framework-qt-local-client PROPERTIES AUTOMOC TRUE)
set_target_properties(storage-framework-qt-local-client PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
target_link_libraries(storage-framework-qt-local-client
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    ${Boost_LIBRARIES}
    ${GLIB_DEPS_LIBRARIES})
